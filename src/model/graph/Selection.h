#pragma once
#include "AlternativeGraph.h"
#include "Choice.h"
#include "Edge.h"
#include "Node.h"
#include "model/definitions.h"
#include <vector>

namespace fb::graph {


    class Selection {

    public:
        explicit Selection(const AlternativeGraph &graph)
            : graph_{&graph}, decisions_(graph.choices.size() * 2, false),
              edge_selected_(graph.alternative_edges.size(), false), remaining_decisions_{graph.choices.size()} {}


        struct Decision {
            bool made;
            AlternativeIndex chosen;
        };

        struct DecisionResult {
            std::vector<a_edge_index_t> new_edges{};
            std::vector<FullAlternativeIndex> causes{};

            void append(DecisionResult &other);
            DecisionResult &operator+=(DecisionResult &other) {
                append(other);
                return *this;
            }
            DecisionResult &operator+=(DecisionResult &&other) {
                append(other);
                return *this;
            }
        };


        inline bool isSelectedEdge(a_edge_index_t index) const { return edge_selected_[index]; }

        Decision decisionFor(choice_index_t index) const { return {decisionMade(index), chosenAlternative(index)}; }

        inline bool decisionMade(choice_index_t index) const { return decisions_[2 * index]; }
        inline AlternativeIndex chosenAlternative(choice_index_t index) const {
            return static_cast<AlternativeIndex>(static_cast<bool>(decisions_[2 * index + 1]));
        }
        inline bool isChosen(FullAlternativeIndex fai) const {
            return decisionMade(fai.choice_index) && chosenAlternative(fai.choice_index) == fai.alternative_index;
        }

        inline bool isDenied(FullAlternativeIndex fai) const {
            return decisionMade(fai.choice_index) && chosenAlternative(fai.choice_index) != fai.alternative_index;
        }
        bool decisionMade(fb::graph::Choice const &choice) const { return decisionMade(choice.index); }

        DecisionResult makeDecision(FullAlternativeIndex fai);

        void undoDecision(DecisionResult const &result);


        bool isEmpty() const { return remaining_decisions_ == graph_->choices.size(); }
        bool isPartial() const { return remaining_decisions_ > 0; }
        bool isFull() const { return remaining_decisions_ == 0; }
        size_t madeDecisions() const { return graph_->choices.size() - remaining_decisions_; }
        size_t remainingDecisions() const { return remaining_decisions_; }

        AlternativeGraph const &getGraph() const { return *graph_; }


    private:
        std::vector<bool> edge_selected_;
        std::vector<bool> decisions_;
        size_t remaining_decisions_;

        AlternativeGraph const *graph_;
    };


}// namespace fb::graph
