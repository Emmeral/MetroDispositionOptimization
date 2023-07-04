
#pragma once

#include "model/graph/definitions.h"
#include <cassert>
#include <optional>
namespace fb {

    struct HeadwayNodes {

        enum class Type : bool { IN, OUT };


        struct HeadwayNode {
            graph::node_index_t node_index{invalid<decltype(node_index)>()};
            std::optional<Activity> start_activity = std::nullopt;
            std::optional<Activity> end_activity = std::nullopt;


            operator const graph::node_index_t &() const { return node_index; }
        };

        std::array<HeadwayNode, 9> nodes{};

        HeadwayNodes() {
            sx().start_activity = Activity::STOP;
            px().start_activity = Activity::PASS;

            ss().start_activity = Activity::STOP;
            ss().end_activity = Activity::STOP;

            sp().start_activity = Activity::STOP;
            sp().end_activity = Activity::PASS;

            ps().start_activity = Activity ::PASS;
            ps().end_activity = Activity::STOP;

            pp().start_activity = Activity::PASS;
            pp().end_activity = Activity::PASS;

            xp().end_activity = Activity::PASS;
            xs().end_activity = Activity::STOP;
        }

        inline HeadwayNode &xx() { return nodes[getIndex({}, {})]; }
        inline HeadwayNode &xp() { return nodes[getIndex({}, Activity::PASS)]; }
        inline HeadwayNode &xs() { return nodes[getIndex({}, Activity::STOP)]; }
        inline HeadwayNode &px() { return nodes[getIndex(Activity::PASS, {})]; }
        inline HeadwayNode &pp() { return nodes[getIndex(Activity::PASS, Activity::PASS)]; }
        inline HeadwayNode &ps() { return nodes[getIndex(Activity::PASS, Activity::STOP)]; }
        inline HeadwayNode &sx() { return nodes[getIndex(Activity::STOP, {})]; }
        inline HeadwayNode &sp() { return nodes[getIndex(Activity::STOP, Activity::PASS)]; }
        inline HeadwayNode &ss() { return nodes[getIndex(Activity::STOP, Activity::STOP)]; }


        HeadwayNode &get(std::optional<Activity> start_activity, std::optional<Activity> end_activity) {
            return nodes[getIndex(start_activity, end_activity)];
        }
        HeadwayNode const &get(std::optional<Activity> start_activity, std::optional<Activity> end_activity) const {
            return nodes[getIndex(start_activity, end_activity)];
        }

        static constexpr size_t getIndex(std::optional<Activity> start_activity, std::optional<Activity> end_activity) {
            auto first = start_activity ? static_cast<bool>(start_activity.value()) + 1 : 0;
            auto second = end_activity ? static_cast<bool>(end_activity.value()) + 1 : 0;
            return first * 3 + second;
        }
    };


}// namespace fb
