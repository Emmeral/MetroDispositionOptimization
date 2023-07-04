#pragma once


#include "model/graph/abstractions/TrainAtStationAbstraction.h"
namespace fb::graph {


    class NonRealizedAbstraction : public virtual TrainAtStationAbstraction {


    public:
        const ScheduleItem *getRealizedInScheduleItem() const final { return nullptr; }
        const ScheduleItem *getRealizedOutScheduleItem() const final { return nullptr; }
    };
}// namespace fb::graph
