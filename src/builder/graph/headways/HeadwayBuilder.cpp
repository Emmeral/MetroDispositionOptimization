
#include "HeadwayBuilder.h"
#include "DutyConsistencyStaticImplicationBuilder.h"
#include "StaticImplicationBuilder.h"
#include "builder/graph/headways/line/LineHeadwayBuilder.h"
#include "builder/graph/headways/track/TrackHeadwayBuilder.h"

namespace fb {
    HeadwayBuilder::HeadwayBuilder(const Model &model, graph::GraphWithMeta &gwm,
                                   const fb::GraphBuilderOptions &options)
        : model_(model), graph_(gwm.graph), abstractions_(gwm.abstractions),
          no_constrain_distances_(gwm.no_constrain_distances), headways_(model.nodes.size(), model.links.size()),
          options_{options} {}

    HeadwayBuilder::HeadwayBuilder(const Model &model, graph::AlternativeGraph &graph,
                                   graph::Abstractions &abstractions,
                                   std::vector<distance_t> const &no_constrain_distances,
                                   GraphBuilderOptions const &options)
        : model_(model), graph_(graph), abstractions_(abstractions), no_constrain_distances_(no_constrain_distances),
          headways_(model.nodes.size(), model.links.size()), options_{options} {}


    void HeadwayBuilder::buildAllHeadways() {

        // store outgoing headways for later

        buildLineHeadways();
        buildStationHeadways();
        addStaticImplications();
    }
    void HeadwayBuilder::buildLineHeadways() {

        LineHeadwayBuilder lhb{graph_, abstractions_, no_constrain_distances_, options_};

        for (auto const &link: model_.links) {

            auto &link_sorted = abstractions_.atLink(link.id);
            buildLineHeadwayAtLink(lhb, link, link_sorted);

            if (link.direction == Direction::BOTH) {
                auto rev_id = model_.reverse_links[link.id];
                if (rev_id < link.id) {
                    continue;// do not build headways twice
                }
                auto &at_reverse_link = abstractions_.atLink(rev_id);
                buildLineHeadwayAtBothLink(lhb, link, model_.links[rev_id], link_sorted, at_reverse_link);
            }
        }
    }


    void HeadwayBuilder::buildLineHeadwayAtLink(LineHeadwayBuilder &lhb, const Link &link,
                                                const std::vector<abs_pair> &abs_at_link) {

        MinHeadways min_headways{link.headways.headways};
        for (auto i = 0; i < abs_at_link.size(); ++i) {
            auto const &[first, first_next] = abs_at_link[i];

            auto const &first_abs = abstractions_[first];
            for (auto j = i + 1; j < abs_at_link.size(); ++j) {

                auto const &[second, second_next] = abs_at_link[j];
                auto const &second_abs = abstractions_[second];

                if (first_abs.getOutCourse().duty == second_abs.getOutCourse().duty) {
                    // same duty, no line headway
                    continue;
                }
                HeadwayBuildResult hw_result;

                hw_result = lhb.build(first_abs, second_abs, link, min_headways);

                if (!hw_result.valid) {
                    continue;
                }
                if (isValid(hw_result.choice_index)) {
                    auto choice_index = hw_result.choice_index;


                    std::map<abs_pair, graph::choice_index_t> &map = headways_.line_headways[link.id];
                    map.try_emplace(map.end(), {first, second}, choice_index);

                    abstractions_.registerLinkHeadway(first_abs, {choice_index, graph::AlternativeIndex::FIRST});
                    abstractions_.registerLinkHeadway(second_abs, {choice_index, graph::AlternativeIndex::SECOND});
                } else {
                    std::map<abs_pair, graph::AlternativeIndex> &map = headways_.fixed_line_headways_[link.id];
                    map.try_emplace(map.end(), {first, second}, hw_result.alternative_index);
                }
            }
        }
    }

