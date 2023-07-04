


#include "algo/path/LongestPathCalculator.h"
#include "cstdint"
#include "exceptions/CycleException.h"
#include "model/enums/Enums.h"
#include "options/handle_options.h"
#include "output/dot/graph/GraphDotWriter.h"
#include "output/dot/graph/NameHintHelper.h"
#include "runner/MainRunner.h"
#include "util.h"
#include <iostream>
#include <ranges>


namespace fb {

    fb::graph::Selection &storeTopologicalDotGraph(fb::graph::GraphWithMeta &result,
                                                   const fb::graph::AlternativeGraph &graph, fb::graph::Selection &sel,
                                                   const fb::LongestPathCalculator::State &pc_state,
                                                   fb::OnlineTopologicalSorter::State const &sorter_state) {
        std::vector<std::string> name_hints(graph.nodes.size());

        for (auto const *tas: result.abstractions) {
            addNameHints(name_hints, *tas);
        }

        fb::GraphDotWriter::writeToDot(result, name_hints, "Graph.dot", true, &sel);

        name_hints = fb::getTopologicalNameHints(graph, pc_state, sorter_state);
        fb::GraphDotWriter::writeToDot(result, name_hints, "Graph_Topo.dot", true, &sel);
        return sel;
    }


}// namespace fb

int main(int ac, char *av[]) {
    using namespace fb;

    Options options = handleOptions(ac, av);
    std::cout << options;

    try {
        MainRunner::runAndSave(options);
    } catch (CycleException const &e) { return 1; }

    return 0;
}
