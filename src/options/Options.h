
#pragma once
#include "BranchAndBoundOptions.h"
#include "GraphBuilderOptions.h"
#include "Json.h"
#include <string>


namespace fb {

    struct Options {

        std::string model;
        std::string problem;
        std::string solution;

        std::string name;

        bool sanitize{true};
        bool validate{true};

        bool batch{false};

        enum class Mode { Iterated, Single, FCFS };

        Mode mode{Mode::Iterated};

        GraphBuilderOptions graph{};
        BranchAndBoundOptions bab{};
    };


    std::ostream &operator<<(std::ostream &stm, Options const &opt);


    std::istream &operator>>(std::istream &in, Options::Mode &store);
    std::ostream &operator<<(std::ostream &stm, Options::Mode const &store);

    template<>
    std::string toJson(Options const &t);

    template<>
    std::string toJson(Options::Mode const &t);


}// namespace fb
