
#include "ProblemGenerator.h"

namespace fb {
    ProblemGenerator::ProblemGenerator(const Model &model, const GeneratorOptions &options)
        : model_(model), options_(options), re_(options.seed) {}
    ProblemInstance ProblemGenerator::generateProblemInstance() {


        ProblemInstance problem{};

        auto min = options_.min_snapshot_time;
        auto max = options_.max_snapshot_time;

        if (isInvalid(max)) {
            max = SECONDS_PER_DAY;
        }

        std::uniform_int_distribution<seconds_t> uniform(min, max);

        problem.snapshot_time = uniform(re_);


        // we use the original schedule up to the snapshot time
        for (auto const &c: model_.courses) {
            auto const &sched = c.schedule;

            if (c.planned_end < problem.snapshot_time) {
                problem.realized_schedule.push_back(sched);
                auto &realized = problem.realized_schedule.back();
                realized.front().arrival = model_.realArrivalTime(c, realized.front());
                realized.back().departure = model_.realDepartureTime(c, realized.back());
                if (realized.back().departure >= problem.snapshot_time) {
                    realized.back().departure = invalid<seconds_t>();
                }
                continue;
            }
            auto &realized = problem.realized_schedule.emplace_back();


            for (auto const &si: sched) {
                ScheduleItem realized_si = si;
                realized_si.arrival = model_.realArrivalTime(c, si);// set the arrival from prior course if necessary
                realized_si.departure = model_.realDepartureTime(c, si);
                if (realized_si.departure < problem.snapshot_time) {
                    realized.push_back(realized_si);
                } else if (realized_si.arrival < problem.snapshot_time) {
                    realized_si.departure = invalid<seconds_t>();
                    realized.push_back(realized_si);
                    break;
                }
            }
        }
        generateIncidents(problem);

        return problem;
    }
    void ProblemGenerator::generateIncidents(ProblemInstance &problem) {
        auto required_incidents =
                validOrElse(options_.min_extended_run_time, 0) + validOrElse(options_.min_extended_dwell_time, 0) +
                validOrElse(options_.min_course_dwell_time, 0) + validOrElse(options_.min_late_departures, 0);

        auto max_allowed_incidents = validOrElse(options_.max_extended_run_time, options_.max_incidents) +
                                     validOrElse(options_.max_extended_dwell_time, options_.max_incidents) +
                                     validOrElse(options_.max_course_dwell_time, options_.max_incidents) +
                                     validOrElse(options_.max_late_departures, options_.max_incidents);

        if (required_incidents > options_.max_incidents) {
            throw std::runtime_error("Cannot generate problem instance with given options");
        }
        if (max_allowed_incidents < options_.min_incidents) {
            throw std::runtime_error("Cannot generate problem instance with given options");
        }


        auto real_min_incidents = std::max(options_.min_incidents, required_incidents);
        auto real_max_incidents = std::min(options_.max_incidents, max_allowed_incidents);

        // we generate incidents
        std::uniform_int_distribution<int> incident_dist(real_min_incidents, real_max_incidents);
        int num_incidents = incident_dist(re_);

        std::uniform_int_distribution<seconds_t> incident_time_dist(
                problem.snapshot_time, problem.snapshot_time + options_.max_incident_in_future);


        auto already_generated = 0;
        if (isValid(options_.min_extended_run_time)) {
            while (problem.extended_runtimes.size() < options_.min_extended_run_time) {
                generateExtendedRunTimeIncident(problem);
                ++already_generated;
            }
        }

        if (isValid(options_.min_extended_dwell_time)) {
            while (problem.extended_dwelltimes.size() < options_.min_extended_dwell_time) {
                generateExtendedDwellTimeIncident(problem);
                ++already_generated;
            }
        }
        if (isValid(options_.min_course_dwell_time)) {
            while (problem.extended_train_dwelltimes.size() < options_.min_course_dwell_time) {
                generateCourseDwellTimeIncident(problem);
                ++already_generated;
            }
        }
        if (isValid(options_.min_late_departures)) {
            while (problem.late_departures.size() < options_.min_late_departures) {
                generateLateDepartureIncident(problem);
                ++already_generated;
            }
        }

        std::uniform_int_distribution<int> incident_type_dist(0, 3);
        while (already_generated < num_incidents) {
            auto incident_type = incident_type_dist(re_);

            switch (incident_type) {
                case 0:
                    if (options_.max_extended_run_time > problem.extended_runtimes.size()) {
                        generateExtendedRunTimeIncident(problem);
                        ++already_generated;
                    }
                    break;
                case 1:
                    if (options_.max_extended_dwell_time > problem.extended_dwelltimes.size()) {
                        generateExtendedDwellTimeIncident(problem);
                        ++already_generated;
                    }
                    break;
                case 2:
                    if (options_.max_course_dwell_time > problem.extended_train_dwelltimes.size()) {
                        generateCourseDwellTimeIncident(problem);
                        ++already_generated;
                    }
                    break;
                case 3:
                    if (options_.max_late_departures > problem.late_departures.size()) {
                        generateLateDepartureIncident(problem);
                        ++already_generated;
                    }
                    break;
                default:
                    throw std::runtime_error("Invalid incident type");
            }
        }
    }
    void ProblemGenerator::generateLateDepartureIncident(ProblemInstance &problem) {


        auto &ld = problem.late_departures.emplace_back();

        std::uniform_int_distribution<seconds_t> delay_dist(options_.late_departure_min_delay,
                                                            options_.late_departure_max_delay);
        ld.departure_delay = delay_dist(re_);

        std::uniform_int_distribution<int> course_dist(0, model_.courses.size() - 1);
        ld.course = invalid<course_index_t>();
        while (isInvalid(ld.course)) {
            auto c = course_dist(re_);
            auto const &course = model_.courses[c];
            // only choose courses in the future
            if (course.planned_start > problem.snapshot_time &&
                course.planned_start < problem.snapshot_time + options_.max_incident_in_future) {
                ld.course = c;
            }
        }
    }
    void ProblemGenerator::generateCourseDwellTimeIncident(ProblemInstance &problem) {

        auto &cdt = problem.extended_train_dwelltimes.emplace_back();

        std::vector<course_index_t> potential_courses;

        for (auto const &c: model_.courses) {
            if (c.category == CourseCategory::EMPTY) {
                continue;
            }
            if (c.schedule.size() == problem.realized_schedule[c.index].size()) {
                continue;
            }
            if (c.planned_start > problem.snapshot_time + options_.max_incident_in_future) {
                continue;
            }
            potential_courses.push_back(c.index);
        }
        if (potential_courses.empty()) {
            throw std::runtime_error("No potential courses for extended dwell time incident");
        }

        std::uniform_int_distribution<int> course_dist(0, potential_courses.size() - 1);

        course_index_t cid = invalid<course_index_t>();
        auto stop_count = 0;
        auto stop_map = std::vector<index_t>{};
        while (!isValid(cid)) {
            auto index_in_lookup = course_dist(re_);

            Course const &course = model_.courses[potential_courses[index_in_lookup]];

            // no extended dwell possible for courses that are already realized
            std::vector<ScheduleItem> const &selected_sched = course.schedule;
            std::vector<ScheduleItem> const &realized_sched = problem.realized_schedule[course.index];

            // don't add incidents after threshold
            if (course.planned_start > problem.snapshot_time + options_.max_incident_in_future) {
                continue;
            }

            stop_count = 0;
            auto end = selected_sched.size() - 1;
            auto start =
                    std::max(1ul, realized_sched.size() - 1);// skip first and last element as there is not stop there
            for (auto i = start; i < end; ++i) {
                auto const &si = selected_sched[i];

                // don't add incidents after threshold
                if (si.departure > problem.snapshot_time + options_.max_incident_in_future) {
                    break;
                }

                if (si.original_activity == Activity::STOP) {
                    stop_map.push_back(i);
                    ++stop_count;
                }
            }

            if (stop_count > 0) {
                cid = course.index;
            }
        }

        cdt.course = cid;
        Course const &course = model_.courses[cid];

        std::uniform_int_distribution<int> stop_dist(0, stop_count - 1);
        auto const &stop_si = course.schedule[stop_map[stop_dist(re_)]];

        cdt.node = stop_si.node;

        seconds_t base_dwell_time = stop_si.departure - stop_si.arrival;
        std::uniform_int_distribution<seconds_t> dwell_dist(base_dwell_time + options_.course_dwell_min_extend,
                                                            base_dwell_time + options_.course_dwell_max_extend);
        cdt.extended_dwell = dwell_dist(re_);
    }
    void ProblemGenerator::generateExtendedDwellTimeIncident(ProblemInstance &problem) {


        auto &edt = problem.extended_dwelltimes.emplace_back();

        node_index_t nid = invalid<node_index_t>();
        std::uniform_int_distribution<int> node_dist(0, model_.nodes.size() - 1);
        while (isInvalid(nid)) {
            auto selected = node_dist(re_);
            auto const &node = model_.nodes[selected];
            if (node.category == NodeCategory::STATION) {
                nid = selected;
            }
        }

        setStartEnd(edt, problem);
        edt.node = nid;

        std::uniform_int_distribution<seconds_t> extended_dwell_dist(options_.extended_dwell_min_time,
                                                                     options_.extended_dwell_max_time);
        edt.extended_dwell = extended_dwell_dist(re_);
    }
    void ProblemGenerator::generateExtendedRunTimeIncident(ProblemInstance &problem) {

        auto &ert = problem.extended_runtimes.emplace_back();

        std::uniform_int_distribution<link_index_t> link_dist(0, model_.links.size() - 1);

        auto lid = link_dist(re_);
        ert.link = lid;

        auto const &link = model_.links[lid];

        std::uniform_int_distribution<seconds_t> runtime_dist(
                link.minimumRuntimes.maximum() * options_.extended_runtime_min_factor,
                link.minimumRuntimes.maximum() * options_.extended_runtime_max_factor);
        ert.extended_runtime = runtime_dist(re_);

        setStartEnd(ert, problem);
    }

    template<typename T>
    void ProblemGenerator::setStartEnd(T &incident, ProblemInstance &problem) {
        seconds_t last_start = problem.snapshot_time + options_.max_incident_in_future - options_.interval_min_time;
        std::uniform_int_distribution<seconds_t> start_dist(problem.snapshot_time, last_start);
        incident.start = start_dist(re_);

        std::uniform_int_distribution<seconds_t> end_dist(
                incident.start + options_.interval_min_time,
                std::min(incident.start + options_.interval_max_time,
                         problem.snapshot_time + options_.max_incident_in_future));
        incident.end = end_dist(re_);
    }

}// namespace fb
