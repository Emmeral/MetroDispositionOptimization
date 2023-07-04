
#pragma once

#include "model/graph/abstractions/AbstractTrainAtStationAbstraction.h"
#include "model/graph/abstractions/abstract/FullyDecidedAbstraction.h"
#include "model/graph/abstractions/abstract/NonRealizedAbstraction.h"
#include "model/graph/abstractions/abstract/PredefinedActivityAbstraction.h"
#include "model/graph/abstractions/abstract/SingleScheduleItemAbstraction.h"
#include "model/graph/abstractions/abstract/SingleTrackAbstraction.h"
namespace fb {
    namespace graph {

        class FullyDecidedSingleAbstraction : public SingleScheduleItemAbstraction,
                                              public FullyDecidedAbstraction,
                                              public NonRealizedAbstraction {

        public:
            FullyDecidedSingleAbstraction(const index_t index, const fb::Node &node, const Course &course,
                                          const ScheduleItem &schedule_item,
                                          const TaggedWeight &stop_time = TaggedWeight::fixed(0))
                : FullyDecidedAbstraction(index, node, stop_time, schedule_item),
                  SingleScheduleItemAbstraction(course, schedule_item) {}
        };

    }// namespace graph
}// namespace fb
