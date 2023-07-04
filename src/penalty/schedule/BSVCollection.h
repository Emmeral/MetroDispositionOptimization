
#pragma once

#include "model/main/Model.h"
namespace fb {

    struct BSVCollection {


        void add(bsv_t bsv);
        uint32_t sum() const;

        bsv_t smallest{0};
        bsv_t second_smallest{0};
        uint32_t remaining_sum{0};
    };

}// namespace fb
