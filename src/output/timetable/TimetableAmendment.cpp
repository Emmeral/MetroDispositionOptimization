#include "TimetableAmendment.h"

namespace fb {


    ReTiming::ReTiming(course_index_t course, const ScheduleItem &si, seconds_t new_time)
        : ScheduleItemAmendment(course, si) {
        this->new_time = new_time;
    }
    ScheduleItemAmendment::ScheduleItemAmendment(course_index_t course, const ScheduleItem &si) {
        node = si.node;
        schedule_item = si.index;
        this->course = course;
    }
    std::string ScheduleItemAmendment::baseToTextLine(const Model &model, const std::string &name) const {
        auto const &cc = model.course_code[this->course];
        auto const &nc = model.node_metadata[this->node].code;
        return fmt::format("{}: Course {}/{} ({}) at Node {}", name, cc, this->course, this->schedule_item, nc);
    }
    RePlatforming::RePlatforming(course_index_t course, const ScheduleItem &si, track_id_t new_track)
        : ScheduleItemAmendment(course, si) {
        old_track = si.original_track;
        this->new_track = new_track;
    }
    std::string RePlatforming::toTextLine(const Model &model) const {
        auto base = baseToTextLine(model, "RePlatforming    ");
        auto old_track = model.original_tracks_mapping[this->node][this->old_track];
        auto new_track = model.original_tracks_mapping[this->node][this->new_track];
        return fmt::format("{} : old={} new={}", base, old_track, new_track);
    }
    ArrivalReTiming::ArrivalReTiming(course_index_t course, const ScheduleItem &si, seconds_t new_time)
        : ReTiming(course, si, new_time) {
        old_time = si.arrival;
    }
    std::string ArrivalReTiming::toTextLine(const Model &model) const {
        auto base = baseToTextLine(model, "ArrivalReTiming  ");
        return fmt::format("{}: old={} new={}", base, readable(this->old_time), readable(this->new_time));
    }


    DepartureReTiming::DepartureReTiming(course_index_t course, const ScheduleItem &si, seconds_t new_time)
        : ReTiming(course, si, new_time) {
        old_time = si.departure;
    }
    std::string DepartureReTiming::toTextLine(const Model &model) const {
        auto base = baseToTextLine(model, "DepartureReTiming");
        return fmt::format("{}: old={} new={}", base, readable(this->old_time), readable(this->new_time));
    }
    std::string SkippedStop::toTextLine(const Model &model) const {
        auto base = baseToTextLine(model, "SkippedStop      ");
        return fmt::format("{} : old=STOP new=PASS", base);
    }
}// namespace fb
