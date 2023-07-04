
#pragma once


#include "model/input/InputModel.h"
#include "model/main/Model.h"

namespace fb {


    class ModelBuilder {
    public:
        static Model buildFromInput(input::InputModel const &input);

    private:
        static void buildNodeData(const input::InputModel &input, Model &output);
        static void buildLinkData(const input::InputModel &input, Model &output);
        static void buildCourseData(const input::InputModel &input, Model &output);
        static void buildDutyData(const input::InputModel &input, Model &output);
        static void buildTargetFrequencies(const input::InputModel &input, Model &output);
    };
}// namespace fb
