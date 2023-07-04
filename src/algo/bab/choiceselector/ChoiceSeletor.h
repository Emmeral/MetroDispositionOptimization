#pragma once


#include "model/graph/definitions.h"
namespace fb {


    template<typename State>
    class ChoiceSelector {

    public:
        virtual graph::choice_index_t getBranchChoice(State const &state) = 0;
    };


}// namespace fb
