#pragma once


#include "model/graph/abstractions/AbstractTrainAtStationAbstraction.h"
namespace fb::graph {

    class SingleScheduleItemAbstraction : public virtual TrainAtStationAbstraction {


    public:
        SingleScheduleItemAbstraction(const Course &course, const ScheduleItem &scheduleItem)
            : course_(course), schedule_item_(scheduleItem) {}


        SingleScheduleItemAbstraction(const Course &course, index_t schedule_item_index)
            : SingleScheduleItemAbstraction(course, course.schedule[schedule_item_index]) {}


        const ScheduleItem *getOriginalInScheduleItem() const override final { return &schedule_item_; }
        const ScheduleItem *getOriginalOutScheduleItem() const override final { return &schedule_item_; }
        const Course &getInCourse() const override final { return course_; }
        const Course &getOutCourse() const override final { return course_; }

        seconds_t getOriginalDeparture() const override {
            if (isInvalid(schedule_item_.departure) && schedule_item_.index == course_.schedule.size() - 1) {
                return schedule_item_.arrival;
            }
            return schedule_item_.departure;
        }
        seconds_t getOriginalArrival() const override {
            if (isInvalid(schedule_item_.arrival) && schedule_item_.index == 0) {
                return schedule_item_.departure;
            }
            return schedule_item_.arrival;
        }


    protected:
        Course const &course_;
        /**
         * The schedule item as it is found in the original schedule
         */
        ScheduleItem const &schedule_item_;
    };


}// namespace fb::graph
