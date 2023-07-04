
#pragma once

#include "HeadwayNodes.h"
#include "MinHeadways.h"
#include "algo/path/definitions.h"
#include "builder/graph/headways/HeadwayBuildResult.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/abstractions/Abstractions.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "options/GraphBuilderOptions.h"
namespace fb {

    class LineHeadwayBuilder {

    public:
        LineHeadwayBuilder(graph::AlternativeGraph &graph, graph::Abstractions const &abstractions,
                           std::vector<distance_t> const &no_constrain_distances_,
                           const GraphBuilderOptions &options = GraphBuilderOptions());


        /**
         * Used to build the headways. This function can use cached min headways
         * @param first
         * @param second
         * @param link
         * @param headways
         * @return the index of the created choice
         */
        HeadwayBuildResult build(graph::TrainAtStationAbstraction const &first,
                                 graph::TrainAtStationAbstraction const &second, const Link &link,
                                 MinHeadways headways);

        /**
         * Used to build the headways. This function generates MinHeadways on every invocation.
         */
        HeadwayBuildResult build(graph::TrainAtStationAbstraction const &first,
                                 graph::TrainAtStationAbstraction const &second, const Link &link) {
            return build(first, second, link, MinHeadways{link.headways.headways});
        }

        HeadwayBuildResult buildForBothLink(graph::TrainAtStationAbstraction const &first,
                                            graph::TrainAtStationAbstraction const &second, const Link &link);

        unsigned int optimized_edges_count{0};

    private:
        struct CachedHeadwayNodes {
            bool exists{false};
            HeadwayNodes in;
            HeadwayNodes out;
        };

        const CachedHeadwayNodes &getOrCreateHeadwayNodes(const graph::TrainAtStationAbstraction &tas);

        template<HeadwayNodes::Type TYPE>
        HeadwayNodes createHeadwayNodes(const graph::TrainAtStationAbstraction &abs,
                                        const graph::TrainAtStationAbstraction &next);


        template<HeadwayNodes::Type TYPE, typename fb::Activity A>
        void connectNodes(graph::node_index_t inner_node, graph::node_index_t outer_node, graph::choice_index_t choice);

        void createDepartHeadwayEdges(const MinHeadways &headways, graph::FullAlternativeIndex alternative,
                                      const CachedHeadwayNodes &first_hw_nodes,
                                      const CachedHeadwayNodes &second_hw_nodes);
        void createArrivalHeadwayEdges(graph::FullAlternativeIndex alternative, graph::node_index_t first_node,
                                       graph::node_index_t second_node);

        void createDirectionBothHeadwayEdges(graph::FullAlternativeIndex alternative, graph::node_index_t first_node,
                                             graph::node_index_t second_node);


        HeadwayBuildResult buildBothRealized(graph::TrainAtStationAbstraction const &first,
                                             graph::TrainAtStationAbstraction const &second);

        HeadwayBuildResult buildBothRealizedBothLink(graph::TrainAtStationAbstraction const &first,
                                                     graph::TrainAtStationAbstraction const &second);


        std::pair<bool, bool> checkIfFixed(const graph::TrainAtStationAbstraction &first,
                                           const graph::TrainAtStationAbstraction &second) const;
        graph::choice_index_t createChoiceIfNeeded(const graph::TrainAtStationAbstraction &first,
                                                   const graph::TrainAtStationAbstraction &second, const Link &link,
                                                   bool from_first, bool from_second);

        graph::AlternativeGraph &graph_;
        graph::Abstractions const &abstractions_;
        std::vector<distance_t> const &no_constrain_distances_;

        std::vector<CachedHeadwayNodes> cached_nodes_;


        GraphBuilderOptions const &options_;
    };


}// namespace fb
