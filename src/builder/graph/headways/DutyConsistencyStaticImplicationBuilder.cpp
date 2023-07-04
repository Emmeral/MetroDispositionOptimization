
#include "DutyConsistencyStaticImplicationBuilder.h"

namespace fb {
    DutyConsistencyStaticImplicationBuilder::DutyConsistencyStaticImplicationBuilder(
            const Model &model, graph::AlternativeGraph &graph, const graph::Abstractions &abstractions,
            const GraphBuilderOptions &options)
        : model_(model), graph_(graph), abstractions_(abstractions), options_(options) {}


    void DutyConsistencyStaticImplicationBuilder::addDutyConsistencyStaticImplication(graph::abs_index_t abs_id) {


        auto const &link_out = abstractions_.getOutgoingHeadways(abs_id).link_headways;
        std::vector<std::vector<AltAbsPair>> link_alternatives_by_duty = getAlternativesByDuty(link_out);
        createImplications(link_alternatives_by_duty);

        auto const &track_out = abstractions_.getOutgoingHeadways(abs_id).track_headways;
        std::vector<std::vector<AltAbsPair>> track_alternatives_by_duty = getAlternativesByDuty(track_out);
        createImplications(track_alternatives_by_duty);
    }
    void DutyConsistencyStaticImplicationBuilder::createImplications(
            std::vector<std::vector<AltAbsPair>> &alternatives_by_duty) {
        for (auto &vec: alternatives_by_duty) {
            std::sort(vec.begin(), vec.end());

            for (int i = 0; i < vec.size(); ++i) {
                for (int j = i + 1; j < vec.size(); ++j) {
                    auto &first = vec[i];
                    auto &second = vec[j];
                    // before the prior -> before the latter
                    this->graph_.alternative(first.alt).static_implications.push_back(second.alt);
                    // latter before me -> prior before me
                    this->graph_.alternative(graph::otherAlternative(second.alt))
                            .static_implications.push_back(graph::otherAlternative(first.alt));
                }
            }
        }
    }
    std::vector<std::vector<DutyConsistencyStaticImplicationBuilder::AltAbsPair>>
    DutyConsistencyStaticImplicationBuilder::getAlternativesByDuty(
            const std::vector<graph::FullAlternativeIndex> &outgoing) {

        std::vector<std::vector<AltAbsPair>> alternatives_by_duty{};
        alternatives_by_duty.resize(model_.duties.size(), {});

        for (auto alt: outgoing) {
            const graph::TaggedChoiceMetadata &tagged_meta = graph_.choice_metadata[alt.choice_index];
            auto other = invalid<graph::abs_index_t>();

            if (tagged_meta.type == graph::ChoiceType::LINE_HEADWAY) {
                graph::LineHeadwayMetadata const &meta = tagged_meta.data.lh;
                other = alt.alternative_index == graph::AlternativeIndex::FIRST ? meta.second : meta.first;
            } else if (tagged_meta.type == graph::ChoiceType::TRACK_HEADWAY) {
                graph::TrackHeadwayMetadata const &meta = tagged_meta.data.th;
                other = alt.alternative_index == graph::AlternativeIndex::FIRST ? meta.second : meta.first;
            }
            auto const &other_abs = abstractions_[other];
            Course const &course = other_abs.getOutCourse();
            auto duty = course.duty;
            const ScheduleItem &si = *other_abs.getOriginalOutScheduleItem();
            alternatives_by_duty[duty].push_back({alt, other, course.index_in_duty, si.index});
        }
        return alternatives_by_duty;
    }

}// namespace fb
