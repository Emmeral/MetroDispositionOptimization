#pragma once


#include "StateStore.h"
#include <stack>
namespace fb {


    template<typename State>
    class LIFOStateStore final : public StateStore<State> {


    public:
        bool empty() const override { return stack_.empty(); }
        size_t size() const override { return stack_.size(); }
        State &peekState() override { return stack_.top(); }

        State popState() override {

            auto val = std::move(stack_.top());
            stack_.pop();

            pruneLoop();
            return std::move(val);
        }
        bool registerState(State &&state) override {
            if (upper_bound_ != nullptr && state.lower_bound >= upper_bound_->lower_bound) {
                return false;
            }
            stack_.emplace(state);
            if (statistics != nullptr) {
                statistics->lower_bound_values.newValue(stack_.top().lower_bound);
            }
            return true;
        }
        void registerNewUpperBound(const State &upper_bound) override {

            upper_bound_ = &upper_bound;
            pruneLoop();
        }

        void reRegisterHead() override {
            if (statistics != nullptr) {
                statistics->lower_bound_values.newValue(stack_.top().lower_bound);
            }
            // do nothing, as we are LIFO
        }

        void useStatistics(BABStatistics &stats) override { statistics = &stats; }

    private:
        void pruneLoop() {
            // remove content from the stack that can be pruned;
            while (!empty() && peekState().lower_bound >= upper_bound_->lower_bound) {
                if (statistics != nullptr) {
                    statistics->pruned_by_new_upper_bound++;
                }
                stack_.pop();
            }
        }

        State const *upper_bound_{nullptr};
        std::stack<State> stack_;

        unsigned int total_elements_pruned_{0};
        BABStatistics *statistics{nullptr};
    };


}// namespace fb
