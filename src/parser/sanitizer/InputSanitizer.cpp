
#include "InputSanitizer.h"
#include "Runtimes.h"
#include "model/definitions.h"
#include "model/enums/Enums.h"
#include <cassert>
#include <map>
#include <ranges>
#include <set>
#include <utility>


namespace fb {
    using namespace input;


    InputSanitizer::InputSanitizer(InputModel &input) : input_(input), id_to_node_map_{}, schedule_map_{} {

        for (auto &node: input.nodes) {
            id_to_node_map_[node.code] = &node;
        }


        for (auto const &si: input_.schedule) {
            if (!schedule_map_.contains(si.course_id)) {
                int course_length = si.seq + si.rev_seq - 1;
                // does this create a vector of the given length?
                schedule_map_.emplace(si.course_id, course_length);
            }
            schedule_map_[si.course_id][si.seq - 1] = &si;
        }
    }


    void InputSanitizer::sanitize() {

        overrideWrongIndividualData();
        expandNodeTracks();
        correctMinimalRuntimes();
        correctMinimalHeadways();
    }
    void InputSanitizer::expandNodeTracks() {

        std::map<fb::input::id_t, fb::Direction> course_to_direction_map{};

        for (auto const &course: input_.train_headers) {
            course_to_direction_map[course.train_course_id] = course.direction;
        }

        for (auto const &item: input_.schedule) {

            auto node = id_to_node_map_.at(item.node);
            auto direction = course_to_direction_map.at(item.course_id);

            std::vector<std::string> *tracks = &node->wb_tracks;
            if (direction == fb::Direction::WB) {
                tracks = &node->wb_tracks;
            } else if (direction == fb::Direction::EB) {
                tracks = &node->eb_tracks;
            }
            // add tracks to node if not existent
            if (std::ranges::find(*tracks, item.track) == std::ranges::end(*tracks)) {
                tracks->push_back(item.track);
                this->statistics_.inserted_tracks++;
            }
        }
    }
    void InputSanitizer::overrideWrongIndividualData() {

        // fix gideapark having to many tracks
        if (id_to_node_map_.contains("GIDEPKM")) {
            auto gideapark = id_to_node_map_.at("GIDEPKM");
            gideapark->wb_tracks = {"1"};
            gideapark->eb_tracks = {"1"};
        }

        // add additional tracks that are required
        if (id_to_node_map_.contains("RDNGSTN")) {

            auto rdngstn = id_to_node_map_.at("RDNGSTN");

            for (auto t: {"E1", "E2", "E3", "E4"}) {
                rdngstn->wb_tracks.emplace_back(t);
                rdngstn->eb_tracks.emplace_back(t);
            }
        }

        // manually add short turn information
        if (id_to_node_map_.contains("WBRNPKS")) {
            auto wbrnpks = id_to_node_map_.at("WBRNPKS");
            wbrnpks->st_wb = ShortTurn::BOTH;
        }


        for (auto id: {"HTRWTM4", "HTRWTM5", "RDNGSTN", "SHENFMS"}) {
            if (id_to_node_map_.contains(id)) {
                auto node = id_to_node_map_.at(id);
                node->st_wb = ShortTurn::BOTH;
                node->st_eb = ShortTurn::BOTH;
            }
        }


        seconds_t end_time = 0;

        for (auto i = 0; i < input_.rolling_stock_duties.size(); ++i) {
            auto &de = input_.rolling_stock_duties[i];
            if (de.duty_id == "RSDuty_28") {
                if (de.seq == 24) {
                    end_time = de.start_time_seconds + 420;
                    de.end_time_seconds = end_time;
                }
                if (de.seq == 25) {
                    de.start_time_seconds = end_time;
                    break;
                }
            } else {
                i += de.rev_seq;
            }
        }


        for (auto i = 0; i < input_.schedule.size(); ++i) {

            auto &si = input_.schedule[i];
            if (si.course_id == "9N82RL#1") {
                if (si.seq == 19) {
                    si.departure_seconds = 36030;// wait in node for overtake
                    si.departure_hhmmss = "10:00:30";
                    si.activity = Activity::STOP;
                    si.track = "B";
                }
            } else if (si.course_id == "9Y50RL#1") {
                if (si.seq == 19) {
                    si.departure_seconds = 30780;// wait in node for overtake
                    si.departure_hhmmss = "08:33:00";
                    si.activity = Activity::STOP;
                    si.track = "B";
                }
            } else if (si.course_id == "5Y63RL#1") {
                if (si.seq == 3) {
                    si.departure_seconds = 32580;// wait for overtake
                    si.departure_hhmmss = "09:03:00";
                    si.activity = Activity::STOP;
                    si.track = "B";
                }
                if (si.seq == 4) {
                    si.arrival_seconds = 32640;// delay from overtake
                    si.arrival_hhmmss = "09:04:00";
                    si.departure_seconds = 32640;
                    si.departure_hhmmss = "09:04:00";
                }
                if (si.seq == 5) {
                    si.arrival_seconds = 32700;// delay from overtake
                    si.arrival_hhmmss = "09:05:00";
                    si.departure_seconds = 32700;
                    si.departure_hhmmss = "09:05:00";
                }

            } else if (si.course_id == "9T74RN") {
                if (si.seq == 7) {// fixing track headway problem by slightly earlier departure
                    si.departure_seconds = 63960;
                    si.departure_hhmmss = "17:46:00";
                }
            } else {
                i += si.rev_seq - 1;
            }
        }
    }


