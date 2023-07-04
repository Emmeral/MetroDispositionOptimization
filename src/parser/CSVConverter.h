
#pragma once

#include "csv/CSVRow.h"
#include "model/enums/Enums.h"
#include "model/input/InputModel.h"


namespace fb {

    class CSVConverter {

    public:
        static input::DutyStartEnd parseDutyStartEnd(CSVRow const &row);
        static input::BaseStationValue parseBaseStationValue(CSVRow const &row);
        static input::Headway parseHeadway(CSVRow const &row);
        static input::Link parseLink(CSVRow const &row);
        static input::MinimumRuntime parseMinimumRuntime(CSVRow const &row);
        static input::Node parseNode(CSVRow const &row);
        static input::TargetFrequency parseTargetFrequency(CSVRow const &row);
        static input::RollingStockDuty parseRollingStockDuty(CSVRow const &row);
        static input::TrainHeader parseTrainHeader(CSVRow const &row);
        static input::ScheduleItem parseScheduleItem(CSVRow const &row);
        static input::FrequencyMeasure parseFrequencyMeasure(CSVRow const &row);


        static CSVRow saveDutyStartEnd(input::DutyStartEnd const &duty);
        static CSVRow saveBaseStationValue(input::BaseStationValue const &bss);
        static CSVRow saveHeadway(input::Headway const &hw);
        static CSVRow saveLink(input::Link const &link);
        static CSVRow saveMinumumRuntime(input::MinimumRuntime const &mrt);
        static CSVRow saveNode(input::Node const &node);
        static CSVRow saveTargetFrequency(input::TargetFrequency const &trgf);
        static CSVRow saveRollingStockDuty(input::RollingStockDuty const &rsd);
        static CSVRow saveTrainHeader(input::TrainHeader const &th);
        static CSVRow saveScheduleItem(input::ScheduleItem const &si);
    };

}// namespace fb
