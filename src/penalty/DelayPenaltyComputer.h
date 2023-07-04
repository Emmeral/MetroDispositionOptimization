
#pragma once

#include "algo/path/BackwardsPathCalculator.h"
#include "algo/path/definitions.h"
#include "definitions.h"
#include "model/graph/MeasureNodes.h"
#include "model/graph/definitions.h"
#include <vector>
namespace fb {


    class DelayPenaltyComputer {

    public:
        explicit DelayPenaltyComputer(const graph::MeasureNodes &measure_nodes);
        static penalty_t getPenaltyForDelay(distance_t delay);
        penalty_t computeDelayPenalty(const std::vector<distance_t> &distances) const;

        template<size_t N>
        int computeDelayPenaltyChange(graph::AlternativeGraph const &graph,
                                      std::vector<graph::a_edge_index_t> const &new_edges,
                                      std::vector<distance_t> const &distance_from_start,
                                      std::vector<std::array<DistDestPair, N>> const &distances_to_end,
                                      std::vector<distance_t> const &bias = {}) const {


            std::map<node_index_t, distance_t> updated_distances;

            auto const &dfs = distance_from_start;
            for (auto new_edge_i: new_edges) {
                auto const &edge = graph.alternative_edges[new_edge_i];

                int from_start = dfs[edge.from];

                if (from_start == MIN_DISTANCE) {
                    continue;
                }

                int edge_weight = graph.getEdgeWeight(edge, from_start);

                for (DistDestPair const &dist_dest_pair: distances_to_end[edge.to]) {
                    if (!dist_dest_pair.isValid()) {
                        break;
                    }
                    auto sum = from_start + edge_weight + dist_dest_pair.dist;
                    // subtract the bias that was added for the distances to end calculation
                    auto mc_index = measure_nodes_.measureIndexOfNode(dist_dest_pair.dest);
                    if (mc_index < bias.size()) {
                        sum -= bias[mc_index];
                    }


                    if (sum > dfs[dist_dest_pair.dest] && sum > DELAY_THRESHOLD) {

                        // insert if the key does not exist or is lower
                        auto it = updated_distances.find(dist_dest_pair.dest);
                        if (it == updated_distances.end()) {
                            updated_distances[dist_dest_pair.dest] = sum;
                        } else if (it->second < sum) {
                            it->second = sum;
                        }
                    }
                }
            }

            long change = 0;
            for (auto const &[dest, dist]: updated_distances) {
                change -= getPenaltyForDelay(dfs[dest]);
                change += getPenaltyForDelay(dist);
            }
            assert(change < std::numeric_limits<int>::max());
            return change;
        }


    private:
        graph::MeasureNodes const &measure_nodes_;
    };


}// namespace fb