    void HeadwayBuilder::buildLineHeadwayAtBothLink(LineHeadwayBuilder &lhb, const Link &link, const Link &rev_link,
                                                    const std::vector<abs_pair> &abs_at_link,
                                                    const std::vector<abs_pair> &abs_at_reverse_link) {

        for (auto const &[first, first_next]: abs_at_link) {


            auto const &first_abs = abstractions_[first];
            for (auto const &[second, second_next]: abs_at_reverse_link) {

                auto const &second_abs = abstractions_[second];

                if (first_abs.getOutCourse().duty == second_abs.getOutCourse().duty) {
                    // same duty, no line headway
                    continue;
                }
                HeadwayBuildResult hw_result;
                assert(link.direction == Direction::BOTH && first_abs.getNode().index != second_abs.getNode().index);
                hw_result = lhb.buildForBothLink(first_abs, second_abs, link);


                if (!hw_result.valid) {
                    continue;
                }
                if (isValid(hw_result.choice_index)) {
                    auto choice_index = hw_result.choice_index;


                    std::map<abs_pair, graph::choice_index_t> &map = headways_.line_headways[link.id];
                    map.try_emplace(map.end(), {first, second}, choice_index);

                    std::map<abs_pair, graph::choice_index_t> &rev_map = headways_.line_headways[rev_link.id];
                    rev_map.try_emplace(rev_map.end(), {first, second}, choice_index);

                    abstractions_.registerLinkHeadway(first_abs, {choice_index, graph::AlternativeIndex::FIRST});
                    abstractions_.registerLinkHeadway(second_abs, {choice_index, graph::AlternativeIndex::SECOND});
                } else {
                    std::map<abs_pair, graph::AlternativeIndex> &map = headways_.fixed_line_headways_[link.id];
                    map.try_emplace(map.end(), {first, second}, hw_result.alternative_index);

                    std::map<abs_pair, graph::AlternativeIndex> &rev_map = headways_.fixed_line_headways_[rev_link.id];
                    rev_map.try_emplace(rev_map.end(), {first, second}, hw_result.alternative_index);
                }
            }
        }
    }

    void HeadwayBuilder::buildStationHeadways() {

        TrackHeadwayBuilder thb{graph_, abstractions_, no_constrain_distances_, options_};
        for (auto const &node: model_.nodes) {
            buildStationHeadwaysAtNode(thb, node);
        }
    }

    void HeadwayBuilder::buildStationHeadwaysAtNode(TrackHeadwayBuilder &thb, const Node &node) {


        auto &abs_of_node = abstractions_.atNode(node.index);

        auto const wb_size = abs_of_node.wb.size();
        auto const total_size = abs_of_node.eb.size() + wb_size;

        auto get = [&](auto i) {
            if (i < wb_size) {
                return abs_of_node.wb[i];
            } else {
                return abs_of_node.eb[i - wb_size];
            }
        };


        for (auto i = 0; i < total_size; i++) {

            const auto &first_index = get(i);
            graph::TrainAtStationAbstraction &abs1 = abstractions_[first_index];

            for (auto j = i + 1; j < total_size; j++) {
                const auto &second_index = get(j);
                graph::TrainAtStationAbstraction &abs2 = abstractions_[second_index];


                if (abs1.getInCourse().duty == abs2.getInCourse().duty) {
                    // don't build headways for courses of the same duty
                    continue;
                }

                auto hw_result = thb.build(abs1, abs2);

                if (!hw_result.valid) {
                    continue;
                }

                if (isValid(hw_result.choice_index)) {
                    auto choice_id = hw_result.choice_index;
                    // cache the choice for static implications
                    auto &dir_mapped = headways_.station_headways[node.index];
                    auto *map = &dir_mapped.mixed;
                    if (j < wb_size) {
                        map = &dir_mapped.only_wb;
                    } else if (i >= wb_size) {
                        map = &dir_mapped.only_eb;
                    }
                    map->try_emplace(map->end(), {abs1.getIndex(), abs2.getIndex()}, choice_id);

                    // store the headways of each abstraction for later
                    abstractions_.registerTrackHeadway(abs1, {choice_id, graph::AlternativeIndex::FIRST});
                    abstractions_.registerTrackHeadway(abs2, {choice_id, graph::AlternativeIndex::SECOND});
                } else {
                    auto &dir_mapped = headways_.fixed_station_headways[node.index];
                    auto *map = &dir_mapped.mixed;
                    if (j < wb_size) {
                        map = &dir_mapped.only_wb;
                    } else if (i >= wb_size) {
                        map = &dir_mapped.only_eb;
                    }
                    map->try_emplace(map->end(), {abs1.getIndex(), abs2.getIndex()}, hw_result.alternative_index);
                }
            }
        }
    }
    void HeadwayBuilder::addStaticImplications() {

        StaticImplicationBuilder sib{model_, graph_, abstractions_, headways_, options_};

        for (auto node_id = 0; node_id < model_.nodes.size(); ++node_id) {

            if (options_.build_trivial_implications || options_.build_static_implications_in_same_direction) {
                sib.addStaticImplicationsAtNodeInDirection(node_id, Direction::EB);
                sib.addStaticImplicationsAtNodeInDirection(node_id, Direction::WB);
            }
            if (options_.build_trivial_implications || options_.build_static_implications_in_alternating_directions) {
                sib.addMixedDirStaticImplications(node_id);
            }
        }

        if (options_.build_static_implications_duty_consistency) {
            DutyConsistencyStaticImplicationBuilder dcsib{model_, graph_, abstractions_, options_};

            for (auto const &abs: abstractions_) {
                dcsib.addDutyConsistencyStaticImplication(abs->getIndex());
            }
        }
    }


}// namespace fb
