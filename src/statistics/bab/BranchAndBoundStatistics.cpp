#include "BranchAndBoundStatistics.h"

namespace fb {
    std::ostream &operator<<(std::ostream &stm, const BABStatistics &stat) {

        stm << "BranchAndBoundStatistics: \n";
        stm << "- aborted             : " << stat.aborted << "\n";
        stm << "- Iterations          : " << stat.iterations << "\n";
        stm << "- State Store Max Size: " << stat.state_store_max_size << "\n";
        stm << "- Upper Bound Updates : " << stat.upper_bound_updated << "\n";
        stm << "- Iter. of Last Upd.  : " << stat.iteration_of_last_upper_bound_update << "\n";
        stm << "- Average Lower Bound : " << stat.lower_bound_values.mean()
            << " (Std. dev : " << stat.lower_bound_values.standardDeviation() << ")\n";
        stm << "- Compl. Sel. Considrd: " << stat.complete_selections_considered << "\n";
        stm << "- Closest to complete : " << stat.closest_to_complete << "\n";
        stm << "- Total Pruned        : " << stat.totalPruned() << "\n";
        stm << "  - By new bound      : " << stat.pruned_by_new_upper_bound << "\n";
        stm << "  - By existing bound : " << stat.totalPrunedByExistingUpperBound() << "\n";
        stm << "    - before decision : " << stat.pruned_before_decision << "\n";
        stm << "    - before update   : " << stat.pruned_before_update << "\n";
        stm << "    - after updates   : " << stat.pruned_after_update << "\n";
        stm << "- Dead End Reached    : " << stat.dead_end_reached << "\n";
        stm << "- Dead End Avg. Jump  : " << ((double) stat.dead_end_jumps) / stat.dead_end_reached << "\n";
        stm << "- Dead End Last Choice: \n";
        stat.last_choice_before_dead_end.print(stm, "  - ", 23);
        stm << "- Dead End Jump Dest  : \n";
        stat.dead_end_jump_destination.print(stm, "  - ", 23);
        stm << "- Both Desc. Bef. Upd.: " << stat.both_descendant_valid_before_update << "\n";
        stm << "- Both Desc. Aft. Upd.: " << stat.both_descendant_valid_after_update << "\n";
        stm << "- Choices Considered  : \n";
        stat.choices_considered.print(stm, "  - ", 23);
        stm << "- Choice Considered   : Count (id, type) \n";

        auto i = 1;
        for (auto m: stat.max_choices_statistics.top_choices) {
            stm << "  - " << std::setw(17) << i << " : " << m.count << " (" << m.choice_index << ", " << m.type
                << ")\n";
            ++i;
        }
        stm << "- No Selection Penalty: \n" << stat.no_selection_penalty << "\n";
        stm << "- Initial Selection Penalty: \n" << stat.initial_selection_penalty << "\n";
        stm << "- Best Selection Penalty: \n" << stat.best_selection_penalty << "\n";
        stm << "- Lazy State Loaded   : " << stat.lazy_state_loaded << "\n";

        return stm;
    }
    template<>
    std::string fb::toJson(const BABStatistics &stat) {
        std::string str = "{";
        str += "\"aborted\":" + toJson(stat.aborted) + ",";
        str += "\"iterations\":" + toJson(stat.iterations) + ",";
        str += "\"state_store_max_size\":" + toJson(stat.state_store_max_size) + ",";
        str += "\"upper_bound_updated\":" + toJson(stat.upper_bound_updated) + ",";
        str += "\"iteration_of_last_upper_bound_update\":" + toJson(stat.iteration_of_last_upper_bound_update) + ",";
        str += "\"lower_bound_values\":" + toJson(stat.lower_bound_values) + ",";
        str += "\"complete_selections_considered\":" + toJson(stat.complete_selections_considered) + ",";
        str += "\"closest_to_complete\":" + toJson(stat.closest_to_complete) + ",";
        str += "\"pruned_by_new_upper_bound\":" + toJson(stat.pruned_by_new_upper_bound) + ",";
        str += "\"pruned_before_decision\":" + toJson(stat.pruned_before_decision) + ",";
        str += "\"pruned_before_update\":" + toJson(stat.pruned_before_update) + ",";
        str += "\"pruned_after_update\":" + toJson(stat.pruned_after_update) + ",";
        str += "\"dead_end_reached\":" + toJson(stat.dead_end_reached) + ",";
        str += "\"dead_end_jumps\":" + toJson(stat.dead_end_jumps) + ",";
        str += "\"dead_end_jump_destination\":" + toJson(stat.dead_end_jump_destination) + ",";
        str += "\"last_choice_before_dead_end\":" + toJson(stat.last_choice_before_dead_end) + ",";
        str += "\"both_descendant_valid_before_update\":" + toJson(stat.both_descendant_valid_before_update) + ",";
        str += "\"both_descendant_valid_after_update\":" + toJson(stat.both_descendant_valid_after_update) + ",";
        str += "\"choices_considered\":" + toJson(stat.choices_considered) + ",";
        str += "\"max_choices_statistics\":" + toJson(stat.max_choices_statistics) + ",";
        str += "\"no_selection_penalty\":" + toJson(stat.no_selection_penalty) + ",";
        str += "\"initial_selection_penalty\":" + toJson(stat.initial_selection_penalty) + ",";
        str += "\"best_selection_penalty\":" + toJson(stat.best_selection_penalty) + ",";
        str += "\"lazy_state_loaded\":" + toJson(stat.lazy_state_loaded);
        str += "}";
        return str;
    }
    ChoiceSplitStatistic<double>
    BABStatistics::averageConsideredChoicesByType(std::vector<graph::TaggedChoiceMetadata> const &choice_meta) {

        ChoiceSplitStatistic<unsigned int> considered_individual_choices{};

        std::vector<unsigned int> &ccc = max_choices_statistics.choice_considered_count;
        for (auto ci = 0; ci < ccc.size(); ++ci) {
            auto type = choice_meta[ci].type;
            if (ccc[ci] > 0) {
                considered_individual_choices.of(type)++;
            }
        }

        ChoiceSplitStatistic<double> choices_considered_avg{};
        for (auto t: graph::ALL_CHOICE_TYPES) {
            choices_considered_avg.of(t) = choices_considered.of(t) / (double) considered_individual_choices.of(t);
        }

        return choices_considered_avg;
    }

}// namespace fb
