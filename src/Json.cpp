
#include "Json.h"

namespace fb {

    template<>
    std::string toJson(const std::string &t) {
        return "\"" + t + "\"";
    }

    template<>
    std::string toJson(const bool &t) {
        return t ? "true" : "false";
    }
}// namespace fb
