
#pragma once

#include "model/definitions.h"
#include "model/enums/Enums.h"
#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <ranges>


namespace fb {

    struct Node;
    typedef u_int8_t track_id_t;
    typedef u_int16_t bsv_t;
    // for better intuition on struct declarations
    typedef index_t node_index_t;
    typedef index_t link_index_t;
    typedef index_t course_index_t;
    typedef index_t duty_index_t;

    struct Headways {

        std::array<seconds_t, 16> headways{0};

        inline seconds_t &of(Activity start_front, Activity end_front, Activity start_behind, Activity end_behind) {
            return headways[static_cast<bool>(start_front) + static_cast<bool>(end_front) * 2 +
                            static_cast<bool>(start_behind) * 4 + static_cast<bool>(end_behind) * 8];
        }
        inline seconds_t const &of(Activity start_front, Activity end_front, Activity start_behind,
                                   Activity end_behind) const {
            return headways[static_cast<bool>(start_front) + static_cast<bool>(end_front) * 2 +
                            static_cast<bool>(start_behind) * 4 + static_cast<bool>(end_behind) * 8];
        }
    };

    struct MinimumRuntimes {

        MinimumRuntimes() = default;
        MinimumRuntimes(seconds_t stopStop, seconds_t stopPass, seconds_t passStop, seconds_t passPass)
            : stop_stop(stopStop), stop_pass(stopPass), pass_stop(passStop), pass_pass(passPass) {}

        seconds_t stop_stop{0};
        seconds_t stop_pass{0};
        seconds_t pass_stop{0};
        seconds_t pass_pass{0};

        seconds_t &of(Activity const &start_activity, Activity const &end_activity);
        seconds_t const &of(Activity const &start_activity, Activity const &end_activity) const;

        seconds_t minWithStart(Activity const &start_activity) const;
        seconds_t minWithEnd(Activity const &end_activity) const;
        seconds_t minimum() const;
        seconds_t maximum() const;
    };

    struct Link {
        link_index_t id;

        node_index_t link_start_node;
        node_index_t link_end_node;

        Direction direction;

        Headways headways{};
        MinimumRuntimes minimumRuntimes{};
    };

    struct Tracks {

        Tracks() : all_tracks{}, eb_tracks{}, wb_tracks{}, both_tracks{} {}
        Tracks(std::initializer_list<Direction> init) : all_tracks(init), eb_tracks{}, wb_tracks{} {
            for (auto i = 0; i < all_tracks.size(); ++i) {
                if (all_tracks[i] == Direction::WB || all_tracks[i] == Direction::BOTH) {
                    wb_tracks.push_back(i);
                }
                if (all_tracks[i] == Direction::EB || all_tracks[i] == Direction::BOTH) {
                    eb_tracks.push_back(i);
                }
                if (all_tracks[i] == Direction::BOTH) {
                    both_tracks.push_back(i);
                }
            }
        }

        std::vector<Direction> all_tracks;
        std::vector<track_id_t> eb_tracks;
        std::vector<track_id_t> wb_tracks;
        std::vector<track_id_t> both_tracks;


        size_t countInDirection(Direction dir) const;

        bool hasDirection(track_id_t track, Direction dir) const;

        std::vector<track_id_t> const &tracksInDirection(Direction dir) const;
    };

    struct Node {
        node_index_t index;
        NodeCategory category;
        Tracks tracks;
        ShortTurn short_turn_eb{ShortTurn::FORBIDDEN};
        ShortTurn short_turn_wb{ShortTurn::FORBIDDEN};
        double lat;
        double lon;

        std::vector<link_index_t> outgoing_links{};
    };

    struct NodeMeta {
        std::string name;
        std::string code;
    };


    struct BaseStationValue {
        // the start and end times are fixed
        static constexpr std::array<seconds_t, 11> BSV_END_TIMES{21900, 25200, 27900, 33300, 36000, 57600,
                                                                 60300, 65700, 68400, 75600, 86400};

        std::array<bsv_t, 11> eb_bsv;
        std::array<bsv_t, 11> wb_bsv;

        bsv_t &of(seconds_t time, Direction dir) {
            // get first time, where the end >= end time
            auto const it = std::ranges::lower_bound(BSV_END_TIMES, time % SECONDS_PER_DAY);
            if (dir == Direction::EB) {
                return eb_bsv[it - BSV_END_TIMES.begin()];
            } else {
                return wb_bsv[it - BSV_END_TIMES.begin()];
            }
        }

