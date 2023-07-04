#pragma once
#include "MaxChoiceStatistics.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/Choice.h"
#include <cstddef>
#include <iomanip>
#include <limits>
#include <ostream>

namespace fb {
    template<typename T>
    struct ChoiceSplitStatistic {
        T stop_pass{0};
        T track{0};
        T line_headway{0};
        T track_headway{0};

        T &of(graph::ChoiceType type) {
            switch (type) {
                case graph::ChoiceType::STOP_PASS:
                    return stop_pass;
                case graph::ChoiceType::TRACK:
                    return track;
                case graph::ChoiceType::LINE_HEADWAY:
                    return line_headway;
                case graph::ChoiceType::TRACK_HEADWAY:
                    return track_headway;
                default:
                    std::cout << "This should not happen";
                    return stop_pass;
            }
        }
        T const &of(graph::ChoiceType type) const {
            switch (type) {
                case graph::ChoiceType::STOP_PASS:
                    return stop_pass;
                case graph::ChoiceType::TRACK:
                    return track;
                case graph::ChoiceType::LINE_HEADWAY:
                    return line_headway;
                case graph::ChoiceType::TRACK_HEADWAY:
                    return track_headway;
                default:
                    std::cout << "This should not happen";
                    return stop_pass;
            }
        }


        void print(std::ostream &stm, std::string const &prefix, int width = 0) const {
            int real_width = std::max(0, width - (int) prefix.size() - 2);
            for (auto t: graph::ALL_CHOICE_TYPES) {
                stm << prefix << std::left << std::setw(real_width) << t << " : " << of(t) << "\n";
            }
        }
    };


    template<typename T>
    std::string toJson(ChoiceSplitStatistic<T> const &stat) {
        std::string str = "{";
        str += "\"stop_pass\": " + toJson(stat.stop_pass) + ", ";
        str += "\"track\": " + toJson(stat.track) + ", ";
        str += "\"line_headway\": " + toJson(stat.line_headway) + ", ";
        str += "\"track_headway\": " + toJson(stat.track_headway);
        str += "}";
        return str;
    }
}// namespace fb
