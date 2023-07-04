#include "Selection.h"

namespace fb::graph {


    Selection::DecisionResult Selection::makeDecision(FullAlternativeIndex fai) {


        if (decisionMade(fai.choice_index)) {
            if (fai.alternative_index == chosenAlternative(fai.choice_index)) {
                return {};// nothing to do
            } else {
                // problem;
                assert(false);
                return {};
            }
        }

        DecisionResult result{};
        result.causes.push_back(fai);


        auto const &choice = graph_->choices[fai.choice_index];
        auto const &alternative = choice.alternative(fai.alternative_index);

        decisions_[2 * choice.index] = true;
        decisions_[2 * choice.index + 1] = static_cast<bool>(fai.alternative_index);
        --remaining_decisions_;

        for (auto edge_index: alternative.edges) {
            edge_selected_[edge_index] = true;
        }
        result.new_edges.insert(result.new_edges.end(), alternative.edges.begin(), alternative.edges.end());

        for (auto impl: alternative.static_implications) {

            if (!decisionMade(impl.choice_index)) {
                // TODO: maybe refactor to remove recursion
                auto nested = makeDecision(impl);
                result.append(nested);
            } else if (chosenAlternative(impl.choice_index) != impl.alternative_index) {
                // this shows that we are in a contradicting state
                // check later for better error handling in this case
                assert(false);
            }
        }
        return std::move(result);
    }
    void Selection::undoDecision(const Selection::DecisionResult &result) {

        for (auto fai: result.causes) {
            assert(decisions_[2 * fai.choice_index]);
            decisions_[2 * fai.choice_index] = false;
            ++remaining_decisions_;
        }
        for (auto edge: result.new_edges) {
            assert(edge_selected_[edge]);
            edge_selected_[edge] = false;
        }
    }
    void Selection::DecisionResult::append(Selection::DecisionResult &other) {
        new_edges.insert(new_edges.end(), other.new_edges.begin(), other.new_edges.end());
        causes.insert(causes.end(), other.causes.begin(), other.causes.end());
    }
}// namespace fb::graph
