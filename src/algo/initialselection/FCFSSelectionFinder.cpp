
#include "FCFSSelectionFinder.h"
#include "AbstractFCFSSelectionFinder.h"
#include "algo/OnlineTopologicalSorter.h"
#include "algo/path/LongestPathCalculator.h"
#include "exceptions/CycleException.h"

namespace fb {


    InitialSelectionFinder::Result
    FCFSSelectionFinder::calculateInitialSelectionWithMeta(const graph::GraphWithMeta &gwm) {

        auto const &graph = gwm.graph;
        OnlineTopologicalSorter sorter{graph};
        LongestPathCalculator pathCalculator{graph, gwm.start_node, gwm.measure_nodes.getNodes()};

        graph::Selection sel{graph};


        setTriviallyImpliedChoices(graph, sel);
        setNonOrderingDecisionToOriginal(graph, sel);

        OnlineTopologicalSorter::State sorter_state = sorter.calculateInitialTopologicalOrder(sel);
        LongestPathCalculator::State pc_state = pathCalculator.calculateInitialPaths(sel, sorter_state);


        std::vector<AbsAndState> next_potential_abs{};
        for (auto const &d: duties_) {

            auto first_abs = invalid<graph::abs_index_t>();
            for (auto const &de: d.elements) {
                if (de.event != EventType::TRAIN) {
                    continue;
                }
                auto const *abs = gwm.abstractions.firstAbsOfCourse(de.course);
                if (abs != nullptr) {
                    first_abs = abs->getIndex();
                    break;
                }
            }

            if (isValid(first_abs)) {
                next_potential_abs.push_back({first_abs, AbsAndState::State::ARRIVAL});
            }
        }

        auto last = sel.remainingDecisions();
        while (!next_potential_abs.empty()) {

            if (last - sel.remainingDecisions() > 1000) {
                last = sel.remainingDecisions();
                std::cout << "Remaining decisions: " << sel.remainingDecisions() << "      \r" << std::flush;
            }


            AbstractFCFSSelectionFinder::AbsAndState *chosen =
                    getMinimum(gwm, pc_state, sorter_state, next_potential_abs);

            graph::Selection::DecisionResult dr = makeDecisionsForChosen(gwm, sel, chosen);

            // update state
            sorter.updateTopologicalOrder(sorter_state, sel, dr.new_edges);

            if (sorter_state.cycle) {
                std::cout << "Cycle during construction\n";
                throw CycleException("Cycle during construction", std::move(sel));
            }
            pathCalculator.updateBasedOnNewEdges(pc_state, sel, sorter_state, dr.new_edges);

            increaseOrEraseChosen(gwm, next_potential_abs, chosen);
        }

        assert(sel.isFull());

        return Result(std::move(sel), std::move(pc_state), std::move(sorter_state));
    }
    void FCFSSelectionFinder::increaseOrEraseChosen(const graph::GraphWithMeta &gwm,
                                                    std::vector<AbsAndState> &next_potential_abs,
                                                    AbsAndState *chosen) const {// increase chosen
        if (chosen->state == AbsAndState::State::ARRIVAL) {
            chosen->state = AbsAndState::State::DEPARTURE;
        } else {
            auto const *next = gwm.abstractions.next(chosen->abs);
            if (next != nullptr) {
                chosen->abs = next->getIndex();
                chosen->state = AbsAndState::State::ARRIVAL;
            } else {
                next_potential_abs.erase(next_potential_abs.begin() + std::distance(next_potential_abs.data(), chosen));
            }
        }
    }
    AbstractFCFSSelectionFinder::AbsAndState *
    FCFSSelectionFinder::getMinimum(const graph::GraphWithMeta &gwm, const LongestPathCalculator::State &pc_state,
                                    const OnlineTopologicalSorter::State &sorter_state,
                                    std::vector<AbsAndState> &next_potential_abs) const {// choose minimum
        auto min_time = invalid<distance_t>();
        auto min_sort_order = invalid<order_t>();
        AbsAndState *chosen = nullptr;
        for (auto &pot: next_potential_abs) {
            graph::TrainAtStationAbstraction const &abs = gwm.abstractions[pot.abs];


            graph::node_index_t node;
            seconds_t original_time;
            if (pot.state == AbsAndState::State::ARRIVAL) {
                node = abs.getArrivalTimeOutNode();
                original_time = abs.getOriginalInScheduleItem()->arrival;
            } else {
                node = abs.getDepartureTimeOutNode();
                original_time = abs.getOriginalInScheduleItem()->departure;
            }

            distance_t time = pc_state.distance_from_start[node];
            order_t order = sorter_state.topological_order[node];

            //time = std::max(time, (distance_t) original_time);

            if (time < min_time || (time == min_time && order < min_sort_order)) {
                min_time = time;
                min_sort_order = order;
                chosen = &pot;
            }
        }
        return chosen;
    }
    FCFSSelectionFinder::FCFSSelectionFinder(const std::vector<RSDuty> &duties) : AbstractFCFSSelectionFinder(duties) {}

}// namespace fb
