
#pragma once

#include "model/graph/abstractions/Abstractions.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"
#include "options/GraphBuilderOptions.h"
namespace fb {

    class AbstractionCreator {
    public:
        explicit AbstractionCreator(Model const &model, GraphBuilderOptions const &options = GraphBuilderOptions{})
            : model_(model), options_(options){};


        graph::TrainAtStationAbstraction *createAbstraction(const ProblemInstance &problem,
                                                            graph::Abstractions &abstractions,
                                                            course_index_t next_course_index, const Course &course,
                                                            index_t si_index,
                                                            const graph::TaggedWeight &stop_time) const;

    private:
        const Model &model_;
        const GraphBuilderOptions &options_;
    };

}// namespace fb
