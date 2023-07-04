#include "DetailedPenalty.h"
namespace fb {


    std::ostream &operator<<(std::ostream &os, const DetailedPenalty &penalty) {
        os << "delay_penalty: " << penalty.delay_penalty << std::endl;
        os << "skipped_stop_penalty: " << penalty.skipped_stop_penalty << std::endl;
        os << "frequency_penalty: " << penalty.frequency_penalty << std::endl;
        os << "total: " << penalty.total() << std::endl;
        return os;
    }
    template<>
    std::string fb::toJson(const DetailedPenalty &t) {
        std::string s = "{";
        s += "\"delay_penalty\":" + toJson(t.delay_penalty) + ",";
        s += "\"skipped_stop_penalty\":" + toJson(t.skipped_stop_penalty) + ",";
        s += "\"frequency_penalty\":" + toJson(t.frequency_penalty) + ",";
        s += "\"total\":" + toJson(t.total());
        s += "}";
        return s;
    }
}// namespace fb
