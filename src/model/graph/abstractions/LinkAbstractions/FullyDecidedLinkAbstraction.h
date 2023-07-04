
#pragma once
#include "model/graph/abstractions/abstract/CourseLinkAbstraction.h"
#include "model/graph/abstractions/abstract/FullyDecidedAbstraction.h"
#include "model/graph/abstractions/abstract/NonRealizedAbstraction.h"
#include "model/graph/abstractions/abstract/SingleTrackAbstraction.h"

namespace fb::graph {

    class FullyDecidedLinkAbstraction : public CourseLinkAbstraction,
                                        public FullyDecidedAbstraction,
                                        public NonRealizedAbstraction {


    public:
        FullyDecidedLinkAbstraction(const index_t index, const fb::Node &node, const Course &in_course,
                                    const Course &out_course, const seconds_t change_end_time)
            : FullyDecidedLinkAbstraction(index, node, in_course, out_course, TaggedWeight::fixed(change_end_time)) {}

        FullyDecidedLinkAbstraction(const index_t index, const fb::Node &node, const Course &in_course,
                                    const Course &out_course, const TaggedWeight &change_end_time)
            : FullyDecidedAbstraction(index, node, change_end_time, in_course.schedule.back()),
              CourseLinkAbstraction(in_course, out_course) {}

    private:
    };

}// namespace fb::graph
