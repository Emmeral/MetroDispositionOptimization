#pragma once

#include "model/graph/abstractions/TrainAtStationAbstraction.h"
namespace fb::graph {


    class MultiTrackAbstraction : public virtual TrainAtStationAbstraction {

    public:
        const std::vector<choice_index_t> *getTrackChoices() const override { return &track_choices_; }
        unsigned int consideredTrackCount() const override {
            return getNode().tracks.countInDirection(getInCourse().direction);
        }

        bool considersTrack(track_id_t track) const override {
            auto const &tracks = consideredTracksAsVec();
            return std::find(tracks.begin(), tracks.end(), track) != tracks.end();
        }

        std::vector<track_id_t> const &consideredTracksAsVec() const {
            Direction direction = getInCourse().direction;
            if (direction != getOutCourse().direction) {
                direction = Direction::BOTH;
            }

            return getNode().tracks.tracksInDirection(direction);
        }

        std::span<const track_id_t, std::dynamic_extent> consideredTracks() const override {
            auto const &vec = consideredTracksAsVec();
            return std::span(vec.begin(), vec.end());
        }

    protected:
        std::vector<choice_index_t> track_choices_;


        void createTrackChoices(AlternativeGraph &graph, track_id_t original_track) {


            auto const &node = getNode();
            auto original_index_in_direction = 0;

            std::vector<track_id_t> const &tracks_in_direction = consideredTracksAsVec();
            assert(tracks_in_direction.size() > 1);
            for (auto i = 0; i < tracks_in_direction.size(); ++i) {
                if (tracks_in_direction[i] == original_track) {
                    original_index_in_direction = i;
                    break;
                }
            }

            auto track_amount = tracks_in_direction.size();
            auto track_guard_amount = std::bit_width(track_amount - 1);
            track_choices_ = {};
            track_choices_.reserve(track_guard_amount);
            for (unsigned int i = 0; i < track_guard_amount; ++i) {

                auto original = static_cast<AlternativeIndex>((original_index_in_direction & (0b1 << i)) > 0);

                auto choice = graph.createChoice<ChoiceType::TRACK>(original, {getNode().index, i, getIndex()});
                track_choices_.push_back(choice);
                graph.choices[choice].relations.reserve(track_guard_amount);
            }

            // relate track decisions in the same group
            for (auto i = 0; i < track_choices_.size(); ++i) {
                for (auto j = i + 1; j < track_choices_.size(); ++j) {
                    graph.choices[track_choices_[i]].relations.push_back(track_choices_[j]);
                    graph.choices[track_choices_[j]].relations.push_back(track_choices_[i]);
                }
            }
        }
    };
}// namespace fb::graph