    void InputSanitizer::correctMinimalRuntimes() {

        std::map<std::pair<input::id_t, input::id_t>, Runtimes> runtimes_map{};

        for (auto const &rt: input_.minimum_runtimes) {

            auto key = std::make_pair(rt.link_start_node, rt.link_end_node);
            runtimes_map.try_emplace(key, rt.link_start_node, rt.link_end_node);

            runtimes_map.at(key).get(rt.start_activity, rt.end_activity) = rt.minimum_runtime;
        }


        for (auto const &[course_id, schedule]: schedule_map_) {
            for (auto i = 1; i < schedule.size(); ++i) {
                auto const &current = schedule[i];
                auto const &last = schedule[i - 1];

                auto &rt = runtimes_map.at({last->node, current->node});
                auto time = current->arrival_seconds - last->departure_seconds;
                rt.set(last->activity, current->activity, time, true);
            }
        }

        for (auto &[_, rt]: runtimes_map) {

            int fsc = rt.fromScheduleCount();
            switch (fsc) {
                case 1: {
                    if (rt.fs_pass_pass) {
                        const uint32_t reference = rt.pass_pass;
                        rt.pass_stop = reference + 20;
                        rt.stop_pass = reference + 20;
                        rt.stop_stop = reference + 40;
                    } else if (rt.fs_pass_stop) {
                        const uint32_t reference = rt.pass_stop;
                        rt.pass_pass = reference - 20;
                        rt.stop_pass = reference;
                        rt.stop_stop = reference + 20;
                    } else if (rt.fs_stop_pass) {
                        const uint32_t reference = rt.stop_pass;
                        rt.pass_pass = reference - 20;
                        rt.pass_stop = reference;
                        rt.stop_stop = reference + 20;
                    } else if (rt.fs_stop_stop) {
                        const uint32_t reference = rt.stop_stop;
                        rt.pass_pass = reference - 40;
                        rt.pass_stop = reference - 20;
                        rt.stop_pass = reference - 20;
                    }
                    break;
                }
                case 2: {
                    if (rt.fs_pass_pass) {
                        if (rt.fs_stop_pass) {
                            rt.pass_stop = rt.pass_pass + 20;
                            rt.stop_stop = std::max(rt.pass_stop, rt.stop_pass);
                        } else if (rt.fs_pass_stop) {
                            rt.stop_pass = rt.pass_pass + 20;
                            rt.stop_stop = std::max(rt.pass_stop, rt.stop_pass);
                        } else if (rt.stop_stop) {
                            if (rt.stop_stop - rt.pass_pass < 40) {
                                throw std::runtime_error("Cannot infer correct runtime");
                            }
                            rt.pass_stop = rt.pass_pass + 20;
                            rt.stop_pass = rt.pass_pass + 20;
                        }
                    } else if (rt.fs_stop_stop) {
                        if (rt.fs_stop_pass) {
                            rt.pass_stop =
                                    std::min(rt.stop_stop - 20, rt.stop_pass);// always go for the smaller runtime
                            rt.pass_pass = std::min(rt.pass_stop, rt.stop_pass) - 20;
                        } else if (rt.fs_pass_stop) {
                            rt.stop_pass =
                                    std::min(rt.stop_stop - 20, rt.pass_stop);// always go for the smaller runtime
                            rt.pass_pass = std::min(rt.pass_stop, rt.stop_pass) - 20;
                        }

                    } else if (rt.fs_stop_pass && rt.fs_pass_stop) {
                        auto min = std::min(rt.pass_stop, rt.stop_pass);
                        auto max = std::max(rt.pass_stop, rt.stop_pass);
                        rt.pass_pass = min - 20;
                        rt.stop_stop = max + 20;
                    }
                    break;
                }
                case 3: {
                    if (!rt.fs_pass_pass) {
                        rt.pass_pass = std::min(rt.stop_pass, rt.pass_stop) - 20;
                    } else if (!rt.fs_pass_stop) {
                        rt.pass_stop = rt.pass_pass + 20;
                        assert(rt.pass_stop + 20 <= rt.stop_stop);// sanity check
                    } else if (!rt.fs_stop_pass) {
                        rt.stop_pass = rt.pass_pass + 20;
                        assert(rt.pass_stop + 20 <= rt.stop_stop);// sanity check
                    } else if (!rt.fs_stop_stop) {
                        rt.stop_stop = std::max(rt.pass_stop, rt.stop_pass) + 20;
                    }
                    break;
                }
                case 4:// all values are taken from the schedule -> don't do anything
                default: {
                    // can't happen
                }
            }
        }


        for (auto &rt: input_.minimum_runtimes) {
            auto key = std::make_pair(rt.link_start_node, rt.link_end_node);

            const uint32_t &new_rt = runtimes_map.at(key).get(rt.start_activity, rt.end_activity);
            if (rt.minimum_runtime != new_rt) {
                rt.minimum_runtime = new_rt;
                statistics_.changed_runtimes++;
            }
        }
    }


