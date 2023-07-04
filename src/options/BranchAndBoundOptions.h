
#pragma once
#include <limits>

#include "Json.h"
#include "penalty/definitions.h"
#include <string>

namespace fb {


    struct BranchAndBoundOptions {
        enum class StoreOption { LIFO, BOL, PO };

        enum class InitialSelection { ORIGINAL, FCFS, SIMPLE_FCFS, NOOP };

        InitialSelection initial_selection{InitialSelection::NOOP};
        // in ms
        unsigned long timeout{std::numeric_limits<unsigned long>::max()};

        unsigned int skipped_stop_impact_factor{BIGGEST_SFF};
        unsigned int relation_impact_factor{1000};

        std::string state_config{"Default"};

        StoreOption store{StoreOption::LIFO};
        unsigned int bol_threshold{9};
        unsigned int store_max_load{3000};

        bool distance_to_end_normalization{true};
    };

    std::istream &operator>>(std::istream &in, BranchAndBoundOptions::StoreOption &store);
    std::ostream &operator<<(std::ostream &stm, BranchAndBoundOptions::StoreOption const &store);

    std::istream &operator>>(std::istream &in, BranchAndBoundOptions::InitialSelection &initial_selection);
    std::ostream &operator<<(std::ostream &stm, BranchAndBoundOptions::InitialSelection const &initial_selection);

    std::ostream &operator<<(std::ostream &stm, BranchAndBoundOptions const &opt);

    template<>
    std::string toJson(BranchAndBoundOptions const &t);

    template<>
    std::string toJson(BranchAndBoundOptions::StoreOption const &t);

    template<>
    std::string toJson(BranchAndBoundOptions::InitialSelection const &t);

}// namespace fb
