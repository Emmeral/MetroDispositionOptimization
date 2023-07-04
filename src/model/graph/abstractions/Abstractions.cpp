
#include "Abstractions.h"

namespace fb::graph {

    TrainAtStationAbstraction const *Abstractions::ofIndex(abs_index_t const &index) const {
        return isValid(index) ? all[index] : nullptr;
    }

    TrainAtStationAbstraction const *Abstractions::next(TrainAtStationAbstraction const &current) const {
        return next(current.getIndex());
    }
    TrainAtStationAbstraction const *Abstractions::next(abs_index_t const &index) const {
        if (next_indices.size() <= index) {
            return nullptr;
        }
        auto next_index = next_indices[index];
        return ofIndex(next_index);
    }
    TrainAtStationAbstraction const *Abstractions::prior(abs_index_t const &index) const {
        if (prior_indices.size() <= index) {
            return nullptr;
        }
        auto prior_index = prior_indices[index];
        return ofIndex(prior_index);
    }

    TrainAtStationAbstraction const *Abstractions::prior(TrainAtStationAbstraction const &current) const {
        return prior(current.getIndex());
    }

    TrainAtStationAbstraction const *Abstractions::nextAbsOfCourse(TrainAtStationAbstraction const &current,
                                                                   course_index_t course) const {
        if (current.getIndex() == course_ends[course]) {
            return nullptr;
        } else {
            return next(current);
        }
    }

    TrainAtStationAbstraction const *Abstractions::ofNode(node_index_t node) const {

        auto it =
                std::lower_bound(all.begin(), all.end(), node, [](TrainAtStationAbstraction const *tas, auto const &n) {
                    return tas->getOffset() + tas->getNodeCount() <= n;
                });

        if (it == all.end()) {
            return nullptr;
        }
        TrainAtStationAbstraction const *tas = *it;

        if (node < tas->getOffset() + tas->getNodeCount()) {
            return tas;
        }
        return nullptr;
    }
    TrainAtStationAbstraction &Abstractions::operator[](const abs_index_t &index) { return *all[index]; }
    size_t Abstractions::size() const { return all.size(); }
    bool Abstractions::empty() const { return all.empty(); }
    TrainAtStationAbstraction const &Abstractions::operator[](const abs_index_t &index) const { return *all[index]; }
    TrainAtStationAbstraction const *Abstractions::firstAbsOfCourse(course_index_t course) const {

        if (first_abs_of_course.size() <= course) {
            return nullptr;
        }
        auto first_index = first_abs_of_course[course];
        return ofIndex(first_index);
    }
    TrainAtStationAbstraction const *Abstractions::courseEnd(course_index_t course) const {

        if (course_ends.size() <= course) {
            return nullptr;
        }
        auto end_index = course_ends[course];
        return ofIndex(end_index);
    }
    void Abstractions::registerLinkHeadway(const TrainAtStationAbstraction &first, const FullAlternativeIndex &choice) {
        outgoing_headways[first.getIndex()].link_headways.push_back(choice);
    }
    void Abstractions::registerTrackHeadway(const TrainAtStationAbstraction &first,
                                            const FullAlternativeIndex &choice) {
        outgoing_headways[first.getIndex()].track_headways.push_back(choice);
    }

    DirectionSortedAbstractions const &Abstractions::atNode(node_index_t node) const { return node_sorted[node]; }
    TrainAtStationAbstraction *Abstractions::next(const TrainAtStationAbstraction &current) {
        return next(current.getIndex());
    }
    TrainAtStationAbstraction *Abstractions::next(const abs_index_t &index) {
        if (next_indices.size() <= index) {
            return nullptr;
        }
        auto next_index = next_indices[index];
        return ofIndex(next_index);
    }
    TrainAtStationAbstraction *Abstractions::ofIndex(const abs_index_t &index) {
        return isValid(index) ? all[index] : nullptr;
    }
    void Abstractions::registerSuccessor(const TrainAtStationAbstraction &first,
                                         const TrainAtStationAbstraction &second, const Link &link) {
        next_indices[first.getIndex()] = second.getIndex();
        prior_indices[second.getIndex()] = first.getIndex();

        link_sorted[link.id].emplace_back(first.getIndex(), second.getIndex());
    }
    TrainAtStationAbstraction *Abstractions::prior(const TrainAtStationAbstraction &current) {
        return prior(current.getIndex());
    }
    TrainAtStationAbstraction *Abstractions::prior(const abs_index_t &index) {
        if (prior_indices.size() <= index) {
            return nullptr;
        }
        auto prior_index = prior_indices[index];
        return ofIndex(prior_index);
    }
    std::vector<std::pair<abs_index_t, abs_index_t>> const &Abstractions::atLink(link_index_t link) const {
        return link_sorted[link];
    }
    void Abstractions::updateFirstAndLastOfCourse(
            abs_index_t id, const TrainAtStationAbstraction &created) {// set first and last abs of course
        auto const &out_course = created.getOutCourse();
        auto const &in_course = created.getInCourse();


        auto const *out_current_first = firstAbsOfCourse(out_course.index);
        auto const *in_current_first = firstAbsOfCourse(in_course.index);
        if (out_current_first == nullptr ||
            out_current_first->scheduleItemOfCourse(out_course)->index > created.getOriginalOutScheduleItem()->index) {
            first_abs_of_course[out_course.index] = id;
        }
        if (in_current_first == nullptr ||
            in_current_first->scheduleItemOfCourse(in_course)->index > created.getOriginalInScheduleItem()->index) {
            first_abs_of_course[in_course.index] = id;
        }

        auto const *out_current_last = courseEnd(out_course.index);
        auto const *in_current_last = courseEnd(in_course.index);
        if (out_current_last == nullptr ||
            out_current_last->scheduleItemOfCourse(out_course)->index < created.getOriginalOutScheduleItem()->index) {
            course_ends[in_course.index] = id;
        }
        if (in_current_last == nullptr ||
            in_current_last->scheduleItemOfCourse(in_course)->index < created.getOriginalInScheduleItem()->index) {
            course_ends[out_course.index] = id;
        }
    }

    template<>
    std::deque<MultiTrackSingleAbstraction> &Abstractions::storageForType() {
        return multi_track_single;
    };

    template<>
    std::deque<TrackDecidedSingleAbstraction> &Abstractions::storageForType() {
        return track_decided_single;
    };
    template<>
    std::deque<PredefinedActivityMultiTrackAbstraction> &Abstractions::storageForType() {
        return activity_decided_single;
    };

    template<>
    std::deque<FullyDecidedSingleAbstraction> &Abstractions::storageForType() {
        return fully_decided_single;
    };

    template<>
    std::deque<RealizedSingleAbstraction> &Abstractions::storageForType() {
        return realized_single;
    };

    template<>
    std::deque<MultiTrackLinkAbstraction> &Abstractions::storageForType() {
        return multi_track_link;
    };

    template<>
    std::deque<FullyDecidedLinkAbstraction> &Abstractions::storageForType() {
        return fully_decided_link;
    };

    template<>
    std::deque<RealizedLinkAbstraction> &Abstractions::storageForType() {
        return realized_link;
    };

}// namespace fb::graph
