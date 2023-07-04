
#include "AbstractionLinker.h"
#include <functional>

namespace fb {
    AbstractionLinker::AbstractionLinker(
            graph::AlternativeGraph &graph,
            std::map<link_index_t, std::vector<ExtendedRunTime *>> const &extended_runtimes)
        : graph_(graph), extended_runtimes_{extended_runtimes} {}


    void AbstractionLinker::link(graph::TrainAtStationAbstraction &first, graph::TrainAtStationAbstraction &second,
                                 const Link &link) {


        auto out = first.getDepartureTimeOutNode();
        auto in = second.getArrivalTimeInNode();
        MinimumRuntimes const &runtimes = link.minimumRuntimes;

        // link according to realized schedule if present
        if (isValid(first.getRealizedDeparture()) && isValid(second.getRealizedArrival())) {
            auto dif = second.getRealizedArrival() - first.getRealizedDeparture();
            graph_.createFixedEdge(out, in, dif);
            return;
        }

        std::function<graph::TaggedWeight(seconds_t)> getTime;

        getTime = [&](seconds_t time) { return graph::TaggedWeight::fixed(time); };

        std::map<seconds_t, index_t> dependent_indices{};
        if (extended_runtimes_.contains(link.id)) {
            auto const &erts = extended_runtimes_.at(link.id);
            // TODO: add support for multiple problems at the same link
            ExtendedRunTime const &ert = *erts[0];


            getTime = [&](seconds_t time) {
                // cache to not create unecessary indices
                if (!dependent_indices.contains(time)) {
                    if (time < ert.extended_runtime) {
                        auto index = graph_.createTimeDependentWeight(time, ert.extended_runtime, ert.start, ert.end);
                        dependent_indices[time] = index;
                    } else {
                        return graph::TaggedWeight::fixed(time);
                    }
                }
                return graph::TaggedWeight::dependent(dependent_indices[time]);
            };
        }


        if (first.hasStopPassChoice() && second.hasStopPassChoice()) {

            // create intermediate nodes
            auto first_stopped = graph_.createNode();
            auto first_passed = graph_.createNode();

            const graph::choice_index_t first_choice = first.getStopPassChoice().value();
            const graph::choice_index_t second_choice = second.getStopPassChoice().value();

            graph_.createStopEdge(out, first_stopped, first_choice);
            graph_.createPassEdge(out, first_passed, first_choice);


            graph_.createFixedEdge(out, in, getTime(runtimes.minimum()));
            graph_.createStopEdge(out, in, second_choice, getTime(runtimes.minWithEnd(Activity::STOP)));
            graph_.createPassEdge(out, in, second_choice, getTime(runtimes.minWithEnd(Activity::PASS)));


            graph_.createStopEdge(first_stopped, in, second_choice, getTime(runtimes.stop_stop));
            graph_.createPassEdge(first_stopped, in, second_choice, getTime(runtimes.stop_pass));
            graph_.createFixedEdge(first_stopped, in, getTime(runtimes.minWithStart(Activity::STOP)));

            graph_.createStopEdge(first_passed, in, second_choice, getTime(runtimes.pass_stop));
            graph_.createPassEdge(first_passed, in, second_choice, getTime(runtimes.pass_pass));
            graph_.createFixedEdge(first_passed, in, getTime(runtimes.minWithStart(Activity::PASS)));


        } else if (first.hasStopPassChoice()) {

            auto second_act = second.predefinedActivity().value();
            const graph::choice_index_t first_choice = first.getStopPassChoice().value();

            auto rt_stop = runtimes.of(Activity::STOP, second_act);
            auto rt_pass = runtimes.of(Activity::PASS, second_act);
            auto rt_min = runtimes.minWithEnd(second_act);

            graph_.createStopEdge(out, in, first_choice, getTime(rt_stop));
            graph_.createPassEdge(out, in, first_choice, getTime(rt_pass));
            graph_.createFixedEdge(out, in, getTime(rt_min));


        } else if (second.hasStopPassChoice()) {

            auto first_act = first.predefinedActivity().value();
            const graph::choice_index_t second_choice = second.getStopPassChoice().value();

            auto rt_stop = runtimes.of(first_act, Activity::STOP);
            auto rt_pass = runtimes.of(first_act, Activity::PASS);
            auto rt_min = runtimes.minWithStart(first_act);

            graph_.createStopEdge(out, in, second_choice, getTime(rt_stop));
            graph_.createPassEdge(out, in, second_choice, getTime(rt_pass));
            graph_.createFixedEdge(out, in, getTime(rt_min));

        } else {
            auto rt = runtimes.of(first.predefinedActivity().value(), second.predefinedActivity().value());
            graph_.createFixedEdge(out, in, getTime(rt));
        }
    }
}// namespace fb
