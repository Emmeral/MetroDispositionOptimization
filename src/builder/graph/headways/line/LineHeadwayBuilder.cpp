
#include "LineHeadwayBuilder.h"
#include "HeadwayNodes.h"

#include "builder/graph/headways/HeadwayBuildResult.h"
#include <ranges>

namespace fb {
    LineHeadwayBuilder::LineHeadwayBuilder(graph::AlternativeGraph &graph, graph::Abstractions const &abstractions,
                                           std::vector<distance_t> const &no_constrain_distances_,
                                           const GraphBuilderOptions &options)
        : graph_(graph), abstractions_(abstractions), cached_nodes_(abstractions.size()),
          no_constrain_distances_(no_constrain_distances_), options_(options) {}


    HeadwayBuildResult LineHeadwayBuilder::build(graph::TrainAtStationAbstraction const &first,
                                                 graph::TrainAtStationAbstraction const &second, const Link &link,
                                                 MinHeadways headways) {

        if (isValid(first.getRealizedDeparture()) && isValid(second.getRealizedDeparture())) {
            return buildBothRealized(first, second);
        }
        auto [from_first, from_second] = checkIfFixed(first, second);

        assert(from_first || from_second);


        graph::choice_index_t headway_choice_index = createChoiceIfNeeded(first, second, link, from_first, from_second);


        auto const &first_hw_nodes = getOrCreateHeadwayNodes(first);
        auto const &second_hw_nodes = getOrCreateHeadwayNodes(second);

        auto const *first_next = abstractions_.next(first);
        auto const *second_next = abstractions_.next(second);

        // first to second
        if (from_first) {
            graph::FullAlternativeIndex alt = {headway_choice_index, graph::AlternativeIndex::FIRST};
            createDepartHeadwayEdges(headways, alt, first_hw_nodes, second_hw_nodes);
            createArrivalHeadwayEdges(alt, first_next->getArrivalTimeOutNode(), second_next->getArrivalTimeInNode());
        }

        // second to first
        if (from_second) {
            graph::FullAlternativeIndex alt = {headway_choice_index, graph::AlternativeIndex::SECOND};
            createDepartHeadwayEdges(headways, alt, second_hw_nodes, first_hw_nodes);
            createArrivalHeadwayEdges(alt, second_next->getArrivalTimeOutNode(), first_next->getArrivalTimeInNode());
        }

        if (isValid(headway_choice_index)) {
            return HeadwayBuildResult(headway_choice_index);
        }
        if (from_first) {
            return HeadwayBuildResult(graph::AlternativeIndex::FIRST);
        }
        return HeadwayBuildResult(graph::AlternativeIndex::SECOND);
    }
    graph::choice_index_t LineHeadwayBuilder::createChoiceIfNeeded(const graph::TrainAtStationAbstraction &first,
                                                                   const graph::TrainAtStationAbstraction &second,
                                                                   const Link &link, bool from_first,
                                                                   bool from_second) {
        auto first_dep = first.getOriginalDeparture();
        auto second_dep = second.getOriginalDeparture();

        auto original = static_cast<graph::AlternativeIndex>(second_dep < first_dep);


        auto headway_choice_index = invalid<graph::choice_index_t>();
        if (from_first && from_second) {
            // only create choice if the direction can be decided, otherwise we create fixed edges
            headway_choice_index = graph_.createChoice<graph::ChoiceType::LINE_HEADWAY>(
                    original, {link.id, first.getIndex(), second.getIndex()});
        }
        return headway_choice_index;
    }
    std::pair<bool, bool> LineHeadwayBuilder::checkIfFixed(const graph::TrainAtStationAbstraction &first,
                                                           const graph::TrainAtStationAbstraction &second) const {
        // if one side is already realized we don't have incoming headway edges
        bool from_first = true;
        bool from_second = true;

        auto const first_realized_dep = first.getRealizedDeparture();
        auto const second_realized_dep = second.getRealizedDeparture();

        if (isValid(first_realized_dep)) {
            from_second = false;
        } else if (isValid(second_realized_dep)) {
            from_first = false;
        }

        seconds_t first_departure = no_constrain_distances_[first.getDepartureTimeOutNode()];
        seconds_t second_departure = no_constrain_distances_[second.getDepartureTimeOutNode()];
        auto diff = std::abs(((long) first_departure) - second_departure);

        if (diff > options_.headway_choice_max_difference) {
            if (first_departure > second_departure && !isValid(first_realized_dep)) {
                from_first = false;
            } else if (!isValid(second_realized_dep)) {
                from_second = false;
            }
        }

        return std::make_pair(from_first, from_second);
    }
    HeadwayBuildResult LineHeadwayBuilder::buildBothRealized(graph::TrainAtStationAbstraction const &first,
                                                             graph::TrainAtStationAbstraction const &second) {

        auto const first_realized_dep = first.getRealizedDeparture();
        auto const second_realized_dep = second.getRealizedDeparture();

        auto const *first_next = abstractions_.next(first);
        auto const *second_next = abstractions_.next(second);

        bool first_first_departed = first_realized_dep < second_realized_dep;
        bool first_next_realized = isValid(first_next->getRealizedArrival());
        bool second_next_realized = isValid(second_next->getRealizedArrival());
        bool both_next_not_realized = !first_next_realized && !second_next_realized;

        if (both_next_not_realized) {

            if (first_first_departed) {
                createArrivalHeadwayEdges(invalid<graph::FullAlternativeIndex>(), first_next->getArrivalTimeOutNode(),
                                          second_next->getArrivalTimeInNode());
                return HeadwayBuildResult(graph::AlternativeIndex::FIRST);
            }

            createArrivalHeadwayEdges(invalid<graph::FullAlternativeIndex>(), second_next->getArrivalTimeOutNode(),
                                      first_next->getArrivalTimeInNode());
            return HeadwayBuildResult(graph::AlternativeIndex::SECOND);
        }

        if (!first_next_realized) {
            createArrivalHeadwayEdges(invalid<graph::FullAlternativeIndex>(), second_next->getArrivalTimeOutNode(),
                                      first_next->getArrivalTimeInNode());
            return HeadwayBuildResult(graph::AlternativeIndex::SECOND);
        }
        if (!second_next_realized) {
            createArrivalHeadwayEdges(invalid<graph::FullAlternativeIndex>(), first_next->getArrivalTimeOutNode(),
                                      second_next->getArrivalTimeInNode());
            return HeadwayBuildResult(graph::AlternativeIndex::FIRST);
        }
        // nothing to do if the link is already crossed

        return HeadwayBuildResult();
    }
    void LineHeadwayBuilder::createDepartHeadwayEdges(const MinHeadways &headways,
                                                      graph::FullAlternativeIndex alternative,
                                                      const LineHeadwayBuilder::CachedHeadwayNodes &first_hw_nodes,
                                                      const LineHeadwayBuilder::CachedHeadwayNodes &second_hw_nodes) {

        auto constexpr filter = [](auto const &n) { return isValid(n); };

        for (auto const &first_node: first_hw_nodes.out.nodes | std::views::filter(filter)) {
            for (auto const &second_node: second_hw_nodes.in.nodes | std::views::filter(filter)) {

                auto weight = headways.min(first_node.start_activity, first_node.end_activity,
                                           second_node.start_activity, second_node.end_activity);


                if (second_node.end_activity) {
                    auto w_scnd_end_removed = headways.min(first_node.start_activity, first_node.end_activity,
                                                           second_node.start_activity, {});
                    bool same_weight = w_scnd_end_removed == weight;
                    bool node_exists = isValid(second_hw_nodes.in.get(second_node.start_activity, {}));
                    if (same_weight && node_exists) {// skip edge, if we get the same result with a less specific node
                        ++optimized_edges_count;
                        continue;
                    }
                }
                if (second_node.start_activity) {
                    auto w_scnd_start_removed = headways.min(first_node.start_activity, first_node.end_activity, {},
                                                             second_node.end_activity);

                    bool same_weight = w_scnd_start_removed == weight;
                    bool node_exists = isValid(second_hw_nodes.in.get({}, second_node.end_activity));
                    if (same_weight && node_exists) {
                        ++optimized_edges_count;
                        continue;
                    }
                }
                if (first_node.end_activity) {
                    auto w_fst_end_removed = headways.min(first_node.start_activity, {}, second_node.start_activity,
                                                          second_node.end_activity);
                    bool same_weight = w_fst_end_removed == weight;
                    bool node_exists = isValid(first_hw_nodes.out.get(first_node.start_activity, {}));

                    if (same_weight && node_exists) {
                        ++optimized_edges_count;
                        continue;
                    }
                }

                if (first_node.start_activity) {
                    auto w_fst_start_removed = headways.min({}, first_node.end_activity, second_node.start_activity,
                                                            second_node.end_activity);
                    bool same_weight = w_fst_start_removed == weight;
                    bool node_exists = isValid(first_hw_nodes.out.get({}, first_node.end_activity));
                    if (same_weight && node_exists) {
                        ++optimized_edges_count;
                        continue;
                    }
                }

                if (isValid(alternative.choice_index)) {
                    graph_.createAlternativeEdge(first_node, second_node, alternative, weight);
                } else {
                    graph_.createFixedEdge(first_node, second_node, weight);
                }
            }
        }
    }

