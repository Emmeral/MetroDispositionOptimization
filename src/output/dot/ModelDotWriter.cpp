
#include "ModelDotWriter.h"
#include "fmt/format.h"
#include <fstream>
#include <sstream>
namespace fb {


    std::string ModelDotWriter::writeToDot(const Model &model) {
        std::stringstream stream{};

        writeToDot(stream, model);
        return stream.str();
    }
    void ModelDotWriter::writeToDot(const Model &model, const std::string &filename) {

        std::ofstream file_stream;
        file_stream.open(filename, std::ios::out);
        writeToDot(file_stream, model);
        file_stream.flush();
        file_stream.close();
    }


    template<typename STREAM>
    void ModelDotWriter::writeToDot(STREAM &stream, const Model &model) {

        stream << "digraph G{\n";


        for (auto &node: model.nodes) {
            auto meta = model.node_metadata[node.index];

            std::stringstream tracks{};
            tracks << "[";
            for (auto t: node.tracks.all_tracks) {
                tracks << t << ",";
            };

            tracks << "]";

            double posy = (node.lat - 51.5) * 1000;
            double posx = node.lon * 1000;

            auto label = fmt::format("{}({}), Tracks={}", meta.code, node.index, tracks.str());
            auto pos = fmt::format("{},{}!", posx, posy);
            stream << "Node_" << node.index << "[label=\"" << label << "\" pos=\"" << pos << "\"]"
                   << "\n";
        }

        for (auto &link: model.links) {

            auto dir_as_str = (std::stringstream() << link.direction).str();
            auto val = fmt::format("Node_{} -> Node_{} [label=\"Dir={}, MRT={}\"]\n", link.link_start_node,
                                   link.link_end_node, dir_as_str, link.minimumRuntimes.minimum());
            stream << val;
        }


        stream << "}\n";
    }
}// namespace fb
