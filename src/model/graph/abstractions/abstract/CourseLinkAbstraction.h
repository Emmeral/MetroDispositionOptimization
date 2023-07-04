#pragma once


#include "model/graph/abstractions/AbstractTrainAtStationAbstraction.h"
#include "model/graph/abstractions/abstract/PredefinedActivityAbstraction.h"
namespace fb::graph {


    class CourseLinkAbstraction : public virtual TrainAtStationAbstraction {

    public:
        CourseLinkAbstraction(const Course &in_course, const Course &outCourse)
            : in_course_(in_course), out_course_(outCourse), in_schedule_item_{in_course.schedule.back()},
              out_schedule_item_{outCourse.schedule.front()} {}

        CourseLinkAbstraction(const Course &inCourse, const Course &outCourse, const ScheduleItem &inScheduleItem,
                              const ScheduleItem &outScheduleItem)
            : in_course_(inCourse), out_course_(outCourse), in_schedule_item_(inScheduleItem),
              out_schedule_item_(outScheduleItem) {}


        const ScheduleItem *getOriginalInScheduleItem() const override { return &in_schedule_item_; }
        const ScheduleItem *getOriginalOutScheduleItem() const override { return &out_schedule_item_; }
        const Course &getInCourse() const override { return in_course_; }
        const Course &getOutCourse() const override { return out_course_; }
        seconds_t getOriginalDeparture() const override { return out_schedule_item_.departure; }
        seconds_t getOriginalArrival() const override { return in_schedule_item_.arrival; }


    protected:
        const Course &in_course_;
        const Course &out_course_;
        const ScheduleItem &in_schedule_item_;
        const ScheduleItem &out_schedule_item_;
    };

}// namespace fb::graph
