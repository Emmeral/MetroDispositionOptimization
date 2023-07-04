#pragma once

#include "model/graph/abstractions/TrainAtStationAbstraction.h"
namespace fb::graph {


    class SingleTrackAbstraction : public virtual TrainAtStationAbstraction {
    public:
        explicit SingleTrackAbstraction(track_id_t track_id) : track_id(track_id) {}

        bool considersTrack(track_id_t track) const override { return track == track_id; }
        unsigned int consideredTrackCount() const override { return 1; }
        const std::vector<choice_index_t> *getTrackChoices() const override { return nullptr; }

        track_id_t const &consideredTrack() const { return track_id; }
        std::span<const track_id_t> consideredTracks() const override { return std::span(std::addressof(track_id), 1); }

    protected:
        track_id_t track_id;
    };
}// namespace fb::graph
