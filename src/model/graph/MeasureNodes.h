
#pragma once

#include "definitions.h"
#include "model/main/Model.h"
namespace fb {
    namespace graph {

        struct MeasureNodes {

            MeasureNodes() = default;


            void addMeasureNode(node_index_t node_index, course_index_t course) {
                nodes.push_back(node_index);

                if (node_to_index.size() <= node_index) {
                    node_to_index.resize(node_index + 1);
                }
                if (course_to_index.size() <= course) {
                    course_to_index.resize(course + 1);
                }
                node_to_index[node_index] = nodes.size() - 1;
                course_to_index[course] = nodes.size() - 1;
            }

            std::vector<node_index_t> const &getNodes() const { return nodes; }

            size_t measureIndexOfNode(node_index_t node_index) const {
                if (node_index >= node_to_index.size()) {
                    return invalid<size_t>();
                }
                return node_to_index[node_index];
            }
            size_t measureIndexOfCourse(course_index_t course_index) const {
                if (course_index >= course_to_index.size()) {
                    return invalid<size_t>();
                }
                return course_to_index[course_index];
            }


        private:
            std::vector<node_index_t> nodes{};

            std::vector<size_t> node_to_index{};
            std::vector<size_t> course_to_index{};
        };

    }// namespace graph
}// namespace fb
