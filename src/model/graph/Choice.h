
#pragma once

#include "Edge.h"
#include "Json.h"
#include "definitions.h"
#include "model/definitions.h"
#include "model/main/Model.h"
#include <vector>

namespace fb::graph {


    enum class AlternativeIndex : bool { FIRST, SECOND };

    std::ostream &operator<<(std::ostream &os, AlternativeIndex alt);

    static AlternativeIndex stopAlternative() { return AlternativeIndex::FIRST; }
    static AlternativeIndex passAlternative() { return AlternativeIndex::SECOND; }
    static AlternativeIndex activityToAlternative(Activity act) {
        if (act == Activity::STOP) {
            return stopAlternative();
        } else {
            return passAlternative();
        }
    }

    static AlternativeIndex otherAlternative(AlternativeIndex alt) {
        if (alt == AlternativeIndex::FIRST) {
            return AlternativeIndex::SECOND;
        }
        return AlternativeIndex::FIRST;
    }


    struct FullAlternativeIndex {
        choice_index_t choice_index;
        AlternativeIndex alternative_index;

        static FullAlternativeIndex stopAlternative(choice_index_t choice_index) {
            return {choice_index, graph::stopAlternative()};
        }
        static FullAlternativeIndex passAlternative(choice_index_t choice_index) {
            return {choice_index, graph::passAlternative()};
        }

        auto operator<=>(const FullAlternativeIndex &) const = default;
    };

    static FullAlternativeIndex otherAlternative(FullAlternativeIndex alt) {
        return {alt.choice_index, otherAlternative(alt.alternative_index)};
    }


    struct Alternative {
        std::vector<a_edge_index_t> edges{};
        std::vector<FullAlternativeIndex> static_implications{};

        void addEdge(edge_index_t edge) { edges.push_back(edge); }
    };

    struct Choice {

        explicit Choice(index_t index) : index(index) {}

        index_t index;
        AlternativeIndex original;
        Alternative a1{};
        Alternative a2{};

        std::vector<choice_index_t> relations{};

        Alternative const &alternative(AlternativeIndex const &a_index) const {
            if (a_index == AlternativeIndex::FIRST) {
                return a1;
            } else {
                return a2;
            }
        }
        Alternative &alternative(AlternativeIndex const &a_index) {
            if (a_index == AlternativeIndex::FIRST) {
                return a1;
            } else {
                return a2;
            }
        }

        template<AlternativeIndex a_index>
        Alternative &alternative() {
            if constexpr (a_index == AlternativeIndex::FIRST) {
                return a1;
            } else {
                return a2;
            }
        }
    };

    enum class ChoiceType { STOP_PASS, TRACK, LINE_HEADWAY, TRACK_HEADWAY };

    static constexpr std::array<ChoiceType, 4> ALL_CHOICE_TYPES = {ChoiceType::STOP_PASS, ChoiceType::TRACK,
                                                                   ChoiceType::LINE_HEADWAY, ChoiceType::TRACK_HEADWAY};
    std::ostream &operator<<(std::ostream &stm, const ChoiceType &type);


    struct LineHeadwayMetadata {
        fb::link_index_t link{invalid<link_index_t>()};
        abs_index_t first{invalid<abs_index_t>()};
        abs_index_t second{invalid<abs_index_t>()};
    };
    struct TrackHeadwayMetadata {
        fb::node_index_t node{invalid<fb::node_index_t>()};
        abs_index_t first{invalid<abs_index_t>()};
        abs_index_t second{invalid<abs_index_t>()};
    };

    struct StopPassMetadata {
        fb::node_index_t node{invalid<fb::node_index_t>()};
        fb::course_index_t course{invalid<fb::course_index_t>()};
        abs_index_t abstraction{invalid<abs_index_t>()};
    };

    struct TrackMetadata {
        fb::node_index_t node{invalid<fb::node_index_t>()};
        unsigned int track_order{invalid<unsigned int>()};
        ;
        abs_index_t abstraction{invalid<abs_index_t>()};
    };

    template<ChoiceType type>
    static auto inline getMetadataType() {
        if constexpr (type == ChoiceType::STOP_PASS) {
            return StopPassMetadata{};
        } else if constexpr (type == ChoiceType::TRACK_HEADWAY) {
            return TrackHeadwayMetadata{};
        } else if constexpr (type == ChoiceType::LINE_HEADWAY) {
            return LineHeadwayMetadata{};
        } else if constexpr (type == ChoiceType::TRACK) {
            return TrackMetadata{};
        }
    }

    union ChoiceMetadata {
        LineHeadwayMetadata lh;
        TrackHeadwayMetadata th;
        StopPassMetadata sp;
        TrackMetadata t;
    };

    struct TaggedChoiceMetadata {
        ChoiceType type;
        ChoiceMetadata data{};

        explicit TaggedChoiceMetadata(LineHeadwayMetadata const &lh) {
            type = ChoiceType::LINE_HEADWAY;
            data.lh = lh;
        }
        explicit TaggedChoiceMetadata(TrackHeadwayMetadata const &th) {
            type = ChoiceType::TRACK_HEADWAY;
            data.th = th;
        }
        explicit TaggedChoiceMetadata(TrackMetadata const &t) {
            type = ChoiceType::TRACK;
            data.t = t;
        }
        explicit TaggedChoiceMetadata(StopPassMetadata const &sp) {
            type = ChoiceType::STOP_PASS;
            data.sp = sp;
        }
    };


}// namespace fb::graph

namespace fb {

    template<>
    graph::FullAlternativeIndex constexpr invalid<graph::FullAlternativeIndex>() {
        return {invalid<graph::choice_index_t>(), graph::AlternativeIndex::FIRST};
    }

    template<>
    std::string toJson(graph::ChoiceType const &t);
}// namespace fb
