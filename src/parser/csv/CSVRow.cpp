
#include "CSVRow.h"
#include <ranges>

namespace fb {


    CSVRow::CSVRow(std::vector<std::string> data) : data_(std::move(data)) {}

    void CSVRow::readRow(const std::string &str) {
        data_.clear();
        std::string::size_type pos = 0;
        while (pos < str.size()) {

            if (pos > 0) {
                pos++;
            }

            std::string value;
            if (str[pos] == '"') {
                ++pos;
                auto next_quote = str.find('"', pos);

                value = str.substr(pos, next_quote - pos);
                pos = next_quote + 1;// assume ", -> skip over two characters
            } else {
                auto next_comma = str.find(',', pos);
                if (next_comma == std::string::npos) {
                    next_comma = str.size();
                }

                value = str.substr(pos, next_comma - pos);
                pos = next_comma;// skip over comma
            }
            data_.emplace_back(value);
        }
    }

    std::string CSVRow::operator[](std::size_t index) const {
        return data_[index];
    }

    std::size_t CSVRow::size() const {
        return data_.size();
    }
    std::istream &operator>>(std::istream &str, CSVRow &row) {
        std::string line;
        std::getline(str, line);
        row.readRow(line);
        return str;
    }
    std::vector<std::string> CSVRow::data() {
        return data_;
    }
    std::ostream &operator<<(std::ostream &os, const CSVRow &row) {


        bool first{true};

        for (auto const &entry: row.data_) {

            if (!first) {
                os << ',';
            } else {
                first = false;
            }

            bool escape = false;
            if (entry.find(',') != std::string::npos) {
                escape = true;
            }

            if (escape) {
                os << "\"";
            }
            os << entry;

            if (escape) {
                os << "\"";
            }
        }
        return os;
    }

}// namespace fb
