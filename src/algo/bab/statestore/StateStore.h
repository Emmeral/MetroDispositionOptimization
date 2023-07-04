#pragma once


#include "algo/bab/states/State.h"
#include "statistics/bab/BranchAndBoundStatistics.h"


namespace fb {
    template<typename State>
        requires std::derived_from<State, CommonState>
    class StateStore {
    public:
        virtual bool empty() const = 0;
        virtual size_t size() const = 0;

        /**
         * Get a modifiable reference to the head. If it is changed in any way `reRegisterHead` must be called
         * or the state must be popped.
         * @return  a reference to the next state
         */
        virtual State &peekState() = 0;
        virtual State popState() = 0;

        virtual bool registerState(State &&state) = 0;
        /**
         * Notifies the state store, that the head was changed in-place and shall be reinserted into the store.
         */
        virtual void reRegisterHead() = 0;

        /**
         *
         * @param upper_bound the new upper bound
         * @return the number of elements pruned by the new upper bound
         */
        virtual void registerNewUpperBound(State const &upper_bound) = 0;

        virtual void useStatistics(BABStatistics &stats) = 0;
    };
}// namespace fb