    struct Departure {
        uint32_t departure;

        Activity start_activity;
        Activity end_activity;

        input::id_t course_id;

        friend auto operator<=>(const Departure &lhs, const Departure &rhs) = default;
    };

    struct Headways {


        std::array<uint32_t, 16> headways;

        uint32_t &get(Activity a1, Activity a2, Activity a3, Activity a4) {
            return headways[static_cast<bool>(a1) + static_cast<bool>(a2) * 2 + static_cast<bool>(a3) * 4 +
                            static_cast<bool>(a4) * 8];
        }
        uint32_t const &get(Activity a1, Activity a2, Activity a3, Activity a4) const {
            return headways[static_cast<bool>(a1) + static_cast<bool>(a2) * 2 + static_cast<bool>(a3) * 4 +
                            static_cast<bool>(a4) * 8];
        }
    };


    void InputSanitizer::correctMinimalHeadways() {

        std::map<std::pair<input::id_t, input::id_t>, std::set<Departure>> departure_map{};


        for (auto const &[id, course]: schedule_map_) {

            for (auto i = 1; i < course.size(); ++i) {

                Departure dep{};
                const ScheduleItem &last = *course[i - 1];
                const ScheduleItem &current = *course[i];
                dep.start_activity = last.activity;
                dep.end_activity = current.activity;
                dep.departure = last.departure_seconds;
                dep.course_id = id;

                departure_map[{last.node, current.node}].emplace(std::move(dep));
            }
        }

        std::map<std::pair<input::id_t, input::id_t>, Headways> headways_map{};

        for (auto const &[key, set]: departure_map) {

            Headways &hws = headways_map[key];
            hws.headways.fill(std::numeric_limits<uint32_t>::max());

            const Departure *last = nullptr;
            for (auto const &current: set) {
                if (last == nullptr) {
                    last = &current;
                    continue;
                }

                auto &hw =
                        hws.get(last->start_activity, last->end_activity, current.start_activity, current.end_activity);
                auto const difference = current.departure - last->departure;
                if (difference < hw) {
                    hw = difference;
                }
                last = &current;
            }
        }

        for (auto &hw: input_.headways) {

            auto const key = std::make_pair(hw.link_start_node, hw.link_end_node);
            if (!headways_map.contains(key)) {
                continue;
            }

            auto const &headways = headways_map[key];
            auto min_schedule_headway = headways.get(hw.start_activity_train_front, hw.end_activity_train_front,
                                                     hw.start_activity_train_behind, hw.end_activity_train_behind);

            min_schedule_headway = std::max(30u, min_schedule_headway);
            if (hw.minimum_headway_seconds > min_schedule_headway) {
                hw.minimum_headway_seconds = min_schedule_headway;
                ++statistics_.changed_headways;
            }
        }
    }


}// namespace fb