    void LineHeadwayBuilder::createArrivalHeadwayEdges(graph::FullAlternativeIndex alternative,
                                                       graph::node_index_t first_node,
                                                       graph::node_index_t second_node) {
        if (isValid(alternative.choice_index)) {
            graph_.createAlternativeEdge(first_node, second_node, alternative, options_.line_arrival_headway);
        } else {
            graph_.createFixedEdge(first_node, second_node, options_.line_arrival_headway);
        }
    }

    void LineHeadwayBuilder::createDirectionBothHeadwayEdges(graph::FullAlternativeIndex alternative,
                                                             graph::node_index_t first_node,
                                                             graph::node_index_t second_node) {
        if (isValid(alternative.choice_index)) {
            graph_.createAlternativeEdge(first_node, second_node, alternative, 30);
        } else {
            graph_.createFixedEdge(first_node, second_node, 30);
        }
    }

    const LineHeadwayBuilder::CachedHeadwayNodes &
    LineHeadwayBuilder::getOrCreateHeadwayNodes(const graph::TrainAtStationAbstraction &tas) {

        auto &cached = cached_nodes_[tas.getIndex()];

        if (!cached.exists) {
            auto const &next = *abstractions_.next(tas);
            cached.in = createHeadwayNodes<HeadwayNodes::Type::IN>(tas, next);
            cached.out = createHeadwayNodes<HeadwayNodes::Type::OUT>(tas, next);
            cached.exists = true;
        }

        return cached;
    }
    HeadwayBuildResult LineHeadwayBuilder::buildForBothLink(const graph::TrainAtStationAbstraction &first,
                                                            const graph::TrainAtStationAbstraction &second,
                                                            const Link &link) {

        auto const first_realized_dep = first.getRealizedDeparture();
        auto const second_realized_dep = second.getRealizedDeparture();

        auto const *first_next = abstractions_.next(first);
        auto const *second_next = abstractions_.next(second);

        if (isValid(first_realized_dep) && isValid(second_realized_dep)) {
            return buildBothRealizedBothLink(first, second);
        }
        auto [from_first, from_second] = checkIfFixed(first, second);

        assert(from_first || from_second);


        graph::choice_index_t headway_choice_index = createChoiceIfNeeded(first, second, link, from_first, from_second);


        // first to second
        if (from_first) {
            graph::FullAlternativeIndex alt = {headway_choice_index, graph::AlternativeIndex::FIRST};
            createDirectionBothHeadwayEdges(alt, first_next->getArrivalTimeOutNode(), second.getDepartureTimeInNode());
        }
        // second to first
        if (from_second) {
            graph::FullAlternativeIndex alt = {headway_choice_index, graph::AlternativeIndex::SECOND};
            createDirectionBothHeadwayEdges(alt, second_next->getArrivalTimeOutNode(), first.getDepartureTimeInNode());
        }

        if (isValid(headway_choice_index)) {
            return HeadwayBuildResult(headway_choice_index);
        }
        if (from_first) {
            return HeadwayBuildResult(graph::AlternativeIndex::FIRST);
        }
        return HeadwayBuildResult(graph::AlternativeIndex::SECOND);
    }
    HeadwayBuildResult LineHeadwayBuilder::buildBothRealizedBothLink(const graph::TrainAtStationAbstraction &first,
                                                                     const graph::TrainAtStationAbstraction &second) {
        auto const first_realized_dep = first.getRealizedDeparture();
        auto const second_realized_dep = second.getRealizedDeparture();

        auto const *first_next = abstractions_.next(first);
        auto const *second_next = abstractions_.next(second);

        bool first_first_departed = first_realized_dep < second_realized_dep;
        bool first_next_realized = isValid(first_next->getRealizedArrival());
        bool second_next_realized = isValid(second_next->getRealizedArrival());
        bool both_next_not_realized = !first_next_realized && !second_next_realized;

        if (both_next_not_realized) {

            if (first_first_departed) {
                createDirectionBothHeadwayEdges(invalid<graph::FullAlternativeIndex>(),
                                                first_next->getArrivalTimeOutNode(), second.getDepartureTimeInNode());
                return HeadwayBuildResult(graph::AlternativeIndex::FIRST);
            }

            createArrivalHeadwayEdges(invalid<graph::FullAlternativeIndex>(), second_next->getArrivalTimeOutNode(),
                                      first.getDepartureTimeOutNode());
            return HeadwayBuildResult(graph::AlternativeIndex::SECOND);
        }

        if (!first_next_realized) {
            createArrivalHeadwayEdges(invalid<graph::FullAlternativeIndex>(), second_next->getArrivalTimeOutNode(),
                                      first.getDepartureTimeInNode());
            return HeadwayBuildResult(graph::AlternativeIndex::SECOND);
        }
        if (!second_next_realized) {
            createArrivalHeadwayEdges(invalid<graph::FullAlternativeIndex>(), first_next->getArrivalTimeOutNode(),
                                      second.getDepartureTimeInNode());
            return HeadwayBuildResult(graph::AlternativeIndex::FIRST);
        }
        // nothing to do if the link is already crossed

        return HeadwayBuildResult();
    }


