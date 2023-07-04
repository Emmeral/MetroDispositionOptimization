
#include "AmendmentWriter.h"
#include "model/input/InputModel.h"
#include "parser/DataParser.h"
#include "penalty/DetailedPenalty.h"
#include <filesystem>
#include <fstream>

namespace fb {
    AmendmentWriter::AmendmentWriter(const Model &model) : model_(model) {}


    void AmendmentWriter::writeAmendment(const TimetableAmendment &amendment, const std::string &folder,
                                         const std::vector<std::string> &validation_result,
                                         DetailedPenalty const *penalty) {

        auto export_model = exportModel(amendment);
        saveToFolder(folder, amendment, export_model, validation_result, penalty);
    }
    void AmendmentWriter::saveToFolder(const std::string &folder, TimetableAmendment const &amendment,
                                       const input::InputModel &export_model,
                                       const std::vector<std::string> &validation_result,
                                       DetailedPenalty const *penalty) const {
        DataParser::save(export_model, folder);

        std::ofstream file;
        file.open(folder + "/Amendments.txt");

        bool sorted_by_course = true;

        if (sorted_by_course) {


            std::vector<ScheduleItemAmendment const *> all{};
            all.reserve(amendment.arrival_retimings.size() + amendment.departure_retimings.size() +
                        amendment.replatformings.size() + amendment.skipped_stops.size());
            for (auto const &retiming: amendment.arrival_retimings) {
                all.push_back(&retiming);
            }
            for (auto const &replatforming: amendment.replatformings) {
                all.push_back(&replatforming);
            }
            for (auto const &ss: amendment.skipped_stops) {
                all.push_back(&ss);
            }
            for (auto const &retiming: amendment.departure_retimings) {
                all.push_back(&retiming);
            }

            std::stable_sort(all.begin(), all.end(),
                             [](ScheduleItemAmendment const *a, ScheduleItemAmendment const *b) {
                                 return std::make_pair(a->course, a->schedule_item) <
                                        std::make_pair(b->course, b->schedule_item);
                             });

            for (auto const &a: all) {
                file << a->toTextLine(model_) << "\n";
            }

        } else {
            for (auto const &retiming: amendment.arrival_retimings) {
                file << retiming.toTextLine(model_) << "\n";
            }
            for (auto const &retiming: amendment.departure_retimings) {
                file << retiming.toTextLine(model_) << "\n";
            }
            for (auto const &replatforming: amendment.replatformings) {
                file << replatforming.toTextLine(model_) << "\n";
            }
            for (auto const &ss: amendment.skipped_stops) {
                file << ss.toTextLine(model_) << "\n";
            }
        }


        file.close();


        file.open(folder + "/Violations.txt");
        for (auto const &s: validation_result) {
            file << s << "\n";
        }


        if (penalty != nullptr) {
            file.open(folder + "/Penalty.txt");
            file << *penalty;
        }

        file.close();


        // TODO: save textual changes
    }
    input::InputModel AmendmentWriter::exportModel(const TimetableAmendment &amendment) {

        input::InputModel export_model{};
        std::vector<input::ScheduleItem> &schedule = export_model.schedule;
        std::vector<input::TrainHeader> &courses = export_model.train_headers;
        std::vector<input::RollingStockDuty> &duties = export_model.rolling_stock_duties;

        for (auto const &c: amendment.courses) {

            std::string course_id = getCourseCode(c);

            for (auto const &si: c.schedule) {
                input::ScheduleItem &isi = schedule.emplace_back();
                isi.seq = si.index + 1;
                isi.rev_seq = c.schedule.size() - si.index;

                isi.node = model_.node_metadata[si.node].code;
                isi.activity = si.original_activity;
                isi.track = model_.original_tracks_mapping[si.node][si.original_track];
                isi.arrival_seconds = si.arrival;
                isi.arrival_hhmmss = readable(si.arrival);
                isi.departure_seconds = si.departure;
                isi.departure_hhmmss = readable(si.departure);

                isi.course_id = course_id;
            }

            input::TrainHeader &th = courses.emplace_back();
            th.start_node = model_.node_metadata[c.start_node].code;
            th.end_node = model_.node_metadata[c.end_node].code;

            th.direction = c.direction;
            th.category = c.category;
            th.start_seconds = c.planned_start;
            th.end_seconds = c.planned_end;

            th.train_course_id = course_id;
        }

        for (auto const &d: amendment.duties) {

            std::string duty_id;
            if (d.index < model_.duty_code.size()) {
                duty_id = model_.course_code[d.index];
            } else {
                duty_id = "AMENDED" + std::to_string(d.index - model_.duty_code.size());
            }

            auto index = 0;
            for (auto const &de: d.elements) {

                auto &rs = duties.emplace_back();

                rs.end_node = model_.node_metadata[de.start_node].code;
                rs.start_node = model_.node_metadata[de.end_node].code;
                rs.seq = index + 1;
                rs.rev_seq = d.elements.size() - index;
                rs.start_time_seconds = de.planned_start_time;
                rs.end_time_seconds = de.planned_end_time;
                rs.start_time_hhmmss = readable(de.planned_start_time);
                rs.end_time_hhmmss = readable(de.planned_end_time);

                rs.duty_id = duty_id;
                rs.event_type = de.event;

                if (de.event == EventType::TRAIN) {
                    rs.train_course_id = getCourseCode(amendment.courses[de.course]);
                } else {
                    rs.train_course_id = "";
                }
                ++index;
            }
        }

        return export_model;
    }
    std::string AmendmentWriter::getCourseCode(const Course &c) const {
        std::string course_id;
        if (c.index < model_.course_code.size()) {
            course_id = model_.course_code[c.index];
        } else {
            course_id = "AMENDED" + std::to_string(c.index - model_.course_code.size());
        }
        return course_id;
    }
}// namespace fb
