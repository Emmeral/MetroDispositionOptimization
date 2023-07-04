
#include "GraphDotWriter.h"
#include "fmt/format.h"
#include "model/graph/Selection.h"
#include <fstream>
#include <sstream>

namespace fb {


    std::string nodeName(graph::node_index_t index) { return fmt::format("NODE_{}", index); }

    void fb::GraphDotWriter::writeToDot(const graph::GraphWithMeta &graph_meta,
                                        std::vector<std::string> const &node_name_hints, const std::string &filename,
                                        bool ignore_headways, const graph::Selection *sel,
                                        std::function<bool(graph::Node const &)> const &node_filter,
                                        std::function<bool(graph::Edge const &)> const &edge_filter) {


        std::ofstream file_stream;
        file_stream.open(filename, std::ios::out);

        writeToDot(file_stream, graph_meta, node_name_hints, ignore_headways, sel, node_filter, edge_filter);
        file_stream.close();
    }
    std::string GraphDotWriter::writeToDot(const graph::GraphWithMeta &graph_meta,
                                           std::vector<std::string> const &node_name_hints, bool ignore_headways,
                                           const graph::Selection *sel,
                                           std::function<bool(graph::Node const &)> const &node_filter,
                                           std::function<bool(graph::Edge const &)> const &edge_filter) {
        std::stringstream stream{};

        writeToDot(stream, graph_meta, node_name_hints, ignore_headways, sel, node_filter, edge_filter);
        return stream.str();
    }
    template<typename STREAM>
    void GraphDotWriter::writeToDot(STREAM &stream, const graph::GraphWithMeta &graph_meta,
                                    std::vector<std::string> const &node_name_hints, bool ignore_headways,
                                    const graph::Selection *sel,
                                    std::function<bool(graph::Node const &)> const &node_filter,
                                    std::function<bool(graph::Edge const &)> const &edge_filter) {


        const graph::Selection &selection = sel != nullptr ? *sel : graph::Selection{graph_meta.graph};

        stream << "digraph G{\n";

        auto const &graph = graph_meta.graph;

        auto const &abstractions = graph_meta.abstractions;
        auto abs_index = 0;
        auto abs_start = invalid<node_index_t>();
        auto abs_end = invalid<node_index_t>();

        if (!abstractions.empty()) {
            abs_start = abstractions[abs_index].begin();
        }

        for (auto const &node: graph.nodes) {
            if (!node_filter(node)) {
                continue;
            }

            if (node.index >= abs_end) {
                stream << "}\n";
                abs_end = invalid<node_index_t>();
                // print subgraph end
                ++abs_index;
                if (abstractions.size() > abs_index) {
                    abs_start = abstractions[abs_index].begin();
                }
            }

            // print subgraph start;
            auto const *abs = graph_meta.abstractions.ofNode(node.index);
            if (node.index >= abs_start) {


                if (abs != nullptr) {
                    abs_index = abs->getIndex();
                    abs_start = invalid<node_index_t>();
                    graph::TrainAtStationAbstraction const &abstraction = abstractions[abs_index];
                    abs_end = abstraction.end();

                    const ScheduleItem *realized_in = abstraction.getRealizedInScheduleItem();
                    const ScheduleItem *realized_out = abstraction.getRealizedOutScheduleItem();
                    std::string realized = realized_in != nullptr && isValid(realized_in->arrival) ? "realized" : "";
                    if (!realized.empty()) {
                        auto prefix =
                                realized_out != nullptr && isValid(realized_out->departure) ? "fully-" : "partly-";
                        realized = prefix + realized;
                    }

                    stream << "subgraph cluster_abstraction_" << abs_index << " {\n";
                    stream << "label=\"Abs " << abs_index << " at node " << abstraction.getNode().index << "("
                           << realized << ")"
                           << "\"\n";
                }
            }


            stream << nodeName(node.index);
            if (node_name_hints.size() > node.index && !node_name_hints[node.index].empty()) {
                stream << "[label=\"(" << node.index << ") " << node_name_hints[node.index] << "\"]";
            } else if (abs != nullptr) {
                if (node.index == abs->getArrivalTimeInNode()) {
                    stream << "[label=\"(" << node.index << ") "
                           << " Station"
                           << "\"]";
                } else if (node.index == abs->getDepartureTimeInNode()) {
                    stream << "[label=\"(" << node.index << ") "
                           << " Link"
                           << "\"]";
                }
            }


            stream << "\n";
        }

        if (isValid(abs_end)) {
            stream << "}\n";
        }

        for (auto const &edge: graph.fixed_edges) {
            if (!edge_filter(edge)) {
                continue;
            }

            stream << fmt::format("{} -> {}", nodeName(edge.from), nodeName(edge.to));
            stream << fmt::format("[label=\"{}\"]", getEdgeWeightLabel(graph, edge));
            stream << "\n";
        }

        stream << "{\n";


        auto outputAlternative = [&](graph::FullAlternativeIndex fai) {
            auto const &edges = graph.choices[fai.choice_index].alternative(fai.alternative_index).edges;

            if (selection.isDenied(fai)) {
                // don't print unselected edges
                return;
            }
            std::string style = "";
            if (selection.isChosen(fai)) {
                style = "bold";
            } else {
                style = "dashed";
            }
            auto const &meta = graph.choice_metadata[fai.choice_index];
            if (ignore_headways &&
                (meta.type == graph::ChoiceType::LINE_HEADWAY || meta.type == graph::ChoiceType::TRACK_HEADWAY)) {
                return;
            }

            for (auto const edge_i: edges) {
                auto const &edge = graph.alternative_edges[edge_i];

                if (!edge_filter(edge)) {
                    continue;
                }

                stream << fmt::format("{} -> {}", nodeName(edge.from), nodeName(edge.to));

                std::string label;
                bool constraint = true;
                if (meta.type == graph::ChoiceType::STOP_PASS) {
                    label = (fai.alternative_index == graph::stopAlternative()) ? " STOP" : " PASS";
                } else if (meta.type == graph::ChoiceType::TRACK) {
                    label = "Track Decision ";
                } else if (meta.type == graph::ChoiceType::LINE_HEADWAY) {
                    label = "LH ";
                    constraint = false;
                } else if (meta.type == graph::ChoiceType::TRACK_HEADWAY) {
                    label = "TH ";
                    constraint = false;
                }


                label += getEdgeWeightLabel(graph, edge);
                label += " Choice " + std::to_string(fai.choice_index) + "(";
                label += (std::stringstream() << fai.alternative_index << ")").str();

                stream << fmt::format("[label=\"{}\", constraint={}, style={}]", label, constraint, style) << "\n";
            }
        };

        for (auto const &choice: graph.choices) {
            outputAlternative({choice.index, graph::AlternativeIndex::FIRST});
            outputAlternative({choice.index, graph::AlternativeIndex::SECOND});
        }

        stream << "}\n";

        stream << "}";
    }

    std::string GraphDotWriter::getEdgeWeightLabel(const graph::AlternativeGraph &graph, const graph::Edge &edge) {
        if (!edge.is_time_dependent && edge.weight.weight != 0) {
            return std::to_string(edge.weight.weight);
        } else if (edge.is_time_dependent) {
            auto time_dep = graph.time_dependent_weights[edge.weight.time_dependent_index];
            return fmt::format("Base = {}, Extended = {}, Start = {}, End = {}", time_dep.base_weight,
                               time_dep.extended_weight, time_dep.start, time_dep.end);
        }
        return "";
    }

    const std::function<bool(graph::Node const &)> GraphDotWriter::NO_NODE_FILTER = [](graph::Node const &) {
        return true;
    };
    const std::function<bool(graph::Edge const &)> GraphDotWriter::NO_EDGE_FILTER = [](graph::Edge const &) {
        return true;
    };

}// namespace fb
