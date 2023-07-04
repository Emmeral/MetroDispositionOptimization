#include "AlternativeGraph.h"
namespace fb::graph {


    graph::node_index_t AlternativeGraph::createNode() {
        auto index = nodes.size();
        auto &node = nodes.emplace_back();
        node.index = index;
        return node.index;
    }
    graph::edge_index_t AlternativeGraph::createFixedEdge(graph::Node &from, graph::Node &to,
                                                          graph::TaggedWeight cost) {

        auto index = fixed_edges.size();
        auto &edge = fixed_edges.emplace_back();
        edge.index = index;
        edge.from = from.index;
        edge.to = to.index;
        edge.weight = cost.weight;
        edge.is_time_dependent = cost.time_dependent;
        edge.is_fixed = true;

        from.outgoing_edges.push_back(index);
        to.incoming_edges.push_back(index);

        return edge.index;
    }
    graph::edge_index_t AlternativeGraph::createAlternativeEdge(graph::Node &from, graph::Node &to,
                                                                FullAlternativeIndex alt, TaggedWeight cost) {
        auto index = alternative_edges.size();
        auto &edge = alternative_edges.emplace_back();
        edge.index = index;
        edge.from = from.index;
        edge.to = to.index;
        edge.weight = cost.weight;
        edge.is_time_dependent = cost.time_dependent;
        edge.is_fixed = false;

        from.outgoing_alternative_edges.push_back(index);
        to.incoming_alternative_edges.push_back(index);
        choices[alt.choice_index].alternative(alt.alternative_index).addEdge(index);
        alternative_edges_metadata.emplace_back(alt);

        return edge.index;
    }
    graph::edge_index_t AlternativeGraph::createFixedEdge(graph::node_index_t from, graph::node_index_t to,
                                                          TaggedWeight cost) {
        assert(to < nodes.size());
        return createFixedEdge(nodes[from], nodes[to], cost);
    }
    graph::edge_index_t AlternativeGraph::createAlternativeEdge(graph::node_index_t from, graph::node_index_t to,
                                                                FullAlternativeIndex alt, TaggedWeight cost) {
        return createAlternativeEdge(nodes[from], nodes[to], alt, cost);
    }
    graph::choice_index_t AlternativeGraph::createChoice(ChoiceType type, AlternativeIndex originalDecision) {
        switch (type) {
            case ChoiceType::STOP_PASS:
                return createChoice<ChoiceType::STOP_PASS>(originalDecision, {});
            case ChoiceType::TRACK_HEADWAY:
                return createChoice<ChoiceType::TRACK_HEADWAY>(originalDecision, {});
            case ChoiceType::LINE_HEADWAY:
                return createChoice<ChoiceType::LINE_HEADWAY>(originalDecision, {});
            case ChoiceType::TRACK:
                return createChoice<ChoiceType::TRACK>(originalDecision, {});
            default:
                return invalid<choice_index_t>();
        }
    }
    graph::edge_index_t AlternativeGraph::createStopEdge(graph::node_index_t from, graph::node_index_t to,
                                                         choice_index_t choice, TaggedWeight cost) {
        return createAlternativeEdge(from, to, FullAlternativeIndex::stopAlternative(choice), cost);
    }
    graph::edge_index_t AlternativeGraph::createPassEdge(graph::node_index_t from, graph::node_index_t to,
                                                         choice_index_t choice, TaggedWeight cost) {
        return createAlternativeEdge(from, to, FullAlternativeIndex::passAlternative(choice), cost);
    }
    const Edge *AlternativeGraph::getNthOutgoingEdge(node_index_t node_id, size_t n) const {
        auto const &node = nodes[node_id];

        if (n < node.outgoing_edges.size()) {
            return &fixed_edges[node.outgoing_edges[n]];
        }

        n -= node.outgoing_edges.size();
        if (n < node.outgoing_alternative_edges.size()) {
            return &alternative_edges[node.outgoing_alternative_edges[n]];
        }
        return nullptr;
    }
    void AlternativeGraph::addTrivialImplication(FullAlternativeIndex fai) { trivial_implications.push_back(fai); }

    template<ChoiceType type, typename META_TYPE>
    graph::choice_index_t AlternativeGraph::createChoice(AlternativeIndex originalDecision, META_TYPE metadata) {
        auto index = choices.size();
        auto &choice = choices.emplace_back(index);
        choice.original = originalDecision;

        choice_metadata.emplace_back(metadata);

        // statistics
        if constexpr (type == ChoiceType::STOP_PASS) {
            ++stop_pass_choice_count;
        } else if constexpr (type == ChoiceType::TRACK) {
            ++track_decision_choice_count;
        } else if constexpr (type == ChoiceType::LINE_HEADWAY) {
            ++line_headway_choice_count;
        } else if constexpr (type == ChoiceType::TRACK_HEADWAY) {
            ++track_headway_choice_count;
        }
        return choice.index;
    }

    template graph::choice_index_t
    AlternativeGraph::createChoice<ChoiceType::STOP_PASS, StopPassMetadata>(AlternativeIndex originalDecision,
                                                                            StopPassMetadata metadata);
    template graph::choice_index_t
    AlternativeGraph::createChoice<ChoiceType::TRACK, TrackMetadata>(AlternativeIndex originalDecision,
                                                                     TrackMetadata metadata);
    template graph::choice_index_t
    AlternativeGraph::createChoice<ChoiceType::LINE_HEADWAY, LineHeadwayMetadata>(AlternativeIndex originalDecision,
                                                                                  LineHeadwayMetadata metadata);
    template graph::choice_index_t
    AlternativeGraph::createChoice<ChoiceType::TRACK_HEADWAY, TrackHeadwayMetadata>(AlternativeIndex originalDecision,
                                                                                    TrackHeadwayMetadata metadata);


}// namespace fb::graph
