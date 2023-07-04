
#include "GraphStatistics.h"

namespace fb {
    GraphStatistics::GraphStatistics(graph::GraphWithMeta gwm, unsigned long build_time_ms) {
        this->build_time_ms = build_time_ms;
        vertex_count = gwm.graph.nodes.size();
        fixed_edge_count = gwm.graph.fixed_edges.size();
        alternative_edge_count = gwm.graph.alternative_edges.size();

        measure_vertex_count = gwm.measure_nodes.getNodes().size();


        choice_count.stop_pass = gwm.graph.stop_pass_choice_count;
        choice_count.line_headway = gwm.graph.line_headway_choice_count;
        choice_count.track_headway = gwm.graph.track_headway_choice_count;
        choice_count.track = gwm.graph.track_decision_choice_count;

        abstraction_count = gwm.abstractions.size();
        activity_decided_single_count = gwm.abstractions.activity_decided_single.size();
        fully_decided_single_count = gwm.abstractions.fully_decided_single.size();
        multi_track_single_count = gwm.abstractions.multi_track_single.size();
        track_decided_single_count = gwm.abstractions.track_decided_single.size();
        multi_track_link_count = gwm.abstractions.multi_track_link.size();
        fully_decided_link_count = gwm.abstractions.fully_decided_link.size();
        realized_link_count = gwm.abstractions.realized_link.size();
        realized_single_count = gwm.abstractions.realized_single.size();


        for (auto const &meta: gwm.graph.alternative_edges_metadata) {

            auto type = gwm.graph.choice_metadata[meta.choice_index].type;

            switch (type) {
                case fb::graph::ChoiceType::STOP_PASS:
                    edge_count_by_choice_type.stop_pass++;
                    break;
                case fb::graph::ChoiceType::TRACK:
                    edge_count_by_choice_type.track++;
                    break;
                case fb::graph::ChoiceType::TRACK_HEADWAY:
                    edge_count_by_choice_type.track_headway++;
                    break;
                case fb::graph::ChoiceType::LINE_HEADWAY:
                    edge_count_by_choice_type.line_headway++;
                    break;
            }
        }
    }
    std::ostream &operator<<(std::ostream &os, const GraphStatistics &stats) {


        os << "Graph statistics:\n";
        os << "  Build time: " << stats.build_time_ms << "ms\n";
        os << "  Vertex count: " << stats.vertex_count << "\n";
        os << "  Fixed edge count: " << stats.fixed_edge_count << "\n";
        os << "  Alternative edge count: " << stats.alternative_edge_count << "\n";
        os << "  Measure vertex count: " << stats.measure_vertex_count << "\n";
        os << "  Choice count:\n";
        stats.choice_count.print(os, "    ");
        os << "  Edge count by choice type:\n";
        stats.edge_count_by_choice_type.print(os, "    ");
        os << "  Abstraction count: " << stats.abstraction_count << "\n";
        os << "    Fully decided single count: " << stats.fully_decided_single_count << "\n";
        os << "    Multi track single count: " << stats.multi_track_single_count << "\n";
        os << "    Track decided single count: " << stats.track_decided_single_count << "\n";
        os << "    Activity decided single count: " << stats.activity_decided_single_count << "\n";
        os << "    Multi track link count: " << stats.multi_track_link_count << "\n";
        os << "    Fully decided link count: " << stats.fully_decided_link_count << "\n";
        os << "    Realized link count: " << stats.realized_link_count << "\n";
        os << "    Realized single count: " << stats.realized_single_count << "\n";
        return os;
    }
    template<>
    std::string toJson(const GraphStatistics &stats) {
        std::string str = "{";
        str += "\"build_time_ms\":" + toJson(stats.build_time_ms) + ",";
        str += "\"vertex_count\":" + toJson(stats.vertex_count) + ",";
        str += "\"fixed_edge_count\":" + toJson(stats.fixed_edge_count) + ",";
        str += "\"alternative_edge_count\":" + toJson(stats.alternative_edge_count) + ",";
        str += "\"measure_vertex_count\":" + toJson(stats.measure_vertex_count) + ",";
        str += "\"choice_count\":" + toJson(stats.choice_count) + ",";
        str += "\"edge_count_by_choice_type\":" + toJson(stats.edge_count_by_choice_type) + ",";
        str += "\"abstraction_count\":" + toJson(stats.abstraction_count) + ",";
        str += "\"fully_decided_single_count\":" + toJson(stats.fully_decided_single_count) + ",";
        str += "\"multi_track_single_count\":" + toJson(stats.multi_track_single_count) + ",";
        str += "\"track_decided_single_count\":" + toJson(stats.track_decided_single_count) + ",";
        str += "\"activity_decided_single_count\":" + toJson(stats.activity_decided_single_count) + ",";
        str += "\"multi_track_link_count\":" + toJson(stats.multi_track_link_count) + ",";
        str += "\"fully_decided_link_count\":" + toJson(stats.fully_decided_link_count) + ",";
        str += "\"realized_link_count\":" + toJson(stats.realized_link_count) + ",";
        str += "\"realized_single_count\":" + toJson(stats.realized_single_count);
        str += "}";
        return str;
    }
}// namespace fb
