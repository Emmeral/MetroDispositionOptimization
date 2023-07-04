
#include "ModelBuilder.h"

#include <ranges>
#include <set>
#include <vector>

namespace fb {
    Model ModelBuilder::buildFromInput(const input::InputModel &input) {


        auto model = fb::Model{};
        buildNodeData(input, model);
        buildLinkData(input, model);
        buildCourseData(input, model);
        buildDutyData(input, model);
        buildTargetFrequencies(input, model);


        return model;
    }

    void ModelBuilder::buildNodeData(const input::InputModel &input, Model &output) {
        auto node_count = input.nodes.size();
        output.nodes.reserve(node_count);
        output.node_metadata.reserve(node_count);
        output.original_tracks_mapping.reserve(node_count);

        for (auto node_index = 0; node_index < node_count; ++node_index) {

            auto const &in = input.nodes[node_index];
            auto out = Node{};
            auto meta = NodeMeta{};
            std::vector<std::string> original_tracks{};


            out.index = node_index;
            out.category = in.category;
            out.short_turn_eb = in.st_eb;
            out.short_turn_wb = in.st_wb;
            out.lat = in.latitude;
            out.lon = in.longitude;

            meta.code = in.code;
            meta.name = in.name;

            std::set<std::string> all_tracks{};
            all_tracks.insert(in.eb_tracks.begin(), in.eb_tracks.end());
            all_tracks.insert(in.wb_tracks.begin(), in.wb_tracks.end());


            auto tracks = Tracks{};
            tracks.all_tracks.reserve(all_tracks.size());
            tracks.eb_tracks.reserve(in.eb_tracks.size());
            tracks.wb_tracks.reserve(in.wb_tracks.size());
            // map the tracks such that they are integral instead of strings
            auto j = 0;
            for (auto const &track: all_tracks) {
                original_tracks.push_back(track);

                Direction direction;
                bool eb = false;
                bool wb = false;
                if (std::ranges::find(in.eb_tracks, track) != in.eb_tracks.end()) {
                    direction = Direction::EB;
                    eb = true;
                    tracks.eb_tracks.push_back(j);
                }
                if (std::ranges::find(in.wb_tracks, track) != in.wb_tracks.end()) {
                    direction = Direction::WB;
                    wb = true;
                    tracks.wb_tracks.push_back(j);
                }
                if (eb && wb) {
                    direction = Direction::BOTH;
                    tracks.both_tracks.push_back(j);
                }
                tracks.all_tracks.push_back(direction);

                ++j;
            }
            out.tracks = std::move(tracks);

            output.nodes.push_back(std::move(out));
            output.node_metadata.push_back(std::move(meta));
            output.original_tracks_mapping.push_back(std::move(original_tracks));
            output.code_to_node_index.emplace(in.code, node_index);
        }

        output.base_station_values.resize(node_count, BaseStationValue());
        for (auto const &bsv: input.base_station_values) {
            auto node_index = output.code_to_node_index[bsv.node_id];
            output.base_station_values[node_index].of(bsv.end_timeband_seconds - 1, bsv.direction) = bsv.bsv;
        }
    }

    void ModelBuilder::buildLinkData(const input::InputModel &input, Model &output) {
        std::map<std::pair<node_index_t, node_index_t>, link_index_t> temp_link_map{};

        output.links.reserve(input.links.size());
        output.reverse_links.resize(input.links.size(), invalid<link_index_t>());
        auto link_id = 0;
        for (auto const &link: input.links) {

            auto out = Link{};
            out.direction = link.direction;
            auto const &start_node_id = output.code_to_node_index[link.start_node];
            out.link_start_node = start_node_id;
            auto const &end_node_id = output.code_to_node_index[link.end_node];
            out.link_end_node = end_node_id;
            out.id = link_id;


            // lookup reverse
            auto const &out_from_end = output.nodes[end_node_id].outgoing_links;
            for (auto const &l: out_from_end) {
                auto potential_rev = output.links[l];
                if (potential_rev.link_end_node == start_node_id) {
                    output.reverse_links[link_id] = l;
                    output.reverse_links[l] = link_id;
                }
            }


            output.nodes[start_node_id].outgoing_links.push_back(link_id);
            output.links.push_back(std::move(out));
            temp_link_map[{start_node_id, end_node_id}] = link_id;

            ++link_id;
        }

        for (auto const &hw: input.headways) {

            auto start_node = output.code_to_node_index[hw.link_start_node];
            auto end_node = output.code_to_node_index[hw.link_end_node];

            Link &link = output.links[temp_link_map.at({start_node, end_node})];


            link.headways.of(hw.start_activity_train_front, hw.end_activity_train_front, hw.start_activity_train_behind,
                             hw.end_activity_train_behind) = hw.minimum_headway_seconds;
            output.max_headway = std::max(output.max_headway, hw.minimum_headway_seconds);
        }

        for (auto const &mrt: input.minimum_runtimes) {
            auto start_node = output.code_to_node_index[mrt.link_start_node];
            auto end_node = output.code_to_node_index[mrt.link_end_node];
            Link &link = output.links[temp_link_map.at({start_node, end_node})];

            link.minimumRuntimes.of(mrt.start_activity, mrt.end_activity) = mrt.minimum_runtime;
            output.max_runtime = std::max(output.max_runtime, mrt.minimum_runtime);
        }
    }

