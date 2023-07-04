
#include "CSVParser.h"

namespace fb {


    std::vector<CSVRow> CSVParser::parse(std::istream &str) {

        std::vector<CSVRow> vector{};
        bool empty = true;

        CSVRow row{};
        str >> row;
        while (row.size() > 0) {
            vector.emplace_back(row);
            row = CSVRow{};
            str >> row;
        }
        return vector;
    }

}// namespace fb
