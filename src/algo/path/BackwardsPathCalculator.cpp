
#include "BackwardsPathCalculator.h"

namespace fb {


    template<>
    bool BackwardsPathCalculator::relaxEdge<1U>(const graph::Edge &edge, BackwardsPathState<1> &state,
                                                std::vector<distance_t> const &dist_from_start) {
        auto &at_node = state.distances_to_end[edge.to][0];
        auto &at_prior = state.distances_to_end[edge.from][0];

        auto from_start = 0;
        if (dist_from_start.size() > edge.from) {
            from_start = dist_from_start[edge.from];
        }
        // edge weight is based on distance to start
        auto edge_weight = graph_.getReverseEdgeWeight(edge, from_start);

        if (at_prior.dist < at_node.dist + edge_weight) {
            at_prior.dist = at_node.dist + edge_weight;
            at_prior.dest = at_node.dest;
            return true;
        }
        return false;
    }

}// namespace fb
