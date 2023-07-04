
#pragma once

#include "StateStore.h"
#include <set>
namespace fb {


    template<typename State>
    class PenaltyOrderedStateStore final : public StateStore<State> {

    public:
        explicit PenaltyOrderedStateStore(State const *upper_bound = nullptr) : upper_bound_(upper_bound) {}

        bool empty() const override { return map_.empty(); };
        size_t size() const override { return map_.size(); }
        State &peekState() override { return map_.begin()->second; }

        State popState() override {
            auto it = map_.begin();
            auto val = std::move((*it).second);
            map_.erase(it);
            return std::move(val);
        };

        void reRegisterHead() override {
            auto node = map_.extract(map_.begin());
            node.key() = node.mapped().lower_bound;


            map_.insert(std::move(node));
        }

        bool registerState(State &&state) override {
            auto lb = state.lower_bound;
            if (upper_bound_ == nullptr || lb < upper_bound_->lower_bound) {
                map_.emplace(state.lower_bound, state);
                if (statistics != nullptr) {
                    statistics->lower_bound_values.newValue(lb);
                }
                return true;
            }
            return false;
        };

        void registerNewUpperBound(State const &upper_bound) override {
            upper_bound_ = &upper_bound;
            // delete elements that are too big now
            auto start_it = map_.lower_bound(upper_bound.lower_bound);
            auto deleted_amount = std::distance(start_it, map_.end());
            if (statistics != nullptr) {
                statistics->pruned_by_new_upper_bound += deleted_amount;
            }
            map_.erase(start_it, map_.end());
        };

        void useStatistics(BABStatistics &stats) override { statistics = &stats; }

    private:
        State const *upper_bound_;
        std::multimap<penalty_t, State> map_{};

        BABStatistics *statistics{nullptr};
    };

}// namespace fb
