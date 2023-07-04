
#pragma once

#include <utility>

#include "model/graph/Selection.h"
namespace fb {

    class CycleException : public std::exception {

    public:
        CycleException(std::string message, graph::Selection &&selection)
            : selection_(std::move(selection)), message_(std::move(message)) {}


        const char *what() { return message_.c_str(); }

        const std::string message_;
        const graph::Selection selection_;
    };

}// namespace fb
