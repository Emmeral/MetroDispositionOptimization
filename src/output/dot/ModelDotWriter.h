
#pragma once

#include "model/main/Model.h"
#include <string>
namespace fb {

    class ModelDotWriter {
    public:
        static std::string writeToDot(const Model &model);
        static void writeToDot(const Model &model, const std::string &filename);

        template<typename STREAM>
        static void writeToDot(STREAM &stream, const Model &model);
    };
}// namespace fb
