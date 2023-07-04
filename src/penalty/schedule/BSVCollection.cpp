
#include "BSVCollection.h"
#include "penalty/definitions.h"

namespace fb {
    void BSVCollection::add(bsv_t bsv) {

        if (bsv < smallest) {
            remaining_sum += second_smallest;
            second_smallest = smallest;
            smallest = bsv;
        } else if (bsv < second_smallest) {
            remaining_sum += second_smallest;
            second_smallest = bsv;
        } else {
            remaining_sum += bsv;
        }
    }
    uint32_t BSVCollection::sum() const {
        return smallest * BIGGEST_SFF + second_smallest * SECOND_BIGGEST_SFF + remaining_sum * DEFAULT_SFF;
    }
}// namespace fb
