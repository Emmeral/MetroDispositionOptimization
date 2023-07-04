
#pragma once


#include "csv/CSVRow.h"
#include "model/enums/Enums.h"
#include "model/input/InputModel.h"
#include <fstream>
#include <functional>
#include <unordered_map>

namespace fb {


    class DataParser {


    public:
        static input::InputModel parse(std::string directory);

        static void save(input::InputModel const &model, const std::string &directory);

    private:
        static void constructFromCsv(std::vector<CSVRow> const &rows, const std::string &filename,
                                     input::InputModel &input_model);

        template<typename T, typename CONVERTER>
        static void saveSingleFile(std::string const &path, CSVRow const &header, std::vector<T> const data,
                                   CONVERTER converter, bool skipOnEmpty = true) {

            if (data.empty() && skipOnEmpty) {
                return;
            }
            std::ofstream file;
            file.open(path);
            file << header << "\n";
            for (auto const &item: data) {
                auto const row = converter(item);
                file << row << "\n";
            }
            file.close();
        }
    };
}// namespace fb
