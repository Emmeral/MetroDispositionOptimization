
#pragma once

#include "model/definitions.h"
#include <optional>
namespace fb {

    class MinHeadways {

        static unsigned int inline toIndex(std::optional<Activity> act) {
            return act ? static_cast<bool>(act.value()) : 2;
        }

    public:
        explicit MinHeadways() : headways_{0} {};
        explicit MinHeadways(std::array<seconds_t, 16> const &hws);

        inline const seconds_t &of(Activity start_front, Activity end_front, Activity start_behind,
                                   Activity end_behind) {
            return headways_[static_cast<bool>(start_front) + static_cast<bool>(end_front) * 3 +
                             static_cast<bool>(start_behind) * 9 + static_cast<bool>(end_behind) * 27];
        }

        inline seconds_t const &min(std::optional<Activity> start_front = std::nullopt,
                                    std::optional<Activity> end_front = std::nullopt,
                                    std::optional<Activity> start_behind = std::nullopt,
                                    std::optional<Activity> end_behind = std::nullopt) const {
            return headways_[toIndex(start_front) + toIndex(end_front) * 3 + toIndex(start_behind) * 9 +
                             toIndex(end_behind) * 27];
        };

    private:
        std::array<seconds_t, 81> headways_{0};
    };


}// namespace fb
