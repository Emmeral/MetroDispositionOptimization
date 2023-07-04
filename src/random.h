#pragma once
#include "algorithm"
#include "fmt/format.h"
#include "model/definitions.h"
#include "model/main/Model.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/GraphWithMeta.h"
#include "model/input/InputModel.h"
#include "algo/bab/states/DistanceState.h"
#include <map>
#include <set>
#include <queue>


namespace fb {

    struct Event {
        uint32_t time;
        bool isStart;
        input::id_t duty_id;

        auto operator<=>(const Event &) const = default;


        friend std::ostream &operator<<(std::ostream &os, const Event &event) {
            auto const t = event.time;
            auto time_readable = readable(t);

            os << time_readable << ": ";
            if (event.isStart) {
                os << "START";
            } else {
                os << "STOP";
            }
            os << " " << event.duty_id;
            return os;
        }
    };

    void changeEndTimeStats(fb::Model const &model);

    void rolling_stock_analyses(fb::input::InputModel const &data);

    void station_track_connection_analyses(fb::input::InputModel const &data);

    void trackCountInDirection(fb::Model const &model);


    void startNodeTrackCount(fb::Model const &data);

    bool dutyAlwaysStartsWithStop(fb::Model const &data);

    bool dutyKeepsTrackAfterCourseChange(fb::Model const &data);

    fb::seconds_t minRollingStockDutyStart(fb::Model const &data, bool ignoreReserve = true);


    double averageStaticImplicationCount(graph::AlternativeGraph const &graph,
                                         std::vector<graph::ChoiceType> included_types);


    void printCriticalPath(graph::GraphWithMeta const &graph, DistanceState const &distance_state);

}// namespace fb
