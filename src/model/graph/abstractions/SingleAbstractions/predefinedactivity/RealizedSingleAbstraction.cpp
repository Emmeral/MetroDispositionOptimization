
#include "RealizedSingleAbstraction.h"

namespace fb::graph {
    RealizedSingleAbstraction::RealizedSingleAbstraction(const index_t index, const fb::Node &node,
                                                         const Course &course, const ScheduleItem &realized_sched_item)
        : FullyDecidedAbstraction(index, node, TaggedWeight::fixed(0), realized_sched_item),
          SingleScheduleItemAbstraction(course, course.schedule[realized_sched_item.index]),
          realized_schedule_item_{realized_sched_item} {


        auto dwell_time = 0u;
        // only use the schedules dwell time if the departure has not already happened
        if (isValid(realized_schedule_item_.departure) && isValid(realized_schedule_item_.arrival)) {
            dwell_time = realized_schedule_item_.departure - realized_schedule_item_.arrival;
        } else if (realized_schedule_item_.original_activity == Activity::STOP && isValid(schedule_item_.departure) &&
                   isValid(schedule_item_.arrival)) {
            // use original dwell time, but only if original has arrival and departure set. Otherwise, there is no dwell time
            dwell_time = schedule_item_.departure - schedule_item_.arrival;
        }
        stop_time_ = TaggedWeight::fixed(dwell_time);
    }

    RealizedSingleAbstraction::RealizedSingleAbstraction(index_t index, const fb::Node &node, const Course &course,
                                                         const ScheduleItem &realized_sched_item,
                                                         TaggedWeight stop_time)
        : FullyDecidedAbstraction(index, node, stop_time, realized_sched_item),
          SingleScheduleItemAbstraction(course, course.schedule[realized_sched_item.index]),
          realized_schedule_item_{realized_sched_item} {

        // overwrite given stop_time if the stop time is already in the past
        if (isValid(realized_schedule_item_.departure) && isValid(realized_schedule_item_.arrival)) {
            stop_time_ = TaggedWeight::fixed(realized_schedule_item_.departure - realized_schedule_item_.arrival);
        }
        // also use 0 stop time if we passed the stop
        if (realized_sched_item.original_activity == Activity::PASS) {
            stop_time_ = TaggedWeight::fixed(0);
        }
    }



}// namespace fb::graph
