
#pragma once

#include "CSVRow.h"
#include "model/input/InputModel.h"
#include <vector>

namespace fb {

    class CSVParser {

    public:
        static std::vector<CSVRow> parse(std::istream &str);
    };

}// namespace fb
