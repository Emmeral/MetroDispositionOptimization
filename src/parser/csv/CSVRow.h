
#pragma once


#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace fb {

    class CSVRow {
    public:
        explicit CSVRow(std::vector<std::string> data);
        explicit CSVRow() = default;

        std::string operator[](std::size_t index) const;

        std::size_t size() const;

        std::vector<std::string> data();

        void readRow(const std::string &str);

        friend std::istream &operator>>(std::istream &str, CSVRow &row);

        friend std::ostream &operator<<(std::ostream &os, const CSVRow &row);

    private:
        std::vector<std::string> data_{};
    };
}// namespace fb
