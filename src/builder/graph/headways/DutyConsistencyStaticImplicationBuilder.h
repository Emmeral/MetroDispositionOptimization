
#pragma once

#include "model/graph/AlternativeGraph.h"
#include "model/graph/abstractions/Abstractions.h"
#include "model/main/Model.h"
#include "options/GraphBuilderOptions.h"
namespace fb {

    class DutyConsistencyStaticImplicationBuilder {

    public:
        DutyConsistencyStaticImplicationBuilder(const Model &model, graph::AlternativeGraph &graph,
                                                const graph::Abstractions &abstractions,
                                                const GraphBuilderOptions &options = GraphBuilderOptions());


        /**
         * Adds static implications with the semantic:
         * "If I'm here before a course of another duty, I'm here before any later course of the other duty" and
         * "If the later course is here before me, the prior course is also here before me"
         * @param abs_id
         */
        void addDutyConsistencyStaticImplication(graph::abs_index_t abs_id);

    private:
        Model const &model_;
        graph::AlternativeGraph &graph_;
        graph::Abstractions const &abstractions_;

        GraphBuilderOptions const &options_;


        struct AltAbsPair {
            graph::FullAlternativeIndex alt;
            graph::abs_index_t other_abs;
            index_t index_in_duty;
            index_t si_index;

            auto operator<(const AltAbsPair &other) {
                return std::tie(index_in_duty, si_index, other_abs) <
                       std::tie(other.index_in_duty, other.si_index, other_abs);
            }
        };
        std::vector<std::vector<AltAbsPair>>
        getAlternativesByDuty(const std::vector<graph::FullAlternativeIndex> &outgoing);
        void createImplications(std::vector<std::vector<AltAbsPair>> &alternatives_by_duty);
    };

}// namespace fb
