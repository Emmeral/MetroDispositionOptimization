
#pragma once


#include "Json.h"
#include <cmath>
#include <string>

namespace fb {
    class CharacteristicStatistic {


    public:
        void newValue(double value);

        long count() const { return count_; }

        double mean() const { return mean_; }

        double sampleVariance() const;
        double standardDeviation() const;

    private:
        double mean_{0};
        long count_{0};
        long double m2_{0};
    };

    template<>
    std::string toJson(const CharacteristicStatistic &t);


}// namespace fb
