#include "BranchAndBoundOptions.h"
#include "GraphBuilderOptions.h"
#include "Options.h"
#include <boost/program_options.hpp>
#include <fstream>

namespace fb {
    namespace po = boost::program_options;

    Options handleOptions(int ac, char *av[]) {


        Options options{};
        std::string config_file;

        po::options_description generic("Generic Options");
        generic.add_options()("config,c", po::value(&config_file)->default_value("config.cfg"))(
                "model,m", po::value<>(&options.model),
                "Directory of model")("problem,p", po::value<>(&options.problem), "Directory of problem")(
                "sanitize", po::value<>(&options.sanitize), "Whether to sanitize the input model")(
                "validate", po::value<>(&options.validate), "Whether to validate the solution")(
                "solution", po::value(&options.solution), "Directory for solution")("mode", po::value(&options.mode))(
                "name,n", po::value(&options.name), "Name of the run")("batch", po::value(&options.batch));

        GraphBuilderOptions &gbo = options.graph;
        po::options_description graph_builder("Graph Builder Options");
        graph_builder.add_options()("graph.max_premature_departure", po::value(&gbo.max_premature_departure))(
                "graph.static_implications_same_direction",
                po::value(&gbo.build_static_implications_in_same_direction))(
                "graph.static_implications_alternating_directions",
                po::value(&gbo.build_static_implications_in_alternating_directions))(
                "graph.static_implications_duty_consistency",
                po::value(&gbo.build_static_implications_duty_consistency))(
                "graph.headway_choice_max_difference", po::value(&gbo.headway_choice_max_difference))(
                "graph.stop_pass_choice_after_last_incident", po::value(&gbo.stop_pass_choice_after_last_incident))(
                "graph.track_choice_after_last_incident", po::value(&gbo.track_choice_after_last_incident))(
                "graph.line_arrival_headway", po::value(&gbo.line_arrival_headway))(
                "graph.track_relation_max_difference", po::value(&gbo.track_relation_max_difference))(
                "graph.track_relation_only_for_link", po::value(&gbo.track_relation_only_for_link))(
                "graph.extend", po::value(&gbo.extend))("graph.stop_pass_choices", po::value(&gbo.stop_pass_choices))(
                "graph.track_choices", po::value(&gbo.track_choices))("graph.build_trivial_implications",
                                                                      po::value(&gbo.build_trivial_implications));


        BranchAndBoundOptions &babo = options.bab;

        po::options_description bab_opts("Branch and Bound Options");
        bab_opts.add_options()("bab.timeout", po::value(&babo.timeout))("bab.skipped_stop_impact_factor",
                                                                        po::value(&babo.skipped_stop_impact_factor))(
                "bab.state_config", po::value(&babo.state_config))("bab.relation_impact_factor",
                                                                   po::value(&babo.relation_impact_factor))(
                "bab.store", po::value(&babo.store))("bab.bol_threshold", po::value(&babo.bol_threshold))(
                "bab.store_max_load", po::value(&babo.store_max_load))("bab.initial_selection",
                                                                       po::value(&babo.initial_selection))(
                "bab.distance_to_end_normalization", po::value(&babo.distance_to_end_normalization));


        po::options_description all;
        all.add(generic).add(graph_builder).add(bab_opts);
        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, all), vm);
        po::notify(vm);


        po::store(po::parse_config_file(config_file.c_str(), all), vm);
        po::notify(vm);

        return options;
    }
}// namespace fb
