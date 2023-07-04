


#include "ProblemGenerator.h"
#include "ProblemWriter.h"
#include "builder/ModelBuilder.h"
#include "options/GeneratorOptions.h"
#include "options/handle_generator_options.h"
#include "parser/DataParser.h"
#include "parser/sanitizer/InputSanitizer.h"
#include "util.h"
#include <filesystem>
#include <iostream>
int main(int ac, char *av[]) {
    using namespace fb;

    GeneratorOptions options = handleGeneratorOptions(ac, av);

    if (isInvalid(options.seed)) {
        options.seed = time(nullptr);
    }

    std::cout << options;

    auto parser = DataParser();
    auto input_data = parser.parse(options.model);

    auto sanitizer = InputSanitizer(input_data);
    sanitizer.overrideWrongIndividualData();
    sanitizer.expandNodeTracks();
    sanitizer.correctMinimalRuntimes();
    sanitizer.correctMinimalHeadways();


    auto model = ModelBuilder::buildFromInput(input_data);
    ProblemGenerator generator(model, options);
    ProblemWriter writer{model};

    if (options.name.empty()) {
        options.name = nowAsString("%Y-%m-%d-%H-%M-%S");
    }

    std::cout << options.name << "\n";

    auto out_folder = options.output_folder + "/" + options.name;

    std::filesystem::create_directories(out_folder);
    auto option_store_file = std::ofstream(out_folder + "/Options.json");
    option_store_file << toJson(options);
    option_store_file.close();

    for (auto i = 0; i < options.count; ++i) {
        std::cout << "Generating problems: " << i + 1 << "/" << options.count << "\r" << std::flush;
        auto const problem = generator.generateProblemInstance();
        writer.writeProblem(problem, out_folder + "/" + std::to_string(i));
    }
    std::cout << "\n";


    return 0;
}
