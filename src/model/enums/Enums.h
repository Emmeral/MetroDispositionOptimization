#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>
namespace fb {

    enum class Direction : uint8_t { EB, WB, BOTH };
    std::ostream &operator<<(std::ostream &stm, Direction const &dir);
    Direction opposite(Direction d);

    enum class Activity : bool { PASS, STOP };
    std::ostream &operator<<(std::ostream &stm, Activity const &act);
    enum class NodeCategory { STATION, JUNCTION, CONTROL_POINT };
    std::ostream &operator<<(std::ostream &stm, NodeCategory const &nc);
    enum class ShortTurn { FORBIDDEN, SAME_DIRECTION, OPPOSITE_DIRECTION, BOTH, OTHER };
    enum class EventType { TRAIN, CHANGE_END, SPARE, RESERVE };
    std::ostream &operator<<(std::ostream &stm, EventType const &et);
    enum class CourseCategory : bool { PASSENGER, EMPTY };


    inline static std::unordered_map<std::string, Direction> const direction_table{{"WB", Direction::WB},
                                                                                   {"EB", Direction::EB},
                                                                                   {"BOTH", Direction::BOTH}};
    inline static std::unordered_map<std::string, Activity> const activity_table{{"PASS", Activity::PASS},
                                                                                 {"STOP", Activity::STOP}};
    inline static std::unordered_map<std::string, NodeCategory> const node_category_table{
            {"STATION", NodeCategory::STATION},
            {"CONTROL_POINT", NodeCategory::CONTROL_POINT},
            {"JUNCTION", NodeCategory::JUNCTION}};

    inline static std::unordered_map<std::string, ShortTurn> const short_turn_table{
            {"", ShortTurn::FORBIDDEN},
            {"1", ShortTurn::SAME_DIRECTION},
            {"2", ShortTurn::OPPOSITE_DIRECTION},
            {"3", ShortTurn::BOTH},
            {"SA", ShortTurn::OTHER}};

    inline static std::unordered_map<std::string, EventType> const rolling_stock_event_type_table{
            {"RESERVE", EventType::RESERVE},
            {"TRAIN", EventType::TRAIN},
            {"CHANGE_END", EventType::CHANGE_END},
            {"SPARE", EventType::SPARE}};

    inline static std::unordered_map<std::string, CourseCategory> const course_category_table{
            {"OO", CourseCategory::PASSENGER},
            {"EE", CourseCategory::EMPTY}};

}// namespace fb
