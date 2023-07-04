
#include "handle_generator_options.h"
#include <boost/program_options.hpp>

namespace fb {
    namespace po = boost::program_options;

    GeneratorOptions handleGeneratorOptions(int ac, char *av[]) {

        GeneratorOptions options{};

        std::string config_file;

        po::options_description all;

        all.add_options()("config,c", po::value(&config_file)->default_value("config.cfg"))(
                "model,m", po::value<>(&options.model))("output_folder,o", po::value<>(&options.output_folder))(
                "count", po::value<>(&options.count))("name,n", po::value(&options.name))(
                "seed,s", po::value<>(&options.seed))("min_snapshot_time", po::value(&options.min_snapshot_time))(
                "max_snapshot_time", po::value(&options.max_snapshot_time))("max_incident_in_future",
                                                                            po::value(&options.max_incident_in_future))(
                "max_incidents", po::value(&options.max_incidents))("min_incidents", po::value(&options.min_incidents))(
                "max_extended_dwell_time", po::value(&options.max_extended_dwell_time))(
                "min_extended_dwell_time", po::value(&options.min_extended_dwell_time))(
                "max_extended_run_time", po::value(&options.max_extended_run_time))(
                "min_extended_run_time", po::value(&options.min_extended_run_time))(
                "max_course_dwell_time", po::value(&options.max_course_dwell_time))(
                "min_course_dwell_time", po::value(&options.min_course_dwell_time))(
                "max_late_departures", po::value(&options.max_late_departures))(
                "min_late_departures", po::value(&options.min_late_departures))(
                "interval_min_time", po::value(&options.interval_min_time))("interval_max_time",
                                                                            po::value(&options.interval_max_time))(
                "late_departure_min_delay", po::value(&options.late_departure_min_delay))(
                "late_departure_max_delay", po::value(&options.late_departure_max_delay))(
                "course_dwell_min_extend", po::value(&options.course_dwell_min_extend))(
                "course_dwell_max_extend", po::value(&options.course_dwell_max_extend))(
                "extended_dwell_min_time", po::value(&options.extended_dwell_min_time))(
                "extended_dwell_max_time", po::value(&options.extended_dwell_max_time))(
                "extended_runtime_min_factor", po::value(&options.extended_runtime_min_factor))(
                "extended_runtime_max_factor", po::value(&options.extended_runtime_max_factor));


        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, all), vm);
        po::notify(vm);

        po::store(po::parse_config_file(config_file.c_str(), all), vm);
        po::notify(vm);

        return options;
    }
}// namespace fb
