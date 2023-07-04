#pragma once

#include "model/graph/abstractions/TrainAtStationAbstraction.h"
namespace fb::graph {


    class PredefinedActivityAbstraction : public virtual TrainAtStationAbstraction {

    public:
        explicit PredefinedActivityAbstraction(Activity predefined_activity)
            : predefined_activity_(predefined_activity) {}


        std::optional<choice_index_t> getStopPassChoice() const final { return std::nullopt; }
        std::optional<Activity> predefinedActivity() const final { return std::make_optional(predefined_activity_); }

    protected:
        Activity predefined_activity_;
    };

}// namespace fb::graph
