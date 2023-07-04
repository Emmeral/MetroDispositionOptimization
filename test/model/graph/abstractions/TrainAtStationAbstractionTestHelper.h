#pragma once

#include "model/graph/AlternativeGraph.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
namespace fb::graph {


    void checkStopConsidered(AlternativeGraph graph, TrainAtStationAbstraction &tas, seconds_t stop_time);

    void checkLineHeadwayInConsidered(AlternativeGraph graph, TrainAtStationAbstraction &tas, seconds_t stop_time);

    void checkTrackHeadwayInConsidered(AlternativeGraph graph, TrainAtStationAbstraction &tas, seconds_t stop_time);
}// namespace fb::graph
