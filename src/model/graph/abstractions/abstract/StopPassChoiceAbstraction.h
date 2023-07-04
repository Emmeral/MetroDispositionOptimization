#pragma once

#include "model/graph/abstractions/TrainAtStationAbstraction.h"
namespace fb::graph {


    class StopPassChoiceAbstraction : public virtual TrainAtStationAbstraction {
    public:
        std::optional<Activity> predefinedActivity() const final { return std::nullopt; };
        std::optional<choice_index_t> getStopPassChoice() const final { return std::make_optional(stop_pass_choice_); };


    protected:
        graph::choice_index_t stop_pass_choice_{invalid<choice_index_t>()};


        graph::choice_index_t createStopPassChoice(AlternativeGraph &graph) {

            if (isValid(stop_pass_choice_)) {
                return stop_pass_choice_;
            }
            AlternativeIndex original;

            auto const &si = *getOriginalInScheduleItem();
            if (si.original_activity == Activity::STOP) {
                original = AlternativeIndex::FIRST;
            } else {
                original = AlternativeIndex::SECOND;
            }

            stop_pass_choice_ = graph.createChoice<ChoiceType::STOP_PASS>(
                    original, {getNode().index, getInCourse().index, getIndex()});

            return stop_pass_choice_;
        }
    };

}// namespace fb::graph
