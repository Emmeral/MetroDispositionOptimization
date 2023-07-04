
#pragma once

#include "DirectionMappedHeadways.h"
#include "HeadwaysCollection.h"
#include "builder/graph/headways/line/LineHeadwayBuilder.h"
#include "builder/graph/headways/track/TrackHeadwayBuilder.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/main/Model.h"
#include "options/GraphBuilderOptions.h"
namespace fb {


    class HeadwayBuilder {

    public:
        HeadwayBuilder(const Model &model, graph::GraphWithMeta &gwm,
                       const GraphBuilderOptions &options = GraphBuilderOptions());
        HeadwayBuilder(const Model &model, graph::AlternativeGraph &graph, graph::Abstractions &abstractions,
                       std::vector<distance_t> const &no_constrain_distances,
                       const GraphBuilderOptions &options = GraphBuilderOptions());


        void buildAllHeadways();

        void buildLineHeadways();
        void buildStationHeadways();
        void addStaticImplications();


    private:
        Model const &model_;
        graph::AlternativeGraph &graph_;
        graph::Abstractions &abstractions_;
        std::vector<distance_t> const &no_constrain_distances_;

        GraphBuilderOptions const &options_;

        HeadwayCollection headways_;

        void buildStationHeadwaysAtNode(TrackHeadwayBuilder &thb, const Node &node);
        void buildLineHeadwayAtLink(LineHeadwayBuilder &lhb, const Link &link,
                                    const std::vector<abs_pair> &abs_at_link);

        void buildLineHeadwayAtBothLink(LineHeadwayBuilder &lhb, const Link &link, const Link &rev_link,
                                        const std::vector<abs_pair> &abs_at_link,
                                        const std::vector<abs_pair> &abs_at_reverse_link);
    };

}// namespace fb
