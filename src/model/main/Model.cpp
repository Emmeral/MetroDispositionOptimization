
#include "Model.h"

namespace fb {


    seconds_t &MinimumRuntimes::of(const Activity &start_activity, const Activity &end_activity) {
        if (start_activity == Activity::PASS) {
            if (end_activity == Activity::PASS) {
                return pass_pass;
            } else {
                return pass_stop;
            }
        } else {
            if (end_activity == Activity::PASS) {
                return stop_pass;
            } else {
                return stop_stop;
            }
        }
    }
    seconds_t const &MinimumRuntimes::of(const Activity &start_activity, const Activity &end_activity) const {
        if (start_activity == Activity::PASS) {
            if (end_activity == Activity::PASS) {
                return pass_pass;
            } else {
                return pass_stop;
            }
        } else {
            if (end_activity == Activity::PASS) {
                return stop_pass;
            } else {
                return stop_stop;
            }
        }
    }
    seconds_t MinimumRuntimes::minWithStart(const Activity &start_activity) const {
        if (start_activity == Activity::STOP) {
            return std::min(stop_stop, stop_pass);
        } else {
            return std::min(pass_stop, pass_pass);
        }
    }
    seconds_t MinimumRuntimes::minWithEnd(const Activity &end_activity) const {
        if (end_activity == Activity::STOP) {
            return std::min(pass_stop, stop_stop);
        } else {
            return std::min(pass_pass, stop_pass);
        }
    }
    seconds_t MinimumRuntimes::minimum() const { return std::min({stop_stop, stop_pass, pass_stop, pass_pass}); }
    seconds_t MinimumRuntimes::maximum() const { return std::max({stop_stop, stop_pass, pass_stop, pass_pass}); }


    size_t Tracks::countInDirection(Direction dir) const {

        switch (dir) {
            case Direction::WB:
                return wb_tracks.size();
            case Direction::EB:
                return eb_tracks.size();
            case Direction::BOTH:
                return both_tracks.size();
            default:
                return 0;
        }
    }
    bool Tracks::hasDirection(track_id_t track, Direction dir) const {
        return all_tracks[track] == dir || all_tracks[track] == Direction::BOTH;
    }
    std::vector<track_id_t> const &Tracks::tracksInDirection(Direction dir) const {

        if (dir == Direction::EB) {
            return eb_tracks;
        }
        if (dir == Direction::WB) {
            return wb_tracks;
        }
        if (dir == Direction::BOTH) {
            return both_tracks;
        }
        throw std::runtime_error("Impossible direction chosen");
    }

    Link *Model::getLinkFromTo(const Node &from, const Node &to) {
        return const_cast<Link *>(const_cast<const Model *>(this)->getLinkFromTo(from, to));
    }

    Link *Model::getLinkFromTo(node_index_t from, node_index_t to) { return getLinkFromTo(nodes[from], nodes[to]); }

    Link const *Model::getLinkFromTo(node_index_t from, node_index_t to) const {
        return getLinkFromTo(nodes[from], nodes[to]);
    }
    Link const *Model::getLinkFromTo(const Node &from, const Node &to) const {

        for (auto &link_id: from.outgoing_links) {
            Link const &link = links[link_id];
            if (link.link_end_node == to.index) {
                return &link;
            }
        }

        return nullptr;
    }
    track_id_t Model::getTrackIdAtNode(node_index_t node, std::string const &track_code) const {

        auto vec = original_tracks_mapping[node];


        auto it = std::find(vec.begin(), vec.end(), track_code);

        if (it == vec.end()) {
            return invalid<track_id_t>();
        }
        return it - vec.begin();
    }
    Course const *Model::priorCourseInDuty(const Course &course) const {
        auto const *de = duties[course.duty].lastTrainElementBefore(course.index_in_duty);
        if (de == nullptr) {
            return nullptr;
        }
        return &courses[de->course];
    }
    Course const *Model::nextCourseInDuty(const Course &course) const {
        auto const *de = duties[course.duty].nextTrainElementAfter(course.index_in_duty);
        if (de == nullptr) {
            return nullptr;
        }
        return &courses[de->course];
    }
    seconds_t Model::changeEndTime(const Direction &incoming, const Direction &outgoing,
                                   const node_index_t &node) const {
        if (incoming != outgoing) {
            return 420;
        }

        if (std::find(fast_change_end.begin(), fast_change_end.end(), node) != fast_change_end.end()) {
            return 30;
        }
        return 60;
    }
    seconds_t Model::realArrivalTime(const Course &course, const ScheduleItem &item) const {
        if (item.index > 0) {
            return item.arrival;
        }
        auto const *prior_course = priorCourseInDuty(course);
        if (prior_course == nullptr) {
            return item.departure;
        }
        return prior_course->schedule.back().arrival;
    }
    seconds_t Model::realDepartureTime(const Course &course, const ScheduleItem &item) const {
        if (item.index < course.schedule.size() - 1) {
            return item.departure;
        }
        auto const *next_course = nextCourseInDuty(course);
        if (next_course == nullptr) {
            return item.arrival;
        }
        return next_course->schedule.front().departure;
    }
    RSDutyElement const *RSDuty::lastTrainElementBefore(index_t duty_elem) const {
        if (duty_elem == 0) {
            return nullptr;
        }
        for (auto i = duty_elem - 1; i >= 0; --i) {
            if (elements[i].event == EventType::TRAIN) {
                return &elements[i];
            }
        }
        return nullptr;
    }
    RSDutyElement const *RSDuty::nextTrainElementAfter(index_t duty_elem) const {
        for (auto i = duty_elem + 1; i < elements.size(); ++i) {
            if (elements[i].event == EventType::TRAIN) {
                return &elements[i];
            }
        }
        return nullptr;
    }
    RSDutyElement const *RSDuty::firstTrainElement() const {
        for (const auto &element: elements) {
            if (element.event == EventType::TRAIN) {
                return &element;
            }
        }
        return nullptr;
    }
    RSDutyElement const *RSDuty::changeEndElementBetween(course_index_t first_course,
                                                         course_index_t second_course) const {

        index_t first_index = invalid<index_t>();
        index_t second_index = invalid<index_t>();
        for (auto const &e: trainElements()) {

            if (isValid(first_index)) {
                if (e.course != second_course) {
                    return nullptr;
                } else {
                    second_index = e.index;
                    break;
                }
            }

            if (e.course == first_course) {
                first_index = e.index;
            }
        }

        if (isInvalid(first_index)) {
            return nullptr;
        }

        for (auto i = first_index + 1; i < second_index; ++i) {
            if (elements[i].event == EventType::CHANGE_END) {
                return &elements[i];
            }
        }
        return nullptr;
    }
}// namespace fb
