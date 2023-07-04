
#pragma once
#include "Choice.h"
#include "Edge.h"
#include "Node.h"
#include "TimeDependentWeight.h"
#include "model/definitions.h"
#include <cassert>
#include <vector>

namespace fb::graph {


    struct AlternativeGraph {
        std::vector<Node> nodes{};
        std::vector<Edge> fixed_edges{};

        std::vector<Edge> alternative_edges{};
        std::vector<FullAlternativeIndex> alternative_edges_metadata{};

        std::vector<TimeDependentWeight> time_dependent_weights{};

        std::vector<Choice> choices{};
        std::vector<TaggedChoiceMetadata> choice_metadata{};

        std::vector<FullAlternativeIndex> trivial_implications{};

        unsigned int stop_pass_choice_count{};
        unsigned int line_headway_choice_count{};
        unsigned int track_headway_choice_count{};
        unsigned int track_decision_choice_count{};

        /**
         * Creates a new node and inserts it in the graph
         * @return a reference to the node
         */
        graph::node_index_t createNode();
        graph::edge_index_t createFixedEdge(graph::node_index_t from, graph::node_index_t to,
                                            graph::TaggedWeight cost = TaggedWeight::fixed(0));
        /**
         * Creates a new fixed edge in the graph
         * @param from the index of the edges origin
         * @param to the index of the edges target
         * @param cost the weight of the edge (default 0)
         * @return the index of the created edge
         */
        graph::edge_index_t createFixedEdge(graph::node_index_t from, graph::node_index_t to, graph::weight_t cost) {
            return createFixedEdge(from, to, TaggedWeight::fixed(cost));
        };

        graph::edge_index_t createAlternativeEdge(graph::node_index_t from, graph::node_index_t to,
                                                  FullAlternativeIndex alt, graph::weight_t cost) {
            return createAlternativeEdge(from, to, alt, TaggedWeight::fixed(cost));
        };
        graph::edge_index_t createAlternativeEdge(graph::node_index_t from, graph::node_index_t to,
                                                  FullAlternativeIndex alt,
                                                  graph::TaggedWeight cost = TaggedWeight::fixed(0));

        graph::edge_index_t createStopEdge(graph::node_index_t from, graph::node_index_t to, choice_index_t choice,
                                           TaggedWeight cost = TaggedWeight::fixed(0));

        graph::edge_index_t createPassEdge(graph::node_index_t from, graph::node_index_t to, choice_index_t choice,
                                           TaggedWeight cost = TaggedWeight::fixed(0));

        graph::choice_index_t createChoice(ChoiceType type, AlternativeIndex original_decision);

        template<ChoiceType type, typename META_TYPE = decltype(getMetadataType<type>())>
        graph::choice_index_t createChoice(AlternativeIndex original_decision, META_TYPE metadata = {});


        void addTrivialImplication(FullAlternativeIndex fai);

        void addStopEdgeToChoice(a_edge_index_t edge_index, choice_index_t choice_index) {
            assert(this->choice_metadata[choice_index].type == ChoiceType::STOP_PASS);
            choices[choice_index].a1.addEdge(edge_index);
        }
        void addPassEdgeToChoice(a_edge_index_t edge_index, choice_index_t choice_index) {
            assert(this->choice_metadata[choice_index].type == ChoiceType::STOP_PASS);
            choices[choice_index].a2.addEdge(edge_index);
        }

        template<typename... Args>
        index_t createTimeDependentWeight(Args &&...args) {
            auto index = time_dependent_weights.size();
            time_dependent_weights.emplace_back(std::forward<Args>(args)...);
            return index;
        }

        inline weight_t getEdgeWeight(Edge const &edge, seconds_t time) const {
            if (!edge.is_time_dependent) {
                return edge.weight.weight;
            }
            auto index = edge.weight.time_dependent_index;
            return time_dependent_weights[index].getWeightAt(time);
        }
        inline weight_t getReverseEdgeWeight(Edge const &edge, seconds_t time_at_from) const {
            if (!edge.is_time_dependent) {
                return edge.weight.weight;
            }
            auto index = edge.weight.time_dependent_index;
            return time_dependent_weights[index].getWeightAt(time_at_from);
        }

        inline Alternative const &alternative(FullAlternativeIndex fai) const {
            return choices[fai.choice_index].alternative(fai.alternative_index);
        }
        inline Alternative &alternative(FullAlternativeIndex fai) {
            return choices[fai.choice_index].alternative(fai.alternative_index);
        }

        inline const Edge *getEdgeFromTo(node_index_t from, node_index_t to) const {
            auto node = nodes[from];

            for (auto edge_i: node.outgoing_edges) {
                auto const &edge = fixed_edges[edge_i];
                if (edge.to == to) {
                    return &edge;
                }
            }
            for (auto edge_i: node.outgoing_alternative_edges) {
                auto const &edge = alternative_edges[edge_i];
                if (edge.to == to) {
                    return &edge;
                }
            }
            return nullptr;
        }

        const Edge *getNthOutgoingEdge(node_index_t node_id, size_t n) const;

    private:
        graph::edge_index_t createAlternativeEdge(graph::Node &from, graph::Node &to, FullAlternativeIndex alt,
                                                  graph::weight_t cost = 0) {
            return createAlternativeEdge(from, to, alt, TaggedWeight::fixed(cost));
        };
        graph::edge_index_t createAlternativeEdge(graph::Node &from, graph::Node &to, FullAlternativeIndex alt,
                                                  TaggedWeight cost = TaggedWeight::fixed(0));
        graph::edge_index_t createFixedEdge(graph::Node &from, graph::Node &to, TaggedWeight cost);
    };
}// namespace fb::graph
