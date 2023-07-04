#pragma once

#include "Json.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/Choice.h"
#include "penalty/DetailedPenalty.h"
#include "statistics/CharacteristicStatistic.h"
#include "statistics/ChoiceSplitStatistics.h"
#include "statistics/MaxChoiceStatistics.h"
#include <cstddef>
#include <iomanip>
#include <limits>
#include <ostream>

namespace fb {

    struct BABStatistics {
        explicit BABStatistics(const graph::AlternativeGraph &graph) : max_choices_statistics(graph.choice_metadata) {}

        bool aborted{false};
        unsigned int iterations{0};
        size_t state_store_max_size{0};

        unsigned int iteration_of_last_upper_bound_update{0};
        unsigned int upper_bound_updated{0};
        unsigned long complete_selections_considered{0};

        unsigned long closest_to_complete{std::numeric_limits<decltype(closest_to_complete)>::max()};

        unsigned int pruned_after_update{0};
        unsigned int pruned_before_update{0};
        unsigned int pruned_before_decision{0};

        unsigned int pruned_by_new_upper_bound{0};

        unsigned int both_descendant_valid_before_update{0};
        unsigned int both_descendant_valid_after_update{0};


        unsigned int dead_end_reached{0};
        ChoiceSplitStatistic<unsigned int> last_choice_before_dead_end{};
        ChoiceSplitStatistic<unsigned int> dead_end_jump_destination{};

        long dead_end_jumps{0};
        ChoiceSplitStatistic<unsigned int> choices_considered{};

        MaxChoicesStatistics<30> max_choices_statistics;

        CharacteristicStatistic lower_bound_values{};

        DetailedPenalty no_selection_penalty{};
        DetailedPenalty initial_selection_penalty{};
        DetailedPenalty best_selection_penalty{};

        unsigned long lazy_state_loaded{0};


        inline unsigned int totalPrunedByExistingUpperBound() const {
            return pruned_after_update + pruned_before_update + pruned_before_decision;
        }
        inline unsigned int totalPruned() const {
            return totalPrunedByExistingUpperBound() + pruned_by_new_upper_bound;
        }

        ChoiceSplitStatistic<double>
        averageConsideredChoicesByType(std::vector<graph::TaggedChoiceMetadata> const &choice_meta);

    private:
        friend std::ostream &operator<<(std::ostream &stm, BABStatistics const &stat);
    };

    std::ostream &operator<<(std::ostream &stm, BABStatistics const &stat);

    template<>
    std::string toJson(BABStatistics const &stat);
}// namespace fb
