#pragma once
#include "LazyState.h"
#include "StateStore.h"
#include <memory>

namespace fb {
    template<typename State, typename Initializer>
    class BestOfLastInsertedStateStore final : public StateStore<State> {
    public:
        explicit BestOfLastInsertedStateStore(unsigned int number_considered_for_lasts, unsigned int store_max_load,
                                              Initializer const &initializer)
            : number_considered_for_lasts_(number_considered_for_lasts), store_max_load_(store_max_load),
              initializer_(initializer) {

            assert(store_max_load_ > number_considered_for_lasts_);
        }

        bool empty() const override { return store_.empty(); }
        size_t size() const override { return store_.size(); }
        State &peekState() override {
            assert(!store_.empty());
            assert(best_index_of_lasts_ < store_.size());

            auto &lazy_state = store_[best_index_of_lasts_];
            if (!lazy_state.isLoaded()) {
                statistics->lazy_state_loaded++;
            }
            return lazy_state.getCompleteState(initializer_);
        }
        State popState() override {
            assert(!store_.empty());
            LazyState lazy_state = std::move(store_[best_index_of_lasts_]);

            if (!lazy_state.isLoaded()) {
                statistics->lazy_state_loaded++;
            }

            store_.erase(store_.begin() + best_index_of_lasts_);
            setMinOfLasts();

            return std::move(lazy_state.getCompleteState(initializer_));
        }
        bool registerState(State &&state) override {
            if (upper_bound_ != nullptr && state.lower_bound >= upper_bound_->lower_bound) {
                return false;
            }
            store_.emplace_back(std::move(state));
            if (store_.back().getCommonState().lower_bound <= best_index_of_lasts_) {
                best_index_of_lasts_ = store_.size() - 1;
            }

            if (store_.size() - best_index_of_lasts_ > number_considered_for_lasts_) {
                setMinOfLasts();
            }
            if (statistics != nullptr) {
                statistics->lower_bound_values.newValue(store_.back().getCommonState().lower_bound);
            }
            freeStore();

            return true;
        }
        void reRegisterHead() override {

            store_.emplace_back(std::move(store_[best_index_of_lasts_]));
            store_.erase(store_.begin() + best_index_of_lasts_);

            if (statistics != nullptr) {
                statistics->lower_bound_values.newValue(store_.back().getCommonState().lower_bound);
            }

            setMinOfLasts();
        }
        void registerNewUpperBound(const State &upper_bound) override {
            upper_bound_ = &upper_bound;

            // eagerly remove elements
            auto erased = std::erase_if(store_, [&](const auto &item) {
                return item.getCommonState().lower_bound >= upper_bound_->lower_bound;
            });

            if (statistics != nullptr) {
                statistics->pruned_by_new_upper_bound += erased;
            }

            if (erased > 0) {
                setMinOfLasts();
            }
        }
        void useStatistics(BABStatistics &stats) override { statistics = &stats; }


    private:
        void setMinOfLasts() {
            auto min = std::numeric_limits<penalty_t>::max();
            best_index_of_lasts_ = invalid<decltype(best_index_of_lasts_)>();
            int start = store_.size() < number_considered_for_lasts_ ? 0 : store_.size() - number_considered_for_lasts_;
            for (auto i = start; i < store_.size(); ++i) {
                if (store_[i].getCommonState().lower_bound <= min) {
                    best_index_of_lasts_ = i;
                    min = store_[i].getCommonState().lower_bound;
                }
            }
        }

        void freeStore() {
            if (store_.size() <= store_max_load_ + 1) {
                return;
            }

            auto prune_start = store_.size() - 1 - store_max_load_;

            for (auto i = prune_start; i >= 0; --i) {
                if (!store_[i].isLoaded()) {
                    break;
                }
                store_[i].free();
            }
        }

        State const *upper_bound_{nullptr};

        unsigned int store_max_load_ = 3000;
        unsigned int number_considered_for_lasts_;
        unsigned int best_index_of_lasts_{0};

        Initializer const &initializer_;

        std::vector<LazyState<State>> store_{};


        BABStatistics *statistics{nullptr};
    };
}// namespace fb
