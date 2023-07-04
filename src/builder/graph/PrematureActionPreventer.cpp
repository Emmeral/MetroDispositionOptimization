
#include "PrematureActionPreventer.h"

namespace fb {
    PrematureActionPreventer::PrematureActionPreventer(graph::GraphWithMeta &gwm, ProblemInstance const &problem,
                                                       seconds_t allowed_early_departure, bool limit_early_passes)
        : PrematureActionPreventer(gwm.graph, gwm.abstractions, gwm.start_node, problem, allowed_early_departure,
                                   limit_early_passes) {}


    PrematureActionPreventer::PrematureActionPreventer(graph::AlternativeGraph &graph,
                                                       const graph::Abstractions &abstractions,
                                                       graph::node_index_t start_node, ProblemInstance const &problem,
                                                       seconds_t allowed_early_departure, bool limit_early_passes)
        : graph_(graph), abstractions_(abstractions), start_node_(start_node), problem_(problem),
          allowed_early_departure_(allowed_early_departure), limit_early_passes_(limit_early_passes) {}


    void PrematureActionPreventer::preventEarlyActions(const graph::TrainAtStationAbstraction &tas) {

        auto arr = tas.getRealizedArrival();

        bool isPartlyRealized = isValid(arr);


        seconds_t min_leave_time;
        auto in_node = tas.getDepartureTimeInNode();
        if (isPartlyRealized) {

            // if the arrival is already realized we fix it using this edge
            graph_.createFixedEdge(start_node_, tas.getArrivalTimeInNode(), arr);

            auto dep = tas.getRealizedDeparture();

            if (isValid(dep)) {
                min_leave_time = dep;
            } else if (isInvalid(allowed_early_departure_)) {
                min_leave_time = problem_.snapshot_time;
            } else {
                min_leave_time =
                        std::max(problem_.snapshot_time, tas.getOriginalDeparture() - allowed_early_departure_);
            }

            graph_.createFixedEdge(start_node_, in_node, min_leave_time);

            return;
            // only add edges if the allowed early departure is set
        }


        auto const &realized_sched = problem_.realized_schedule[tas.getInCourse().index];
        index_t in = tas.getOriginalInScheduleItem()->index;
        if (in > 0 && realized_sched.size() >= in && isValid(realized_sched[in - 1].departure)) {
            // prior is realized but current is not -> we have not arrived before snapshot time
            graph_.createFixedEdge(start_node_, tas.getArrivalTimeInNode(), problem_.snapshot_time);
        }


        if (tas.getOriginalOutScheduleItem()->index == 0 && tas.getOutCourse().index_in_duty == 0) {
            // make sure that the first course of a duty does not depart before its planned in the original schedule
            graph_.createFixedEdge(start_node_, tas.getArrivalTimeInNode(), tas.getOutCourse().planned_start);
        }


        if (isValid(allowed_early_departure_)) {
            // if the departure is not realized, it did not happen before snaphot_time and shall not happen too early;
            min_leave_time = std::max(problem_.snapshot_time, tas.getOriginalDeparture() - allowed_early_departure_);
            if (limit_early_passes_ ||
                (!tas.hasStopPassChoice() && tas.predefinedActivity().value() == Activity::STOP)) {
                graph_.createFixedEdge(start_node_, in_node, min_leave_time);
            } else if (tas.hasStopPassChoice()) {
                auto choice = tas.getStopPassChoice().value();
                // early passes are allowed -> we only create edge to stop in
                graph_.createStopEdge(start_node_, in_node, choice, graph::TaggedWeight::fixed(min_leave_time));
            }
        }
    }
}// namespace fb
