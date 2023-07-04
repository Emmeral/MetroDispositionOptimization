
#include "AbstractFCFSSelectionFinder.h"
#include "FCFSSelectionFinder.h"
#include "algo/OnlineTopologicalSorter.h"
#include "algo/path/LongestPathCalculator.h"
#include "exceptions/CycleException.h"

namespace fb {

    AbstractFCFSSelectionFinder::AbstractFCFSSelectionFinder(const std::vector<RSDuty> &duties) : duties_(duties) {}

    void AbstractFCFSSelectionFinder::setNonOrderingDecisionToOriginal(const graph::AlternativeGraph &graph,
                                                                       graph::Selection &sel) const {
        for (graph::Choice const &choice: graph.choices) {

            // don't select headways from original
            auto const &meta = graph.choice_metadata[choice.index];
            if (meta.type == graph::ChoiceType::LINE_HEADWAY || meta.type == graph::ChoiceType::TRACK_HEADWAY) {
                continue;
            }

            // don't need to update as stop/pass and track don't cause cycles
            auto alt = choice.original;
            auto dr = sel.makeDecision({choice.index, alt});
        }
    }
    graph::Selection::DecisionResult
    AbstractFCFSSelectionFinder::makeDecisionsForChosen(const graph::GraphWithMeta &gwm, graph::Selection &sel,
                                                        const AbsAndState *chosen) const {// make decisions
        auto const &hws = gwm.abstractions.getOutgoingHeadways(chosen->abs);
        std::vector<graph::FullAlternativeIndex> const *relavant_hws;
        if (chosen->state == AbsAndState::State::ARRIVAL) {
            relavant_hws = &hws.track_headways;
        } else {
            relavant_hws = &hws.link_headways;
        }


        graph::Selection::DecisionResult dr{};

        for (graph::FullAlternativeIndex const &hw: *relavant_hws) {
            if (!sel.decisionMade(hw.choice_index)) {
                auto partial = sel.makeDecision(hw);
                dr.append(partial);
            }
        }

        return dr;
    }
    void AbstractFCFSSelectionFinder::setTriviallyImpliedChoices(const graph::AlternativeGraph &graph,
                                                                 graph::Selection &sel) const {
        for (auto const &tc: graph.trivial_implications) {
            sel.makeDecision(tc);
        }
    }
}// namespace fb
