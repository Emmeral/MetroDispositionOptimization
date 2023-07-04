
#include "OriginalInitialSelectionFinder.h"
#include "algo/OnlineTopologicalSorter.h"
#include "algo/path/LongestPathCalculator.h"

namespace fb {

    InitialSelectionFinder::Result
    OriginalInitialSelectionFinder::calculateInitialSelectionWithMeta(const graph::GraphWithMeta &gwm) {

        auto const &graph = gwm.graph;
        OnlineTopologicalSorter sorter{graph};
        LongestPathCalculator pathCalculator{graph, gwm.start_node, gwm.measure_nodes.getNodes()};

        graph::Selection sel{graph};
        OnlineTopologicalSorter::State sorter_state = sorter.calculateInitialTopologicalOrder(sel);
        LongestPathCalculator::State pc_state = pathCalculator.calculateInitialPaths(sel, sorter_state);

        for (graph::Choice const &choice: graph.choices) {

            if (sel.decisionMade(choice)) {
                continue;
            }

            auto alt = choice.original;
            auto dr = sel.makeDecision({choice.index, alt});
            sorter.updateTopologicalOrder(sorter_state, sel, dr.new_edges);

            if (sorter_state.cycle) {
                // choose other if original is not possible
                sel.undoDecision(dr);
                dr = sel.makeDecision({choice.index, otherAlternative(alt)});
                // should work as last call did not change anything
                sorter_state.cycle = false;
                sorter.updateTopologicalOrder(sorter_state, sel, dr.new_edges);
            }
            pathCalculator.updateBasedOnNewEdges(pc_state, sel, sorter_state, dr.new_edges);

            if (sorter_state.cycle) {
                std::cout << "Failed to find initial selection based on original decisions\n";
                // we failed to find a solution based on the original decisions
                return Result(graph::Selection{graph});// return empty selection
            }
        }
        return Result(std::move(sel), std::move(pc_state), std::move(sorter_state));
    }


}// namespace fb
