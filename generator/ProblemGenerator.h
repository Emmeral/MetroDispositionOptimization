
#pragma once

#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"
#include "options/GeneratorOptions.h"
#include <random>
namespace fb {

    class ProblemGenerator {

    public:
        ProblemGenerator(const Model &model, const GeneratorOptions &options);

        ProblemInstance generateProblemInstance();

    private:
        void generateIncidents(ProblemInstance &problem);

        void generateExtendedRunTimeIncident(ProblemInstance &problem);
        void generateExtendedDwellTimeIncident(ProblemInstance &problem);
        void generateCourseDwellTimeIncident(ProblemInstance &problem);
        void generateLateDepartureIncident(ProblemInstance &problem);


        template<typename T>
        void setStartEnd(T &incident, ProblemInstance &problem);

        Model const &model_;
        GeneratorOptions const &options_;

        std::mt19937_64 re_;
    };

}// namespace fb
