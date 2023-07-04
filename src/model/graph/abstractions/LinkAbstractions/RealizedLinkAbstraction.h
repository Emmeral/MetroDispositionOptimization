
#pragma once

#include "model/graph/abstractions/abstract/CourseLinkAbstraction.h"
#include "model/graph/abstractions/abstract/FullyDecidedAbstraction.h"
#include "model/graph/abstractions/abstract/SingleTrackAbstraction.h"
namespace fb {
    namespace graph {

        class RealizedLinkAbstraction : public FullyDecidedAbstraction, public CourseLinkAbstraction {

        public:
            RealizedLinkAbstraction(const index_t index, const fb::Node &node, const Course &in_course,
                                    const Course &out_course, const ScheduleItem &realized_in_schedule_item,
                                    const ScheduleItem &realized_out_schedule_item, const seconds_t change_end_time);

            RealizedLinkAbstraction(const index_t index, const fb::Node &node, const Course &in_course,
                                    const Course &out_course, const ScheduleItem &realized_in_schedule_item,
                                    const ScheduleItem &realized_out_schedule_item, const TaggedWeight change_end_time);


            const ScheduleItem *getRealizedInScheduleItem() const final { return &realized_in_schedule_item_; }
            const ScheduleItem *getRealizedOutScheduleItem() const final { return &realized_out_schedule_item_; }

        private:
            ScheduleItem const &realized_in_schedule_item_;
            ScheduleItem const &realized_out_schedule_item_;
        };

    }// namespace graph
}// namespace fb
