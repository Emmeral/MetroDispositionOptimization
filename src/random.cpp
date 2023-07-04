//
// Created by flobe on 15.03.2023.
//
#include "random.h"
namespace fb{


    void rolling_stock_analyses(const input::InputModel &data) {
        std::map<std::string, std::set<Event>> duty_map{};

        for (auto &duty: data.duty_start_end) {
            duty_map[duty.start_node].insert(Event{duty.start_time_seconds, true, duty.duty_id});
            duty_map[duty.end_node].insert(Event{duty.end_time_seconds, false, duty.duty_id});
        }

        auto rolling_stock_sum = 0;
        for (auto const &entry: duty_map) {


            auto min = 0;
            auto count = 0;
            std::cout << entry.first << ":"
                      << "\n";
            for (auto const &event: entry.second) {
                if (event.isStart) {
                    --count;
                } else {
                    ++count;
                }
                std::cout << event << " Count: " << count << "\n";
                min = std::min(count, min);
            }
            std::cout << "Capacity:" << -min << "\n";
            std::cout << "\n";
            rolling_stock_sum += -min;
        }

        std::cout << "Rolling Stock Sum:" << rolling_stock_sum << "\n";



    }
    void station_track_connection_analyses(const input::InputModel &data) {
        using namespace input;
        // check for every link if:
        // start has more than one track in direction + end has more than one track in direction

        std::map<input::id_t, const input::Node *> id_to_node_map{};

        for (auto const &node: data.nodes) {
            id_to_node_map[node.code] = &node;
        }


        for (auto const &link: data.links) {

            auto const &start_node = *id_to_node_map[link.start_node];
            auto const &end_node = *id_to_node_map[link.end_node];

            if (link.direction == Direction::EB) {
                if (start_node.eb_tracks.size() > 1 && end_node.eb_tracks.size() > 1) {
                    std::cout << start_node.code << "-----" << link.direction << "--->" << end_node.code << "\n";
                }
            } else if (link.direction == Direction::WB) {
                if (start_node.wb_tracks.size() > 1 && end_node.wb_tracks.size() > 1) {
                    std::cout << start_node.code << "-----" << link.direction << "--->" << end_node.code << "\n";
                }
            } else if (link.direction == Direction::BOTH) {
                if ((start_node.wb_tracks.size() > 1 && end_node.wb_tracks.size() > 1) ||
                    (start_node.eb_tracks.size() > 1 && end_node.eb_tracks.size() > 1)) {
                    std::cout << start_node.code << "-----" << link.direction << "--->" << end_node.code << "\n";
                }
            }
        }
    }
    void trackCountInDirection(const Model &model) {


        std::vector<int> counts{};
        counts.resize(10, 0);

        for (auto const &node: model.nodes) {
            auto wb_tracks = node.tracks.countInDirection(Direction::WB);
            auto eb_tracks = node.tracks.countInDirection(Direction::EB);

            counts[wb_tracks]++;
            counts[eb_tracks]++;
        }

        std::cout << "Track Counts"
                  << "\n";
        for (auto i = 0; i < counts.size(); i++) {
            std::cout << i << " : " << counts[i] << "\n";
        }
    }
    void startNodeTrackCount(const Model &data) {
        for (auto const &d: data.duties) {

            RSDutyElement const &first_duty = d.elements[0];
            if (first_duty.course == std::numeric_limits<decltype(first_duty.course)>::max()) {
                continue;
            }
            auto const &c = data.courses[first_duty.course];

            Node const &start_node = data.nodes[c.start_node];

            int size;
            if (c.direction == Direction::EB) {
                size = start_node.tracks.eb_tracks.size();
            } else {
                size = start_node.tracks.wb_tracks.size();
            }
            if (size > 1) {
                std::cout << data.node_metadata[c.start_node].code << " " << data.duty_code[d.index] << " "
                          << data.course_code[c.index] << c.direction << "\n";
            }
        }
    }
    bool dutyAlwaysStartsWithStop(const Model &data) {

        bool found_counter_example = false;

        for (auto const &d: data.duties) {

            RSDutyElement const &first_duty = d.elements[0];

            if (first_duty.event == EventType::RESERVE) {
                continue;
            }

            auto const &first_course = data.courses[first_duty.course];

            ScheduleItem const &first_sched = first_course.schedule[0];

            if (first_sched.original_activity != Activity::STOP) {
                found_counter_example = true;
                std::cout << data.course_code[first_course.index] << " " << data.node_metadata[first_sched.node].code
                          << "\n";
            }
        }
        return found_counter_example;
    }
    bool dutyKeepsTrackAfterCourseChange(const Model &data) {

        bool found_counter_example = false;

        for (auto const &d: data.duties) {

            RSDutyElement const &first_duty = d.elements[0];

            if (first_duty.event == EventType::RESERVE) {
                continue;
            }

            auto last_track = invalid<track_id_t>();
            for (auto const &dutyElem: d.elements) {

                if (dutyElem.event != EventType::TRAIN) {
                    continue;
                }

                auto const &course = data.courses[dutyElem.course];
                auto track = course.schedule[0].original_track;
                if (isValid(last_track)) {
                    if (track != last_track) {
                        found_counter_example = true;
                        std::cout << "Track Jump: " << data.duty_code[d.index] << " " << data.course_code[course.index]
                                  << "\n";
                    }
                }
                last_track = course.schedule[course.schedule.size() - 1].original_track;
            }
        }
        return found_counter_example;
    }
    seconds_t minRollingStockDutyStart(const Model &data, bool ignoreReserve) {

        auto min = invalid<seconds_t>();
        auto id = invalid<duty_index_t>();

        for (auto const &d: data.duties) {

            RSDutyElement const &first_duty = d.elements[0];

            if (ignoreReserve && first_duty.event == EventType::RESERVE) {
                continue;
            }

            auto start = first_duty.planned_start_time;
            if (start < min) {
                min = start;
                id = d.index;
            }
        }

        std::cout << data.duty_code[id] << ": " << min << "\n";

        return min;
    }
    double averageStaticImplicationCount(const graph::AlternativeGraph &graph,
                                             std::vector<graph::ChoiceType> included_types) {


        unsigned long sum = 0;
        long count = 0;

        for (auto const &c: graph.choices) {

            if (std::ranges::find(included_types, graph.choice_metadata[c.index].type) == included_types.end()) {
                continue;
            }

            for (auto ai: {graph::AlternativeIndex::FIRST, graph::AlternativeIndex::SECOND}) {
                std::set<graph::FullAlternativeIndex> implied{};
                std::queue<graph::FullAlternativeIndex> to_process{};

                implied.insert({c.index, ai});
                to_process.push({c.index, ai});

                while (!to_process.empty()) {
                    auto current = to_process.front();
                    to_process.pop();
                    graph::Alternative const &alternative =
                            graph.choices[current.choice_index].alternative(current.alternative_index);
                    for (auto implication: alternative.static_implications) {
                        auto inserted = implied.insert(implication).second;
                        if (inserted) {
                            to_process.push(implication);
                        }
                    }
                }
                sum += implied.size();
                count++;
            }
        }

        return sum / (double) count;
    }
    void printCriticalPath(const graph::GraphWithMeta &graph, const DistanceState &distance_state) {

        auto const &dfs = distance_state.distance_from_start;
        auto max = std::numeric_limits<distance_t>::min();
        auto max_node = invalid<graph::node_index_t>();

        for (auto mn: graph.measure_nodes.getNodes()) {
            if (dfs[mn] > max) {
                max = dfs[mn];
                max_node = mn;
            }
        }

        auto before = invalid<node_index_t>();
        auto current = max_node;
        std::cout << "Critical Path: \n";
        while (isValid(current)) {

            if (isValid(before)) {

                auto const *edge = graph.graph.getEdgeFromTo(current, before);

                if (edge != nullptr) {

                    std::cout << "Edge: " << edge->index << " fixed: " << edge->is_fixed
                              << " weight: " << graph.graph.getEdgeWeight(*edge, dfs[current]) << "\n";
                }
            }

            std::cout << "Node: " << current << " Distance: " << dfs[current];
            auto const *tas = graph.abstractions.ofNode(current);
            if (tas != nullptr) {
                std::cout << " Abs: " << tas->getIndex();
            }
            std::cout << "\n";

            before = current;
            current = distance_state.predecessor[current];
        }
    }

    void changeEndTimeStats(const Model &model) {


        std::map<int, std::set<std::string>> places{};
        std::map<int, std::set<std::string>> duties{};
        std::map<int, int> counts{};

        for (auto const &d: model.duties) {
            for (auto const &de: d.elements) {
                if (de.event != EventType::CHANGE_END) {
                    continue;
                }
                auto diff = ((int) de.planned_end_time) - de.planned_start_time;
                counts[diff]++;
                places[diff].insert(model.node_metadata[de.start_node].code);
                duties[diff].insert(model.duty_code[d.index]);
            }
        }

        std::cout << "Change end Stats \n";
        for (auto &count: counts) {
            std::cout << count.first << " : " << count.second << " ";

            for (auto &place: places[count.first]) {
                std::cout << place << " ";
            }

            for (auto &duty: duties[count.first]) {
                std::cout << duty << " ";
            }
            std::cout << "\n";
        }
    }
}// namespace fb
