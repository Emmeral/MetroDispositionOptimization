
#include "util.h"
#include "algo/CycleFinder.h"
#include "output/dot/graph/GraphDotWriter.h"
namespace fb {

    std::vector<std::string> getTopologicalNameHints(graph::AlternativeGraph const &graph,
                                                     LongestPathCalculator::State const &distance_state,
                                                     OnlineTopologicalSorter::State const &sorter_state) {

        std::vector<std::string> name_hints(graph.nodes.size());
        std::vector<fb::order_t> const &order = sorter_state.topological_order;
        auto const &dfs = distance_state.distance_from_start;
        for (int i = 0; i < order.size(); i++) {
            name_hints[i] = "Order:" + std::to_string(order[i]);
            if (dfs[i] != MIN_DISTANCE) {
                name_hints[i] += +" Dist:" + std::to_string(dfs[i]);
            }
        }
        return std::move(name_hints);
    }


    void logCycleException(const graph::GraphWithMeta &gwm, const CycleException &e) {
        CycleFinder finder{};
        auto const &graph = gwm.graph;
        auto cycle = finder.findCycle(graph, e.selection_);


        auto node_filter = [&](const graph::Node &n) {
            // cycle.nodes contains n
            return std::find(cycle.nodes.begin(), cycle.nodes.end(), &n) != cycle.nodes.end();
        };
        auto edge_filter = [&](const graph::Edge &e) {
            // cycle.edges contains e
            return std::find(cycle.edges.begin(), cycle.edges.end(), &e) != cycle.edges.end();
        };

        GraphDotWriter::writeToDot(gwm, std::vector<std::string>{}, "cycle.dot", false, &e.selection_, node_filter,
                                   edge_filter);

        std::cout << e.message_ << "\n";
        for (auto i = 0; i < cycle.nodes.size(); ++i) {
            auto node = cycle.nodes[i];
            auto edge = cycle.edges[i];
            auto *abs = gwm.abstractions.ofNode(node->index);
            std::cout << "Node: " << node->index;
            if (abs != nullptr) {
                std::cout << " (Abs: " << abs->getIndex() << ")";
            }
            std::cout << "\n";
            std::cout << "Edge: " << edge->index;

            if (!edge->is_fixed) {
                auto e_meta = graph.alternative_edges_metadata[edge->index];
                std::cout << "(" << graph.choice_metadata[e_meta.choice_index].type << ")";
            }
            std::cout << "\n";
        }
    }
    std::string nowAsString(const std::string &format) {
        time_t now;
        time(&now);
        char buf[30];
        strftime(buf, sizeof buf, format.data(), gmtime(&now));
        return std::string(buf);
    }
}// namespace fb