    void ModelBuilder::buildCourseData(const input::InputModel &input, Model &output) {
        auto course_index = 0;
        for (auto const &course: input.train_headers) {

            Course out{};
            out.index = course_index;
            out.start_node = output.code_to_node_index[course.start_node];
            out.end_node = output.code_to_node_index[course.end_node];
            out.direction = course.direction;
            out.category = course.category;
            out.planned_start = course.start_seconds;
            out.planned_end = course.end_seconds;
            out.duty = invalid<duty_index_t>();// temporary for now

            output.courses.push_back(std::move(out));
            output.code_to_course_index.emplace(course.train_course_id, course_index);
            output.course_code.push_back(course.train_course_id);

            ++course_index;
        }

        for (auto const &si: input.schedule) {

            auto c_index = output.code_to_course_index[si.course_id];

            auto &course = output.courses[c_index];


            ScheduleItem sched{};
            sched.index = si.seq - 1;
            sched.node = output.code_to_node_index[si.node];
            sched.departure = si.departure_seconds;
            sched.arrival = si.arrival_seconds;
            sched.original_activity = si.activity;

            auto const node_index = output.code_to_node_index[si.node];
            auto const &tracks = output.original_tracks_mapping[node_index];

            // the index is the track as a track is only a number
            auto const index = std::ranges::find(tracks, si.track) - tracks.begin();
            sched.original_track = index;

            const auto schedule_size = si.seq + si.rev_seq - 1;
            if (course.schedule.size() < schedule_size) {
                course.schedule.resize(schedule_size);
            }

            course.schedule[si.seq - 1] = sched;
        }
    }

    void ModelBuilder::buildDutyData(const input::InputModel &input, Model &output) {
        std::map<std::string, duty_index_t> duty_index_map{};
        auto duty_index = 0;
        for (auto const &duty_elem: input.rolling_stock_duties) {

            if (!duty_index_map.contains(duty_elem.duty_id)) {
                RSDuty duty{};
                duty.index = duty_index;
                duty.elements.resize(duty_elem.seq + duty_elem.rev_seq - 1);

                output.duties.push_back(std::move(duty));
                duty_index_map.emplace(duty_elem.duty_id, duty_index);
                output.duty_code.push_back(duty_elem.duty_id);

                ++duty_index;
            }

            auto current_duty_index = duty_index_map[duty_elem.duty_id];
            auto duty_element_index = duty_elem.seq - 1;

            RSDutyElement out{};

            out.index = duty_element_index;
            out.planned_start_time = duty_elem.start_time_seconds;
            out.planned_end_time = duty_elem.end_time_seconds;
            out.start_node = output.code_to_node_index[duty_elem.start_node];
            out.end_node = output.code_to_node_index[duty_elem.end_node];
            out.event = duty_elem.event_type;


            if (out.event == EventType::TRAIN) {
                out.course = output.code_to_course_index[duty_elem.train_course_id];

                // set the duty of the course
                Course &course = output.courses[out.course];
                course.duty = current_duty_index;
                course.index_in_duty = duty_element_index;

            } else {
                out.course = invalid<course_index_t>();
            }


            output.duties[current_duty_index].elements[duty_element_index] = out;
        }
    }
    void ModelBuilder::buildTargetFrequencies(const input::InputModel &input, Model &output) {


        std::vector<input::TargetFrequency> copy = input.target_frequencies;
        std::ranges::sort(copy, [](auto const &a, auto const &b) { return a.end_time_seconds < b.end_time_seconds; });


        if (copy[0].start_time_seconds != 0 && copy.back().end_time_seconds > SECONDS_PER_DAY) {

            input::TargetFrequency first_freq{};
            first_freq.start_time_seconds = 0;
            first_freq.end_time_seconds = copy.back().end_time_seconds % SECONDS_PER_DAY;
            first_freq.threshold_headway_seconds = copy.back().threshold_headway_seconds;

            copy.insert(copy.begin(), first_freq);

            copy.back().end_time_seconds = SECONDS_PER_DAY;
        }

        TargetFrequencies tfq{};
        for (auto const &f: copy) {
            tfq.end_times.push_back(f.end_time_seconds);
            tfq.headway_seconds.push_back(f.threshold_headway_seconds);
        }
        output.target_frequencies = std::move(tfq);


        for (auto const &fmn: input.frequency_measure_nodes) {
            if (fmn.direction == Direction::WB || fmn.direction == Direction::BOTH) {
                output.wb_frequency_measure = output.code_to_node_index[fmn.node_id];
            }
            if (fmn.direction == Direction::EB || fmn.direction == Direction::BOTH) {
                output.eb_frequency_measure = output.code_to_node_index[fmn.node_id];
            }
        }

        if (input.frequency_measure_nodes.size() > 2) {
            std::cout << "[WARN] Got more than 2 frequency measure nodes. Ignoring them";
        }
    }
}// namespace fb
