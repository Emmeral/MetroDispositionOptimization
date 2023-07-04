#include "MaxChoiceStatistics.h"


namespace fb {
    template<>
    std::string toJson(const ChoicePair &t) {
        std::string str = "{";
        str += "\"choice_index\": " + toJson(t.choice_index) + ", ";
        str += "\"count\": " + toJson(t.count) + ", ";
        str += "\"type\": " + toJson(t.type);
        str += "}";
        return str;
    }
}// namespace fb
