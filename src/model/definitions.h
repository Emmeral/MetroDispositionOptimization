#pragma once


#include "model/enums/Enums.h"
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>


namespace fb {
    typedef uint32_t seconds_t;
    static constexpr seconds_t SECONDS_PER_DAY = 60 * 60 * 24;
    std::string readable(seconds_t sec);

    typedef double coordinate_t;
    typedef uint32_t index_t;


    template<typename T>
    T constexpr invalid() {
        return std::numeric_limits<T>::max();
    }

    template<typename T>
    bool constexpr isValid(T const &t) {
        return t != invalid<T>();
    }

    template<typename T>
    bool constexpr isInvalid(T const &t) {
        return !isValid(t);
    }

    template<typename T>
    T validOrElse(T const &to_test, T const &default_value) {
        return isValid(to_test) ? to_test : default_value;
    }


}// namespace fb
