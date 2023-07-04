
#pragma once

#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"
#include "output/timetable/TimetableAmendment.h"
#include "penalty/DetailedPenalty.h"
#include "penalty/definitions.h"
namespace fb {

    class AmendmentPenaltyComputer {
    public:
        AmendmentPenaltyComputer(const Model &model, const ProblemInstance &problem);

        DetailedPenalty computeAmendmentPenalty(TimetableAmendment const &amendment, bool include_realized) const;

        inline penalty_t computeAmendmentPenaltyTotal(TimetableAmendment const &amendment,
                                                      bool include_realized) const {
            return computeAmendmentPenalty(amendment, include_realized).total();
        };


    private:
        penalty_t computeDelayPenalty(TimetableAmendment const &amendment, bool include_realized) const;
        penalty_t computeSkippedStopPenalty(TimetableAmendment const &amendment, bool include_realized) const;
        penalty_t computeFrequencyPenalty(TimetableAmendment const &amendment, bool include_realized) const;


        Model const &model_;
        ProblemInstance const &problem_;
    };

}// namespace fb
