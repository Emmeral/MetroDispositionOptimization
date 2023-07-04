#pragma once


#include <optional>
#include <ostream>
namespace fb {


    template<typename T>
    inline std::ostream &operator<<(std::ostream &stm, std::optional<T> const &opt) {
        if (opt) {
            stm << opt.value();
        } else {
            stm << "nullopt{}";
        }
        return stm;
    }

}// namespace fb