    template<HeadwayNodes::Type TYPE>
    HeadwayNodes LineHeadwayBuilder::createHeadwayNodes(const graph::TrainAtStationAbstraction &abs,
                                                        const graph::TrainAtStationAbstraction &next) {
        auto nodes = HeadwayNodes{};

        auto &xx = nodes.get(abs.predefinedActivity(), next.predefinedActivity());
        auto &sx = nodes.get(Activity::STOP, next.predefinedActivity());
        auto &px = nodes.get(Activity::PASS, next.predefinedActivity());

        xx.node_index = TYPE == HeadwayNodes::Type::OUT ? abs.getDepartureTimeOutNode() : abs.getDepartureTimeInNode();
        if (abs.hasStopPassChoice()) {

            sx.node_index = graph_.createNode();
            px.node_index = graph_.createNode();
            auto choice_index = abs.getStopPassChoice().value();
            connectNodes<TYPE, Activity::PASS>(xx, px, choice_index);
            connectNodes<TYPE, Activity::STOP>(xx, sx, choice_index);
        }

        if (next.hasStopPassChoice()) {

            for (auto &n: {xx, sx, px}) {
                if (isInvalid(n)) {
                    continue;
                }
                auto &xs = nodes.get(n.start_activity, Activity::STOP);
                auto &xp = nodes.get(n.start_activity, Activity::PASS);
                xp.node_index = graph_.createNode();
                xs.node_index = graph_.createNode();

                auto choice_index = next.getStopPassChoice().value();
                connectNodes<TYPE, Activity::PASS>(n, xp, choice_index);
                connectNodes<TYPE, Activity::STOP>(n, xs, choice_index);
            }
        }


        return nodes;
    }
    template<HeadwayNodes::Type DIR, Activity A>
    void LineHeadwayBuilder::connectNodes(graph::node_index_t inner_node, graph::node_index_t outer_node,
                                          graph::choice_index_t choice) {

        auto first = DIR == HeadwayNodes::Type::OUT ? inner_node : outer_node;
        auto second = DIR == HeadwayNodes::Type::OUT ? outer_node : inner_node;

        if constexpr (A == Activity::PASS) {
            graph_.createPassEdge(first, second, choice);
        } else if constexpr (A == Activity::STOP) {
            graph_.createStopEdge(first, second, choice);
        } else {
            // this should not happen;
            assert(false);
        }
    }
}// namespace fb
