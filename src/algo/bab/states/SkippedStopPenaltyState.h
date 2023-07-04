#pragma once
#include "State.h"
#include "algo/SkippedStopsInfo.h"
#include "algo/bab/StateUpdater.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/main/Model.h"
#include "penalty/definitions.h"
#include <set>
#include <vector>

namespace fb {

    struct SkippedStopPenaltyState {
        std::vector<SkippedStopInfo> skipped_stops{};
        penalty_t skipped_stops_penalty_bound{0};
    };


    template<typename State>
        requires std::derived_from<State, SkippedStopPenaltyState> && std::derived_from<State, CommonState>
    class SkippedStopPenaltyStateUpdater : public StateUpdater<State> {
    public:
        SkippedStopPenaltyStateUpdater(const std::vector<BaseStationValue> &base_station_values,
                                       std::vector<Course> const &courses, graph::Abstractions const &abstractions)
            : base_station_values_(base_station_values), courses_(courses), abstractions_(abstractions) {}

        void update(State &state, graph::Selection::DecisionResult decision_result) const override {

            graph::Selection const &selection = state.selection;
            graph::AlternativeGraph const &graph = selection.getGraph();
            std::vector<SkippedStopInfo> &skipped_stops = state.skipped_stops;

            for (auto const &cause: decision_result.causes) {

                auto const &metadata = graph.choice_metadata[cause.choice_index];

                // nothing can change if it is not a stop pass decision
                if (metadata.type != graph::ChoiceType::STOP_PASS) {
                    continue;
                }


                graph::StopPassMetadata const &stop_pass_metadata = metadata.data.sp;
                auto const &abstraction = abstractions_[stop_pass_metadata.abstraction];

                auto const &si = abstraction.getOriginalInScheduleItem();
                auto const &course = abstraction.getInCourse();

                if (course.category == CourseCategory::EMPTY) {
                    continue;
                }

                bsv_t bsv = base_station_values_[stop_pass_metadata.node].of(si->arrival, course.direction);
                SkippedStopInfo &skipped_stop_info = skipped_stops[course.index];
                auto old_bound = skipped_stop_info.bound;
                bool updated = skipped_stop_info.registerDecisionForBsv(bsv, cause.alternative_index);

                if (updated) {
                    state.skipped_stops_penalty_bound -= old_bound;
                    state.skipped_stops_penalty_bound += skipped_stop_info.bound;
                }
            }
        }

        void initialize(State &state) const override {

            state.skipped_stops.reserve(courses_.size());

            graph::Selection const &selection = state.selection;

            for (auto const &c: courses_) {
                if (c.category == CourseCategory::EMPTY) {
                    auto vec = std::vector<bsv_t>();
                    state.skipped_stops.emplace_back(SkippedStopInfo(vec));// no bsvs as we don't consider empty courses
                } else {
                    state.skipped_stops.emplace_back(initializeForCourse(state, selection, c));
                }
            }

            // calculate intitial bound
            for (SkippedStopInfo const &info: state.skipped_stops) {
                state.skipped_stops_penalty_bound += info.bound;
            }
        }
        SkippedStopInfo initializeForCourse(const State &state, const graph::Selection &selection,
                                            const Course &c) const {

            std::vector<bsv_t> bsvs_of_course{};
            std::vector<std::pair<bsv_t, graph::AlternativeIndex>> decisions;


            graph::TrainAtStationAbstraction const *first_abs = abstractions_.firstAbsOfCourse(c.index);
            for (graph::TrainAtStationAbstraction const *abs = first_abs; abs != nullptr;
                 abs = nextAbsOfCourse(abs, c)) {

                const ScheduleItem *schedule_item = abs->getOriginalInScheduleItem();
                if (schedule_item->original_activity == Activity::PASS) {
                    continue;
                }

                std::optional<graph::choice_index_t> choice = abs->getStopPassChoice();

                // if we don't have a choice, we decided for stop/original activity
                if (!choice.has_value()) {
                    continue;
                }

                BaseStationValue const &bsvs = base_station_values_[abs->getNode().index];
                auto bsv = bsvs.of(schedule_item->arrival, c.direction);

                bsvs_of_course.push_back(bsv);

                auto decision = selection.decisionFor(choice.value());

                if (decision.made) {
                    decisions.emplace_back(bsv, decision.chosen);
                }
            }

            SkippedStopInfo info{bsvs_of_course};

            for (auto const &[bsv, alt]: decisions) {
                info.registerDecisionForBsv(bsv, alt);
            }
            return std::move(info);
        }

        graph::TrainAtStationAbstraction const *nextAbsOfCourse(graph::TrainAtStationAbstraction const *abs,
                                                                Course const &c) const {
            if (abs->getIndex() == abstractions_.courseEnd(c.index)->getIndex()) {
                return nullptr;
            } else {
                return abstractions_.next(*abs);
            }
        }

    private:
        std::vector<Course> const &courses_;
        std::vector<BaseStationValue> const &base_station_values_;
        graph::Abstractions const &abstractions_;
    };


}// namespace fb
