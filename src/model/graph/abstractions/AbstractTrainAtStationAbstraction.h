#pragma once


#include "TrainAtStationAbstraction.h"
#include "model/graph/Node.h"
#include "model/graph/definitions.h"
namespace fb::graph {

    class AbstractTrainAtStationAbstraction : public virtual TrainAtStationAbstraction {

    public:
        AbstractTrainAtStationAbstraction(const index_t index, const fb::Node &node, const TaggedWeight stop_time)
            : index(index), node_(node), stop_time_{stop_time} {}


        abs_index_t getIndex() const final { return index; }
        node_index_t getOffset() const final { return offset_; }
        node_index_t getNodeCount() const final { return node_count_; }
        const fb::Node &getNode() const final { return node_; }

        const index_t index;

    protected:
        const fb::Node &node_;
        node_index_t offset_;
        node_index_t node_count_;

        TaggedWeight stop_time_;
    };


}// namespace fb::graph
