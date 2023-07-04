
#include "CharacteristicStatistic.h"

namespace fb {

    // see https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
    void CharacteristicStatistic::newValue(double value) {
        ++count_;
        auto delta = value - mean_;
        mean_ += delta / count_;
        auto delta2 = value - mean_;
        m2_ += delta * delta2;
    }
    double CharacteristicStatistic::sampleVariance() const {
        if (count_ < 2) {
            return std::nan("1");
        }
        return (m2_ / (count_ - 1));
    }
    double CharacteristicStatistic::standardDeviation() const { return std::sqrt(sampleVariance()); }


    template<>
    std::string fb::toJson(const CharacteristicStatistic &t) {
        std::string str = "{";
        str += "\"count\": " + toJson(t.count()) + ", ";
        str += "\"mean\": " + toJson(t.mean()) + ", ";
        str += "\"sample_variance\": " + toJson(t.sampleVariance()) + ", ";
        str += "\"standard_deviation\": " + toJson(t.standardDeviation());
        str += "}";
        return str;
    }
}// namespace fb
