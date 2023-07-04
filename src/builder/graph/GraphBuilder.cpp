
#include "GraphBuilder.h"
#include "AbstractionLinker.h"
#include "PrematureActionPreventer.h"
#include "algo/path/LongestPathCalculator.h"
#include "builder/graph/headways/HeadwayBuilder.h"
#include "model/graph/GraphWithMeta.h"
#include "model/problems/ProblemInstance.h"
#include "options/GraphBuilderOptions.h"

namespace fb {

    GraphBuilder::GraphBuilder(Model const &model, GraphBuilderOptions const &options)
        : model_{model}, options_{options}, creator_(model, options) {}

    graph::GraphWithMeta GraphBuilder::build(const ProblemInstance &problem) {
        PartialSchedule pre_planned_schedule(model_.courses.size());
        return build(problem, pre_planned_schedule);
    }
    graph::GraphWithMeta GraphBuilder::build(const ProblemInstance &problem,
                                             PartialSchedule const &pre_planned_schedule) {

        graph::GraphWithMeta result{model_.nodes.size(), model_.courses.size(), model_.links.size()};

        auto &graph = result.graph;
        result.start_node = graph.createNode();


        for (auto const &rs: model_.duties) {
            buildForDuty(rs, problem, result);
        }


        // link abstractions
        AbstractionLinker linker{result.graph, problem.mapped_extended_runtimes};
        for (auto i = 0; i < result.abstractions.size(); ++i) {
            auto &abs = result.abstractions[i];
            abs.materialize(result.graph);

            auto predecessor = result.abstractions.prior(i);
            if (predecessor != nullptr && predecessor->getIndex() < i) {
                linker.link(*predecessor, abs, *model_.getLinkFromTo(predecessor->getNode(), abs.getNode()));
            }
        }

        for (auto const &c: model_.courses) {
            auto const *end = result.abstractions.courseEnd(c.index);
            if (end == nullptr) {
                continue;
            }
            createMeasureNode(*end, result, graph);
        }


        PrematureActionPreventer pap{result, problem, options_.max_premature_departure, true};
        for (auto *abs: result.abstractions) {
            pap.preventEarlyActions(*abs);
        }


        // process late departures -> line can only be entered after the delayed departure time
        for (auto const &ld: problem.late_departures) {
            auto const *course_start = result.abstractions.firstAbsOfCourse(ld.course);

            if (course_start == nullptr) {
                // course is not in the graph
                assert(model_.courses[ld.course].planned_start < problem.snapshot_time ||
                       model_.courses[ld.course].planned_start > problem.snapshot_time + options_.extend);
                continue;
            }


            assert(course_start->getOriginalOutScheduleItem()->index == 0);
            seconds_t actual_start = model_.courses[ld.course].planned_start + ld.departure_delay;

            if (course_start->getOutCourse().index_in_duty > 0) {
                graph.createFixedEdge(result.start_node, course_start->getDepartureTimeInNode(), actual_start);
            } else {
                // the first course comes from the depot. Therefore, it does not arrive at the node before the late departure time
                graph.createFixedEdge(result.start_node, course_start->getArrivalTimeInNode(), actual_start);
            }
        }


        LongestPathCalculator calc{result.graph, result.start_node, result.measure_nodes.getNodes()};
        auto no_constraint_pc_state = calc.calculateInitialPaths(graph::Selection{result.graph});
        result.no_constrain_distances = std::move(no_constraint_pc_state.distance_from_start);


        HeadwayBuilder hwb{model_, graph, result.abstractions, result.no_constrain_distances, options_};
        hwb.buildAllHeadways();


        return result;
    }


    void GraphBuilder::createMeasureNode(const graph::TrainAtStationAbstraction &tas, graph::GraphWithMeta &result,
                                         graph::AlternativeGraph &graph) const {
        // only necessary for passenger courses
        if (tas.getInCourse().category == CourseCategory::PASSENGER && isInvalid(tas.getRealizedArrival())) {
            auto measure_node = graph.createNode();
            auto planned_arrival = -((graph::weight_t) tas.getOriginalArrival());
            graph.createFixedEdge(tas.getArrivalTimeOutNode(), measure_node, planned_arrival);
            result.measure_nodes.addMeasureNode(measure_node, tas.getInCourse().index);
            result.measure_node_courses.push_back(tas.getInCourse().index);
        }
    }
    void GraphBuilder::buildForDuty(const RSDuty &duty, const ProblemInstance &problem, graph::GraphWithMeta &result) {

        auto max_extend = problem.snapshot_time + options_.extend;

        for (auto di = 0; di < duty.elements.size(); ++di) {

            auto const &duty_elem = duty.elements[di];

            EventType type = duty_elem.event;
            if (type == EventType::RESERVE) {
                break;
            }
            if (type == EventType::SPARE) {
                // ignore
                continue;
            }
            if (type == EventType::CHANGE_END) {
                auto change_end_time = duty_elem.planned_end_time - duty_elem.planned_start_time;
                auto const &prior_course = model_.courses[duty.lastTrainElementBefore(di)->course];
                auto const &next_course = model_.courses[duty.nextTrainElementAfter(di)->course];
                createCourseLink(problem, prior_course, next_course, change_end_time, result);

                continue;
            }

            if (type == EventType::TRAIN) {

                // don't model duty elements out of scope
                if (duty_elem.planned_start_time >= max_extend) {
                    break;
                }

                auto const &course = model_.courses[duty_elem.course];

                bool is_first_course = di == 0;
                bool is_last_course = di == duty.elements.size() - 1;
                createForCourse(problem, course, result, is_first_course, is_last_course);
            }
        }
    }

