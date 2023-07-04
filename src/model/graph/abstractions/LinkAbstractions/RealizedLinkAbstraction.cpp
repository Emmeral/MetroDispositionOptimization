
#include "RealizedLinkAbstraction.h"

namespace fb::graph {
    RealizedLinkAbstraction::RealizedLinkAbstraction(const index_t index, const fb::Node &node, const Course &in_course,
                                                     const Course &out_course, const ScheduleItem &in_schedule_item,
                                                     const ScheduleItem &out_schedule_item,
                                                     const seconds_t change_end_time)
        : RealizedLinkAbstraction(index, node, in_course, out_course, in_schedule_item, out_schedule_item,
                                  TaggedWeight::fixed(change_end_time)) {}


    RealizedLinkAbstraction::RealizedLinkAbstraction(const index_t index, const fb::Node &node, const Course &in_course,
                                                     const Course &out_course,
                                                     const ScheduleItem &realized_in_schedule_item,
                                                     const ScheduleItem &realized_out_schedule_item,
                                                     const TaggedWeight change_end_time)
        : FullyDecidedAbstraction(index, node, change_end_time, realized_in_schedule_item),
          CourseLinkAbstraction(in_course, out_course), realized_in_schedule_item_{realized_in_schedule_item},
          realized_out_schedule_item_{realized_out_schedule_item} {
        // only use change_end_time_ if the departure did not already happen
        if (isValid(realized_out_schedule_item_.departure) && isValid(realized_in_schedule_item_.arrival)) {
            stop_time_ =
                    TaggedWeight::fixed(realized_out_schedule_item_.departure - realized_in_schedule_item_.arrival);
        } else {
            stop_time_ = change_end_time;
        }
    }

}// namespace fb::graph
