#pragma once
#include "Json.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/Choice.h"
#include <cstddef>
#include <iomanip>
#include <limits>
#include <ostream>

namespace fb {
    struct ChoicePair {
        fb::graph::choice_index_t choice_index;
        unsigned int count;
        graph::ChoiceType type;
    };

    template<unsigned int N>
    struct MaxChoicesStatistics {

        explicit MaxChoicesStatistics(std::vector<graph::TaggedChoiceMetadata> const &choice_meta)
            : choice_considered_count(choice_meta.size(), 0) {
            for (auto i = 0; i < top_choices.size(); ++i) {
                top_choices[i].choice_index = i;
                top_choices[i].count = 0;
                top_choices[i].type = choice_meta[i].type;
            }
        }


        void markConsidered(fb::graph::choice_index_t choice, graph::ChoiceType type) {
            ++choice_considered_count[choice];

            bool inTop = false;
            auto revIt = top_choices.rbegin();
            while (revIt != top_choices.rend() && revIt->count < choice_considered_count[choice]) {

                if (revIt->choice_index == choice) {
                    revIt->count = choice_considered_count[choice];
                    inTop = true;
                } else {
                    revIt++;
                }
            }

            if (inTop) {
                // swap up
                while ((revIt + 1) != top_choices.rend() && revIt->choice_index == choice) {
                    if ((revIt + 1)->count < revIt->count) {
                        std::swap(*revIt, *(revIt + 1));
                    }
                    revIt++;
                }
            } else if (revIt != top_choices.rbegin()) {
                // insert into top
                std::rotate(top_choices.rbegin(), top_choices.rbegin() + 1, revIt);
                if (revIt != top_choices.rbegin()) {
                    *(revIt - 1) = {choice, choice_considered_count[choice], type};
                }
            }
        }

        std::array<ChoicePair, N> top_choices;
        std::vector<unsigned int> choice_considered_count{};
    };

    template<unsigned int N>
    std::string toJson(MaxChoicesStatistics<N> const &t) {
        return toJson(t.top_choices);
    }

    template<>
    std::string toJson(ChoicePair const &t);


}// namespace fb
