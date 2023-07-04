#pragma once


#include "model/graph/Choice.h"
#include "model/graph/definitions.h"
namespace fb {


    struct HeadwayBuildResult {

        explicit HeadwayBuildResult() = default;
        explicit HeadwayBuildResult(graph::AlternativeIndex alternative_index)
            : alternative_index(alternative_index), valid(true) {}
        explicit HeadwayBuildResult(graph::choice_index_t choice_index) : choice_index(choice_index), valid{true} {}

        HeadwayBuildResult(HeadwayBuildResult const &) = default;
        HeadwayBuildResult(HeadwayBuildResult &&) = default;
        HeadwayBuildResult &operator=(HeadwayBuildResult const &) = default;

        bool valid{false};
        // if there was a choice created
        graph::choice_index_t choice_index{invalid<graph::choice_index_t>()};
        // if not, which abstraction is considered to be first
        graph::AlternativeIndex alternative_index{graph::AlternativeIndex::FIRST};
    };
}// namespace fb
