
#pragma once

#include "model/problems/ProblemInstance.h"
#include "parser/csv/CSVRow.h"
namespace fb {

    class ProblemParser {

    public:
        explicit ProblemParser(const Model &model);
        ProblemInstance parse(std::string const &directory);

    private:
        void constructFromCsv(std::vector<CSVRow> const &rows, const std::string &filename, ProblemInstance &instance);


        ExtendedRunTime parseExtendedRunTime(CSVRow const &row);
        LateDeparture parseLateDeparture(CSVRow const &row);
        ExtendedDwellTime parseExtendedDwellTime(CSVRow const &row);
        TrainExtendedDwell parseExtendedTrainDwell(CSVRow const &row);

        void parseAndInsertScheduleItem(CSVRow const &row, ProblemInstance &instance);

        Model const &model_;
    };

}// namespace fb
