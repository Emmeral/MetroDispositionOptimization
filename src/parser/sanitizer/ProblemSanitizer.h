
#pragma once

#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"
namespace fb {

    class ProblemSanitizer {

    public:
        void sanitizeRealizedSchedule(ProblemInstance &problem);


    private:
        Model const &model_;

    public:
        explicit ProblemSanitizer(const Model &model);
    };

}// namespace fb
