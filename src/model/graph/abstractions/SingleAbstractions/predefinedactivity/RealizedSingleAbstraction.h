
#pragma once

#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/graph/abstractions/abstract/FullyDecidedAbstraction.h"
#include "model/graph/abstractions/abstract/PredefinedActivityAbstraction.h"
#include "model/graph/abstractions/abstract/SingleScheduleItemAbstraction.h"
#include "model/graph/abstractions/abstract/SingleTrackAbstraction.h"
namespace fb::graph {

    class RealizedSingleAbstraction final : public FullyDecidedAbstraction, public SingleScheduleItemAbstraction {


    public:
        RealizedSingleAbstraction(index_t index, const fb::Node &node, const Course &course,
                                  const ScheduleItem &realized_sched_item);

        RealizedSingleAbstraction(index_t index, const fb::Node &node, const Course &course,
                                  const ScheduleItem &realized_sched_item, TaggedWeight stop_time);


        const ScheduleItem *getRealizedInScheduleItem() const final { return &realized_schedule_item_; }
        const ScheduleItem *getRealizedOutScheduleItem() const final { return &realized_schedule_item_; }


    private:
        ScheduleItem const &realized_schedule_item_;
    };


}// namespace fb::graph
