
#pragma once

#include "TimetableAmendment.h"
#include "model/input/InputModel.h"
#include "penalty/DetailedPenalty.h"
#include <string>
namespace fb {

    class AmendmentWriter {
    public:
        explicit AmendmentWriter(const Model &model);

    public:
        void writeAmendment(const TimetableAmendment &amendment, const std::string &folder,
                            const std::vector<std::string> &validation_result = {},
                            DetailedPenalty const *penalty = nullptr);

    private:
        Model const &model_;


        fb::input::InputModel exportModel(const TimetableAmendment &amendment);
        void saveToFolder(const std::string &folder, const TimetableAmendment &amendment,
                          const input::InputModel &export_model, const std::vector<std::string> &validation_result = {},
                          DetailedPenalty const *penalty = nullptr) const;

        std::string toTextLine(const ScheduleItemAmendment &amendment, const std::string &name) const;
        std::string toTextLine(const ArrivalReTiming &re_timing) const;
        std::string toTextLine(const DepartureReTiming &re_timing) const;
        std::string toTextLine(const RePlatforming &re_platforming) const;
        std::string toTextLine(const SkippedStop &skipped_stop) const;


        std::string getCourseCode(const Course &c) const;
    };

}// namespace fb
