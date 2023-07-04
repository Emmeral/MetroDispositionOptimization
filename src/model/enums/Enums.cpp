#include "Enums.h"

namespace fb {


    std::ostream &operator<<(std::ostream &stm, const Direction &dir) {
        switch (dir) {
            case Direction::WB:
                return stm << "WB";
            case Direction::EB:
                return stm << "EB";
            case Direction::BOTH:
                return stm << "BOTH";
            default:
                return stm << "Direction{" << uint8_t(dir) << '}';
        }
    }

    Direction opposite(Direction d) {
        switch (d) {
            case Direction::WB:
                return Direction::EB;
            case Direction::EB:
                return Direction::WB;
            case Direction::BOTH:
                return Direction::BOTH;
            default:
                return Direction::BOTH;
        }
    }


    std::ostream &operator<<(std::ostream &stm, const Activity &act) {
        switch (act) {
            case Activity::STOP:
                return stm << "STOP";
            case Activity ::PASS:
                return stm << "PASS";

            default:
                return stm << "Activity{" << uint8_t(act) << '}';
        }
    }
    std::ostream &operator<<(std::ostream &stm, const EventType &et) {
        switch (et) {
            case EventType::TRAIN:
                return stm << "TRAIN";
            case EventType::CHANGE_END:
                return stm << "CHANGE_END";
            case EventType::SPARE:
                return stm << "SPARE";
            case EventType::RESERVE:
                return stm << "RESERVE";
            default:
                return stm << "EventType{" << uint8_t(et) << '}';
        }
    }
    std::ostream &operator<<(std::ostream &stm, const NodeCategory &nc) {
        switch (nc) {
            case NodeCategory::STATION:
                return stm << "STATION";
            case NodeCategory::JUNCTION:
                return stm << "JUNCTION";
            case NodeCategory::CONTROL_POINT:
                return stm << "CONTROL_POINT";
            default:
                return stm << "NodeCategory{" << uint8_t(nc) << '}';
        }
    }
}// namespace fb
