
#pragma once

#include "AbstractFCFSSelectionFinder.h"
#include "InitialSelectionFinder.h"
#include "algo/path/LongestPathCalculator.h"
namespace fb {

    class FCFSSelectionFinder : public AbstractFCFSSelectionFinder {

    public:
        explicit FCFSSelectionFinder(const std::vector<RSDuty> &duties);

    private:
        AbsAndState *getMinimum(const graph::GraphWithMeta &gwm, const fb::LongestPathCalculator::State &pc_state,
                                const OnlineTopologicalSorter::State &sorter_state,
                                std::vector<AbsAndState> &next_potential_abs) const;
        void increaseOrEraseChosen(const graph::GraphWithMeta &gwm, std::vector<AbsAndState> &next_potential_abs,
                                   AbsAndState *chosen) const;

    public:
        Result calculateInitialSelectionWithMeta(const graph::GraphWithMeta &gwm) override;
    };

}// namespace fb
