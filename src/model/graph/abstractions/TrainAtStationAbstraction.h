#pragma once


#include "NodeInterface.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/definitions.h"
#include "model/main/Model.h"
#include <bit>
#include <span>
namespace fb::graph {


    class TrainAtStationAbstraction {

    public:
        virtual void materialize(graph::AlternativeGraph &graph) = 0;

        virtual bool considersTrack(track_id_t track) const = 0;
        virtual unsigned int consideredTrackCount() const = 0;
        virtual std::span<const track_id_t> consideredTracks() const = 0;

        virtual std::optional<choice_index_t> getStopPassChoice() const = 0;
        virtual const std::vector<choice_index_t> *getTrackChoices() const = 0;

        /**
         *
         * @return true if the abstractions allows for choosing whether the train shall stop or pass
         */
        virtual bool hasStopPassChoice() const { return getStopPassChoice().has_value(); };
        /**
         * Returns the activity the train does at this station if it is already predefined. Otherwise the returns an empty
         * optional
         * @return the activity
         */
        virtual std::optional<Activity> predefinedActivity() const = 0;

        virtual bool hasTrackChoice() const { return getTrackChoices() != nullptr && !getTrackChoices()->empty(); };

        virtual bool isLink() const { return getInCourse().index != getOutCourse().index; }

        virtual node_index_t getArrivalTimeOutNode() const = 0;
        virtual node_index_t getDepartureTimeOutNode() const = 0;

        virtual node_index_t getArrivalTimeInNode() const = 0;
        virtual node_index_t getDepartureTimeInNode() const = 0;


        virtual const ScheduleItem *getOriginalInScheduleItem() const = 0;
        virtual const ScheduleItem *getOriginalOutScheduleItem() const = 0;

        virtual const ScheduleItem *getRealizedInScheduleItem() const = 0;
        virtual const ScheduleItem *getRealizedOutScheduleItem() const = 0;

        virtual const ScheduleItem *scheduleItemOfCourse(const Course &course) const {
            if (getInCourse().index == course.index) {
                return getOriginalInScheduleItem();
            } else if (getOutCourse().index == course.index) {
                return getOriginalOutScheduleItem();
            } else {
                return nullptr;
            }
        }

        virtual const Course &getInCourse() const = 0;
        virtual const Course &getOutCourse() const = 0;

        virtual fb::Node const &getNode() const = 0;

        virtual seconds_t getOriginalDeparture() const = 0;
        virtual seconds_t getOriginalArrival() const = 0;

        virtual seconds_t getRealizedDeparture() const {
            const ScheduleItem *out = getRealizedOutScheduleItem();
            return out == nullptr ? invalid<seconds_t>() : out->departure;
        }
        virtual seconds_t getRealizedArrival() const {
            const ScheduleItem *in = getRealizedInScheduleItem();
            return in == nullptr ? invalid<seconds_t>() : in->arrival;
        }

        virtual node_index_t getOffset() const = 0;
        virtual node_index_t getNodeCount() const = 0;

        virtual abs_index_t getIndex() const = 0;

        node_index_t begin() const { return getOffset(); }
        node_index_t end() const { return getOffset() + getNodeCount(); }
    };


}// namespace fb::graph
