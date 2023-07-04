#pragma once
#include "model/graph/definitions.h"
#include "model/main/Model.h"

namespace fb::graph {
    struct NodeInterface {

        template<Activity A>
        static inline NodeInterface createWithOnly(node_index_t node) {
            auto i = NodeInterface{};
            i.get<A>() = node;
            return i;
        }

        static NodeInterface createWithOnly(Activity a, node_index_t node) {
            if (a == Activity::STOP) {
                return createWithOnly<Activity::STOP>(node);
            } else {
                return createWithOnly<Activity::PASS>(node);
            }
        }

        explicit NodeInterface(node_index_t min = invalid<node_index_t>(), node_index_t stop = invalid<node_index_t>(),
                               node_index_t pass = invalid<node_index_t>())
            : min(min), stop(stop), pass(pass) {}


        node_index_t min;
        node_index_t stop;
        node_index_t pass;

        inline bool hasMinimum() const { return isValid(min); }
        inline bool hasStop() const { return isValid(stop); }
        inline bool hasPass() const { return isValid(pass); }

        template<Activity A>
        inline bool has() const {
            if constexpr (A == Activity::STOP) {
                return hasStop();
            } else if constexpr (A == Activity::PASS) {
                return hasPass();
            } else {
                return false;
            }
        }

        template<Activity A>
        inline node_index_t const &get() const {
            if constexpr (A == Activity::STOP) {
                return stop;
            } else if constexpr (A == Activity::PASS) {
                return pass;
            } else {
                return min;
            }
        }

        template<Activity A>
        inline node_index_t &get() {
            if constexpr (A == Activity::STOP) {
                return stop;
            } else if constexpr (A == Activity::PASS) {
                return pass;
            } else {
                return min;
            }
        }

        inline node_index_t const &get(Activity activity) const {
            if (activity == Activity::STOP) {
                return stop;
            } else if (activity == Activity::PASS) {
                return pass;
            } else {
                return min;
            }
        }

        template<typename FCT>
        void forEachValid(FCT fct) const {
            if (isValid(min)) {
                fct(min);
            }
            if (isValid(stop)) {
                fct(stop);
            }
            if (isValid(pass)) {
                fct(pass);
            }
        }


        bool operator==(NodeInterface const &other) const = default;
    };
}// namespace fb::graph
