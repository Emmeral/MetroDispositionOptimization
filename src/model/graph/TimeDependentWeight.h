#pragma once


#include "model/graph/Edge.h"
namespace fb::graph {


    struct TimeDependentWeight {

        TimeDependentWeight(weight_t base_weight, weight_t extended_weight, seconds_t start, seconds_t an_end)
            : base_weight(base_weight), extended_weight(extended_weight), start(start), end(an_end) {}

        weight_t base_weight;
        weight_t extended_weight;
        seconds_t start;
        seconds_t end;


        weight_t getWeightAt(seconds_t time) const {

            if (time <= start - base_weight) {
                return base_weight;
            }
            if (time < (end + base_weight) - extended_weight) {
                return extended_weight;
            }
            if (time < end) {
                return (end + base_weight) - time;
            }
            return base_weight;
        }

        weight_t getReverseWeightAt(seconds_t time) const {
            if (time >= end + base_weight) {
                return base_weight;
            }
            if (time > start - base_weight) {// TODO: is this correct?
                return extended_weight;
            }
            return base_weight;
        }
    };


}// namespace fb::graph
