
#pragma once

#include "AbstractionCreator.h"
#include "builder/graph/headways/line/LineHeadwayBuilder.h"
#include "builder/graph/headways/track/TrackHeadwayBuilder.h"
#include "model/PartialSchedule.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/abstractions/LinkAbstractions/MultiTrackLinkAbstraction.h"
#include "model/graph/abstractions/LinkAbstractions/RealizedLinkAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/MultiTrackSingleAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/predefinedactivity/FullyDecidedSingleAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/predefinedactivity/PredefinedActivityMultiTrackAbstraction.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"
#include "options/GraphBuilderOptions.h"


namespace fb {

    class GraphBuilder {
    public:
        explicit GraphBuilder(Model const &model, GraphBuilderOptions const &options = GraphBuilderOptions{});
        graph::GraphWithMeta build(ProblemInstance const &problem);
        graph::GraphWithMeta build(ProblemInstance const &problem, PartialSchedule const &pre_planned_schedule);

    private:
        const Model &model_;
        const GraphBuilderOptions &options_;

        const AbstractionCreator creator_;

        void buildForDuty(const RSDuty &duty, const ProblemInstance &problem, graph::GraphWithMeta &result);


        void createMeasureNode(const graph::TrainAtStationAbstraction &tas, graph::GraphWithMeta &result,
                               graph::AlternativeGraph &graph) const;
        graph::TaggedWeight calculateStopTime(const ProblemInstance &problem, graph::AlternativeGraph &graph,
                                              const Course &course, int si_index, seconds_t change_end_time);
        void createForCourse(const ProblemInstance &problem, const Course &course, graph::GraphWithMeta &result,
                             bool first_course, bool last_course);
        void createCourseLink(const ProblemInstance &problem, const Course &first_course, const Course &second_course,
                              seconds_t change_end_time, graph::GraphWithMeta &result);
        bool hasPotentialImpact(const ProblemInstance &problem, course_index_t course_index, index_t si_index) const;
    };

}// namespace fb
