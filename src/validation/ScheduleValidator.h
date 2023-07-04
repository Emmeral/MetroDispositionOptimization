
#pragma once
#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"
#include "output/timetable/TimetableAmendment.h"

namespace fb {

    class ScheduleValidator {

    public:
        ScheduleValidator(const Model &model, const ProblemInstance &problem);

    public:
        std::vector<std::string> validate(TimetableAmendment const &amendment);

    private:
        Model const &model_;
        ProblemInstance const &problem_;

        struct PairIndex {
            course_index_t course;
            index_t first;
            index_t second;
        };
        std::vector<std::string> validateUsedInfrastructure(TimetableAmendment const &amendment);
        std::vector<std::string> validateRespectRealizedSchedule(TimetableAmendment const &amendment);
        std::vector<std::string> validateDwellTimes(TimetableAmendment const &amendment);
        std::vector<std::string> validateRuntimes(TimetableAmendment const &amendment);
        std::vector<std::string> validateLineHeadways(TimetableAmendment const &amendment);
        std::vector<std::string> validateTrackHeadways(TimetableAmendment const &amendment);
        std::vector<std::string> validateEarlyDepartureBound(TimetableAmendment const &amendment);


        std::vector<std::string> validateLateDepartures(TimetableAmendment const &amendment);
        std::vector<std::string> validateDutyElementConsistency(TimetableAmendment const &amendment);

        std::vector<std::string> validateChangeEndTimes(TimetableAmendment const &amendment);
        seconds_t realArrival(ScheduleItem const &item, Course const &course,
                              const TimetableAmendment &amendment) const;

        seconds_t realDeparture(ScheduleItem const &item, Course const &course,
                                const TimetableAmendment &amendment) const;
        void validateLinkHeadwaysSameDir(const TimetableAmendment &amendment, std::vector<std::string> &violations,
                                         const Link &link, const std::vector<PairIndex> &sis);
        void validateLinkHeadwaysBothDir(const TimetableAmendment &amendment, std::vector<std::string> &violations,
                                         const Link &link, const Link &rev_link, const std::vector<PairIndex> &link_sis,
                                         const std::vector<PairIndex> &rev_link_sis);
    };

}// namespace fb
