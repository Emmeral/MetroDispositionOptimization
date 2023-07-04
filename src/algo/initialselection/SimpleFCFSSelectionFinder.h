
#pragma once

#include "AbstractFCFSSelectionFinder.h"
#include "InitialSelectionFinder.h"
namespace fb {

    class SimpleFCFSSelectionFinder : public AbstractFCFSSelectionFinder {

    public:
        explicit SimpleFCFSSelectionFinder(const std::vector<RSDuty> &duties);


        Result calculateInitialSelectionWithMeta(const graph::GraphWithMeta &gwm) override;

    private:
        struct AbsAndStateAndValue {
            AbsAndState abs_and_state;
            distance_t value;

            bool operator<(const AbsAndStateAndValue &other) const;
        };
    };

}// namespace fb
