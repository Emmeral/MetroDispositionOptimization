
#include "SimpleFCFSSelectionFinder.h"
#include "algo/path/LongestPathCalculator.h"
#include "algo/path/definitions.h"

namespace fb {


    SimpleFCFSSelectionFinder::SimpleFCFSSelectionFinder(const std::vector<RSDuty> &duties)
        : AbstractFCFSSelectionFinder(duties) {}

    InitialSelectionFinder::Result
    SimpleFCFSSelectionFinder::calculateInitialSelectionWithMeta(const graph::GraphWithMeta &gwm) {
        auto const &graph = gwm.graph;
        OnlineTopologicalSorter sorter{graph};
        LongestPathCalculator pathCalculator{graph, gwm.start_node, gwm.measure_nodes.getNodes()};

        graph::Selection sel{graph};


        setTriviallyImpliedChoices(graph, sel);
        setNonOrderingDecisionToOriginal(graph, sel);

        OnlineTopologicalSorter::State sorter_state = sorter.calculateInitialTopologicalOrder(sel);
        LongestPathCalculator::State pc_state = pathCalculator.calculateInitialPaths(sel, sorter_state);


        std::vector<AbsAndStateAndValue> sorted_abs{};

        for (auto const &abs: gwm.abstractions) {

            for (auto const &state: {AbsAndState::State::ARRIVAL, AbsAndState::State::DEPARTURE}) {

                graph::node_index_t node;
                if (state == AbsAndState::State::ARRIVAL) {
                    node = abs->getArrivalTimeOutNode();
                } else {
                    node = abs->getDepartureTimeOutNode();
                }

                distance_t time = pc_state.distance_from_start[node];
                sorted_abs.push_back({{abs->getIndex(), state}, time});
            }
        }

        std::sort(sorted_abs.begin(), sorted_abs.end());

        for (auto const &asv: sorted_abs) {
            auto dr = makeDecisionsForChosen(gwm, sel, &asv.abs_and_state);
        }

        sorter_state = sorter.calculateInitialTopologicalOrder(sel);
        if (sorter_state.cycle) {
            // strategy can find cycles. So return empty if this is the case
            return Result(graph::Selection{graph});
        }
        // can cause cycles, but we are a simple strategy, so we don't care
        pc_state = pathCalculator.calculateInitialPaths(sel, sorter_state);

        return Result(std::move(sel), std::move(pc_state), std::move(sorter_state));
    }


    bool SimpleFCFSSelectionFinder::AbsAndStateAndValue::operator<(
            const SimpleFCFSSelectionFinder::AbsAndStateAndValue &other) const {
        if (value != other.value) {
            return value < other.value;
        }
        if (abs_and_state.state != other.abs_and_state.state) {
            return abs_and_state.state < other.abs_and_state.state;
        }
        return abs_and_state.abs < other.abs_and_state.abs;
    }
}// namespace fb
