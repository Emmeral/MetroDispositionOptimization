
#include "ScheduleItemExtractor.h"
#include "model/graph/abstractions/abstract/MultiTrackAbstraction.h"
#include "model/graph/abstractions/abstract/SingleTrackAbstraction.h"

namespace fb {
    ScheduleItemExtractor::ScheduleItemExtractor(const graph::Selection &selection, const std::vector<distance_t> &dfs)
        : selection_(selection), dfs_(dfs) {}


    ScheduleItem ScheduleItemExtractor::extractItemWithAmendment(const graph::TrainAtStationAbstraction &tas,
                                                                 const Course &course, TimetableAmendment &amendment) {
        ScheduleItem amended_item{};
        ScheduleItem original_item{};
        ScheduleItem const *realized_item;
        if (tas.getInCourse().index == course.index) {
            original_item = *tas.getOriginalInScheduleItem();
            realized_item = tas.getRealizedInScheduleItem();
        } else {
            assert(tas.getOutCourse().index == course.index);
            original_item = *tas.getOriginalOutScheduleItem();
            realized_item = tas.getRealizedOutScheduleItem();
        }

        amended_item = original_item;
        bool was_already_realized = realized_item != nullptr;

        auto new_arrival = dfs_[tas.getArrivalTimeOutNode()];
        if (original_item.arrival != new_arrival && isValid(amended_item.arrival) && !was_already_realized) {
            amendment.arrival_retimings.emplace_back(course.index, original_item, new_arrival);
        }
        amended_item.arrival = new_arrival;

        auto new_departure = dfs_[tas.getDepartureTimeOutNode()];
        bool realized_departure = (was_already_realized && isValid(realized_item->departure));
        if (original_item.departure != new_departure && isValid(amended_item.departure) && !realized_departure) {
            amendment.departure_retimings.emplace_back(course.index, original_item, new_departure);
        }
        amended_item.departure = new_departure;


        auto new_track = getUsedTrack(tas, selection_);

        if (original_item.original_track != new_track && !was_already_realized) {
            amendment.replatformings.emplace_back(course.index, original_item, new_track);
        }
        amended_item.original_track = new_track;

        auto new_activity = getActivity(tas, selection_);

        if (original_item.original_activity != new_activity && !was_already_realized) {
            assert(new_activity == Activity::PASS && original_item.original_activity == Activity::STOP);
            amendment.skipped_stops.emplace_back(course.index, original_item);
        }
        amended_item.original_activity = new_activity;

        return amended_item;
    }


    track_id_t ScheduleItemExtractor::getUsedTrack(const graph::TrainAtStationAbstraction &tas,
                                                   const graph::Selection &selection) {


        if (!tas.hasTrackChoice()) {
            auto const &single_track = dynamic_cast<graph::SingleTrackAbstraction const &>(tas);
            return single_track.consideredTrack();
        }
        auto const &multi_track = dynamic_cast<graph::MultiTrackAbstraction const &>(tas);
        auto const &tracks = multi_track.consideredTracksAsVec();

        auto const &choices = *multi_track.getTrackChoices();

        auto internal_track_id = 0u;
        for (auto i = 0; i < choices.size(); ++i) {
            auto alt = selection.chosenAlternative(choices[i]);
            auto potential_update = internal_track_id | (static_cast<bool>(alt) << i);
            if (potential_update > tracks.size() - 1) {
                break;// the choice is not relevant as there is not track id this large -> is not needed
            }
            internal_track_id = potential_update;
        }

        return tracks[internal_track_id];
    }
    Activity ScheduleItemExtractor::getActivity(const graph::TrainAtStationAbstraction &tas,
                                                const graph::Selection &selection) {

        if (!tas.hasStopPassChoice()) {
            auto const &realized = tas.getRealizedInScheduleItem();
            if (realized != nullptr) {
                return realized->original_activity;
            }

            return tas.getOriginalInScheduleItem()->original_activity;
        }

        return selection.chosenAlternative(tas.getStopPassChoice().value()) == graph::stopAlternative()
                       ? Activity::STOP
                       : Activity::PASS;
    }


    ScheduleItem ScheduleItemExtractor::extractItem(const graph::TrainAtStationAbstraction &tas, const Course &course) {

        ScheduleItem amended_item{};
        amended_item.arrival = dfs_[tas.getArrivalTimeOutNode()];
        amended_item.departure = dfs_[tas.getDepartureTimeOutNode()];

        amended_item.original_track = getUsedTrack(tas, selection_);
        amended_item.original_activity = getActivity(tas, selection_);


        if (tas.isLink() && course.index == tas.getInCourse().index) {
            amended_item.index = tas.getOriginalInScheduleItem()->index;
        } else {
            amended_item.index = tas.getOriginalOutScheduleItem()->index;
        }

        amended_item.node = tas.getNode().index;


        return amended_item;
    }
}// namespace fb