    bool GraphBuilder::hasPotentialImpact(ProblemInstance const &problem, course_index_t course_index,
                                          index_t si_index) const {
        std::vector<ScheduleItem> const &realized_schedule = problem.realized_schedule[course_index];
        bool realized = realized_schedule.size() > si_index;

        if (!realized) {
            return true;
        }
        ScheduleItem const &rsi = realized_schedule[si_index];

        if (isInvalid(rsi.departure)) {
            return true;// only partly realized
        }

        // the next si is not realized -> can impact by forbidding overtaking
        if (problem.realized_schedule[course_index].size() == si_index + 1) {
            return true;
        }

        // fully realized
        seconds_t diff_from_snapshot = problem.snapshot_time - rsi.departure;
        if (diff_from_snapshot < model_.max_headway || diff_from_snapshot < model_.max_runtime) {
            return true;
        }
        return false;
    }

    void GraphBuilder::createCourseLink(const ProblemInstance &problem, const Course &first_course,
                                        const Course &second_course, seconds_t change_end_time,
                                        graph::GraphWithMeta &result) {
        if (!hasPotentialImpact(problem, second_course.index, 0)) {
            return;
        }

        seconds_t change_end_by_infrastructure = model_.changeEndTime(first_course.direction, second_course.direction,
                                                                      first_course.schedule.back().node);
        seconds_t min_change_end = std::min(change_end_time, change_end_by_infrastructure);

        graph::TaggedWeight stop_time = calculateStopTime(problem, result.graph, second_course, 0, min_change_end);
        auto *created = creator_.createAbstraction(problem, result.abstractions, second_course.index, first_course,
                                                   first_course.schedule.size() - 1, stop_time);

        auto index = created->getIndex();
        if (index == 0) {
            return;
        }
        graph::TrainAtStationAbstraction &prior_created = result.abstractions[index - 1];
        if (prior_created.getOutCourse().index == first_course.index) {
            auto const *link = model_.getLinkFromTo(prior_created.getNode(), created->getNode());
            assert(link != nullptr);
            result.abstractions.registerSuccessor(prior_created, *created, *link);
        }
    }

    void GraphBuilder::createForCourse(const ProblemInstance &problem, const Course &course,
                                       graph::GraphWithMeta &result, bool first_course, bool last_course) {

        auto const &duty = model_.duties[course.duty];

        int start = first_course ? 0 : 1;
        int end = last_course ? course.schedule.size() : course.schedule.size() - 1;

        graph::TrainAtStationAbstraction *last_abs = nullptr;
        if (!result.abstractions.empty()) {
            last_abs = &result.abstractions.back();
            if (last_abs->getOutCourse().index != course.index) {
                last_abs = nullptr;
            }
        }

        for (auto i = start; i < end; i++) {


            if (!hasPotentialImpact(problem, course.index, i)) {
                continue;
            }

            graph::TaggedWeight stop_time = calculateStopTime(problem, result.graph, course, i, 0);

            auto *created = creator_.createAbstraction(problem, result.abstractions, invalid<course_index_t>(), course,
                                                       i, stop_time);

            if (last_abs != nullptr) {
                auto const *link = model_.getLinkFromTo(last_abs->getNode(), created->getNode());
                assert(link != nullptr);
                result.abstractions.registerSuccessor(*last_abs, *created, *link);
            }
            last_abs = created;
        }
    }


    graph::TaggedWeight GraphBuilder::calculateStopTime(const ProblemInstance &problem, graph::AlternativeGraph &graph,
                                                        const Course &course, int si_index,
                                                        seconds_t change_end_time = 0) {

        std::vector<ScheduleItem> const &realized = problem.realized_schedule[course.index];
        bool in_realized_sched = realized.size() > si_index &&
                                 (isValid(realized[si_index].arrival) || isValid(realized[si_index].departure));
        auto const &planned_si = course.schedule[si_index];
        auto const &si = in_realized_sched ? realized[si_index] : planned_si;


        // decide on base stop time based on given information
        graph::weight_t base_stop;
        if (isValid(si.arrival) && isValid(si.departure)) {
            base_stop = si.departure - si.arrival;
        } else if (in_realized_sched && isValid(planned_si.departure) && isValid(planned_si.arrival)) {
            // choose planned time
            base_stop = planned_si.departure - planned_si.arrival;
        } else if (si_index == 0 || si_index == course.schedule.size() - 1) {
            base_stop = change_end_time;
        } else {
            base_stop = 0;
        }

        auto stop_time = graph::TaggedWeight::fixed(base_stop);

        if (in_realized_sched && isValid(si.arrival) && isValid(si.departure)) {
            // the departure has already happened nothing can be changed
            return stop_time;
        }

        auto node = model_.nodes[si.node];
        if (problem.mapped_extended_dwelltimes.contains(node.index)) {
            auto const &ext_dwell = problem.mapped_extended_dwelltimes.at(node.index);

            ExtendedDwellTime const &e = *ext_dwell[0];

            if (e.extended_dwell > base_stop) {
                auto index = graph.createTimeDependentWeight(base_stop, e.extended_dwell, e.start, e.end);
                stop_time = graph::TaggedWeight::dependent(index);

                assert(!(si_index == 0) &&
                       !(si_index == course.schedule.size() - 1));// sanity check for now, may be a wrong assumption
            }
        }

        // train specific extended stop time overwrites general node increase
        if (problem.mapped_extended_train_dwelltimes.contains({course.index, node.index})) {
            TrainExtendedDwell const &ext_dwell =
                    *problem.mapped_extended_train_dwelltimes.at({course.index, node.index});
            assert(ext_dwell.extended_dwell > base_stop);
            stop_time = graph::TaggedWeight::fixed(ext_dwell.extended_dwell);
        }
        return stop_time;
    }


}// namespace fb
