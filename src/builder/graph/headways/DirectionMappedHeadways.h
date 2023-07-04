#pragma once

#include "model/graph/Choice.h"
#include "model/graph/definitions.h"
#include <map>
#include <utility>

namespace fb {


    typedef std::pair<graph::abs_index_t, graph::abs_index_t> abs_pair;

    struct DirectionMappedHeadways {
        std::map<abs_pair, graph::choice_index_t> only_eb{};
        std::map<abs_pair, graph::choice_index_t> only_wb{};
        std::map<abs_pair, graph::choice_index_t> mixed{};

        std::map<abs_pair, graph::choice_index_t> &ofDir(Direction dir) {
            if (dir == Direction::EB) {
                return only_eb;
            }
            if (dir == Direction::WB) {
                return only_wb;
            }
            return mixed;
        }

        std::map<abs_pair, graph::choice_index_t> const &ofDir(Direction dir) const {
            if (dir == Direction::EB) {
                return only_eb;
            }
            if (dir == Direction::WB) {
                return only_wb;
            }
            return mixed;
        }
    };

    struct DirectionMappedFixedHeadways {
        std::map<abs_pair, graph::AlternativeIndex> only_eb{};
        std::map<abs_pair, graph::AlternativeIndex> only_wb{};
        std::map<abs_pair, graph::AlternativeIndex> mixed{};

        std::map<abs_pair, graph::AlternativeIndex> &ofDir(Direction dir) {
            if (dir == Direction::EB) {
                return only_eb;
            }
            if (dir == Direction::WB) {
                return only_wb;
            }
            return mixed;
        }

        std::map<abs_pair, graph::AlternativeIndex> const &ofDir(Direction dir) const {
            if (dir == Direction::EB) {
                return only_eb;
            }
            if (dir == Direction::WB) {
                return only_wb;
            }
            return mixed;
        }
    };

}// namespace fb
