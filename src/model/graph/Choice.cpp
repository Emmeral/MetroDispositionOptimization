#include "Choice.h"
#include <sstream>

namespace fb::graph {

    std::ostream &operator<<(std::ostream &stm, const ChoiceType &type) {
        switch (type) {
            case ChoiceType::STOP_PASS:
                stm << "StopPass";
                break;
            case ChoiceType::TRACK:
                stm << "Track";
                break;
            case ChoiceType::LINE_HEADWAY:
                stm << "LineHeadway";
                break;
            case ChoiceType::TRACK_HEADWAY:
                stm << "TrackHeadway";
                break;
            default:
                stm << "Unknown Choice Type";
        }
        return stm;
    }
    std::ostream &operator<<(std::ostream &os, AlternativeIndex alt) {
        switch (alt) {
            case AlternativeIndex::FIRST:
                os << "FIRST";
                break;
            case AlternativeIndex::SECOND:
                os << "SECOND";
                break;
            default:
                os << "Unknown Alternative Index";
        }
        return os;
    }

}// namespace fb::graph

namespace fb {
    template<>
    std::string toJson(const graph::ChoiceType &t) {
        std::ostringstream ss{};
        ss << t;
        return toJson(ss.str());
    }
}// namespace fb
