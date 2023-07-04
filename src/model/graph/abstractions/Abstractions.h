
#pragma once
#include "model/graph/abstractions/LinkAbstractions/FullyDecidedLinkAbstraction.h"
#include "model/graph/abstractions/LinkAbstractions/MultiTrackLinkAbstraction.h"
#include "model/graph/abstractions/LinkAbstractions/RealizedLinkAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/MultiTrackSingleAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/TrackDecidedSingleAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/predefinedactivity/FullyDecidedSingleAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/predefinedactivity/PredefinedActivityMultiTrackAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/predefinedactivity/RealizedSingleAbstraction.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/main/Model.h"
#include <deque>

namespace fb {
    struct GraphStatistics;// here for friend accessing
}

namespace fb::graph {


    struct DirectionSortedAbstractions {
        std::vector<abs_index_t> eb;
        std::vector<abs_index_t> wb;

        std::vector<abs_index_t> &of(Direction dir) {
            if (dir == Direction::WB) {
                return wb;
            } else {
                return eb;
            }
        }

        std::vector<abs_index_t> const &of(Direction dir) const {
            if (dir == Direction::WB) {
                return wb;
            } else {
                return eb;
            }
        }
    };

    struct OutgoingHeadways {
        std::vector<FullAlternativeIndex> link_headways{};
        std::vector<FullAlternativeIndex> track_headways{};
    };


    class Abstractions {

    public:
        Abstractions(size_t node_count, size_t course_count, size_t link_count)
            : first_abs_of_course(course_count, invalid<abs_index_t>()),
              course_ends(course_count, invalid<abs_index_t>()), node_sorted(node_count), link_sorted(link_count) {}


        template<typename T, typename... Args>
        T &emplaceAbstraction(Args &&...args) {
            std::deque<T> &vector = storageForType<T>();
            abs_index_t id = all.size();
            TrainAtStationAbstraction &created = vector.emplace_back(id, std::forward<Args>(args)...);
            all.emplace_back(&vector.back());

            next_indices.push_back(invalid<abs_index_t>());
            prior_indices.push_back(invalid<abs_index_t>());


            node_sorted[created.getNode().index].of(created.getOutCourse().direction).push_back(created.getIndex());
            outgoing_headways.emplace_back();
            updateFirstAndLastOfCourse(id, created);


            return vector.back();
        }


        template<typename T>
        std::deque<T> &storageForType();


        TrainAtStationAbstraction const *ofIndex(abs_index_t const &index) const;
        TrainAtStationAbstraction *ofIndex(abs_index_t const &index);

        TrainAtStationAbstraction &operator[](abs_index_t const &index);
        TrainAtStationAbstraction const &operator[](abs_index_t const &index) const;
        size_t size() const;
        bool empty() const;
        TrainAtStationAbstraction const &back() const { return *all.back(); }
        TrainAtStationAbstraction &back() { return *all.back(); }

        auto begin() { return all.begin(); }
        auto end() { return all.end(); }
        auto begin() const { return all.begin(); }
        auto end() const { return all.end(); }


        TrainAtStationAbstraction const *next(TrainAtStationAbstraction const &current) const;
        TrainAtStationAbstraction const *next(abs_index_t const &index) const;
        TrainAtStationAbstraction const *prior(TrainAtStationAbstraction const &current) const;
        TrainAtStationAbstraction const *prior(abs_index_t const &index) const;

        TrainAtStationAbstraction *next(TrainAtStationAbstraction const &current);
        TrainAtStationAbstraction *next(abs_index_t const &index);
        TrainAtStationAbstraction *prior(TrainAtStationAbstraction const &current);
        TrainAtStationAbstraction *prior(abs_index_t const &index);

        TrainAtStationAbstraction const *nextAbsOfCourse(TrainAtStationAbstraction const &current,
                                                         course_index_t course) const;

        TrainAtStationAbstraction const *ofNode(graph::node_index_t node) const;
        TrainAtStationAbstraction const *firstAbsOfCourse(course_index_t course) const;
        TrainAtStationAbstraction const *courseEnd(course_index_t course) const;

        void registerLinkHeadway(TrainAtStationAbstraction const &first, FullAlternativeIndex const &choice);
        void registerTrackHeadway(TrainAtStationAbstraction const &first, FullAlternativeIndex const &choice);

        void registerSuccessor(TrainAtStationAbstraction const &first, TrainAtStationAbstraction const &second,
                               const Link &link);

        OutgoingHeadways const &getOutgoingHeadways(abs_index_t const &index) const { return outgoing_headways[index]; }
        OutgoingHeadways const &getOutgoingHeadways(TrainAtStationAbstraction const &tas) const {
            return getOutgoingHeadways(tas.getIndex());
        }

        DirectionSortedAbstractions const &atNode(node_index_t node) const;
        std::vector<std::pair<abs_index_t, abs_index_t>> const &atLink(link_index_t link) const;

        friend class fb::GraphStatistics;

    private:
        void updateFirstAndLastOfCourse(abs_index_t id, const TrainAtStationAbstraction &created);

        std::vector<TrainAtStationAbstraction *> all{};

        std::deque<MultiTrackSingleAbstraction> multi_track_single{};
        std::deque<TrackDecidedSingleAbstraction> track_decided_single{};
        std::deque<PredefinedActivityMultiTrackAbstraction> activity_decided_single{};
        std::deque<FullyDecidedSingleAbstraction> fully_decided_single{};
        std::deque<RealizedSingleAbstraction> realized_single{};

        std::deque<MultiTrackLinkAbstraction> multi_track_link{};
        std::deque<FullyDecidedLinkAbstraction> fully_decided_link{};
        std::deque<RealizedLinkAbstraction> realized_link{};

        std::vector<abs_index_t> next_indices{};
        std::vector<abs_index_t> prior_indices{};

        std::vector<abs_index_t> first_abs_of_course{};
        std::vector<abs_index_t> course_ends{};

        std::vector<DirectionSortedAbstractions> node_sorted{};
        std::vector<std::vector<std::pair<abs_index_t, abs_index_t>>> link_sorted{};

        std::vector<OutgoingHeadways> outgoing_headways{};
    };

}// namespace fb::graph