        bsv_t of(seconds_t time, Direction dir) const {
            // get first time, where the end >= time
            auto const it = std::ranges::lower_bound(BSV_END_TIMES, time % SECONDS_PER_DAY);
            if (dir == Direction::EB) {
                return eb_bsv[it - BSV_END_TIMES.begin()];
            } else {
                return wb_bsv[it - BSV_END_TIMES.begin()];
            }
        }
    };

    struct TargetFrequencies {

        std::vector<seconds_t> end_times;
        std::vector<seconds_t> headway_seconds;

        seconds_t &of(seconds_t time) {
            // get first time, where the end >= end time
            auto const it = std::ranges::lower_bound(end_times, time % SECONDS_PER_DAY);
            return headway_seconds[it - end_times.begin()];
        }
        seconds_t of(seconds_t time) const {
            // get first time, where the end >= end time
            auto const it = std::ranges::lower_bound(end_times, time % SECONDS_PER_DAY);
            return headway_seconds[it - end_times.begin()];
        }
    };

    struct ScheduleItem {
        index_t index;
        node_index_t node;
        seconds_t arrival;
        seconds_t departure;
        track_id_t original_track;
        Activity original_activity;
    };

    struct Course {
        course_index_t index;

        duty_index_t duty;
        index_t index_in_duty;

        CourseCategory category;
        Direction direction;

        seconds_t planned_start;
        seconds_t planned_end;

        node_index_t start_node;
        node_index_t end_node;

        std::vector<ScheduleItem> schedule;
    };

    struct RSDutyElement {
        index_t index;

        seconds_t planned_start_time;
        seconds_t planned_end_time;

        node_index_t start_node;
        node_index_t end_node;

        EventType event;
        // only set if event = Train
        course_index_t course;
    };

    struct RSDuty {
        duty_index_t index;
        std::vector<RSDutyElement> elements{};

        RSDutyElement const *lastTrainElementBefore(index_t duty_elem) const;
        RSDutyElement const *nextTrainElementAfter(index_t duty_elem) const;
        RSDutyElement const *firstTrainElement() const;


        auto elementsWithType(EventType const &et) const {
            return elements | std::views::filter([et](auto const &elem) { return elem.event == et; });
        }
        auto trainElements() const { return elementsWithType(EventType::TRAIN); }

        RSDutyElement const *changeEndElementBetween(course_index_t first_course, course_index_t second_course) const;
    };

    struct Model {

        std::vector<Node> nodes;
        std::vector<NodeMeta> node_metadata;

        std::vector<Link> links;
        std::vector<link_index_t> reverse_links;

        std::vector<Course> courses;
        std::vector<std::string> course_code;

        std::vector<RSDuty> duties;
        std::vector<std::string> duty_code;

        std::vector<BaseStationValue> base_station_values;

        TargetFrequencies target_frequencies;
        node_index_t eb_frequency_measure;
        node_index_t wb_frequency_measure;

        std::vector<std::vector<std::string>> original_tracks_mapping;
        std::map<std::string, node_index_t> code_to_node_index;

        std::map<std::string, course_index_t> code_to_course_index;

        seconds_t max_headway{0};
        seconds_t max_runtime{0};

        std::vector<node_index_t> fast_change_end{};

        seconds_t changeEndTime(Direction const &incoming, Direction const &outgoing, node_index_t const &node) const;

        track_id_t getTrackIdAtNode(node_index_t node, std::string const &track_code) const;

        Link *getLinkFromTo(node_index_t from, node_index_t to);
        Link *getLinkFromTo(Node const &from, Node const &to);

        Link const *getLinkFromTo(node_index_t from, node_index_t to) const;
        Link const *getLinkFromTo(Node const &from, Node const &to) const;

        Course const *priorCourseInDuty(Course const &course) const;
        Course const *nextCourseInDuty(Course const &course) const;

        inline Course const *priorCourseInDuty(course_index_t const &course) const {
            return priorCourseInDuty(courses[course]);
        }
        inline Course const *nextCourseInDuty(course_index_t const &course) const {
            return nextCourseInDuty(courses[course]);
        }

        seconds_t realArrivalTime(Course const &course, ScheduleItem const &item) const;
        seconds_t realDepartureTime(Course const &course, ScheduleItem const &item) const;
    };
}// namespace fb
