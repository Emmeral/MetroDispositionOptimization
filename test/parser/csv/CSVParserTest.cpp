
#include "parser/csv/CSVParser.h"
#include "catch2/catch_test_macros.hpp"
#include <sstream>

namespace fb {

    std::vector<CSVRow> applyParser(std::string const &string) {
        auto stream = std::stringstream(string);
        return CSVParser::parse(stream);
    }

    TEST_CASE("CSVParser parses") {

        SECTION("simple CSV") {
            auto input = "a,b,c\n5,6,7";
            auto result = applyParser(input);

            REQUIRE(result.size() == 2);
            REQUIRE(result[0][0] == "a");
            REQUIRE(result[0][2] == "c");
            REQUIRE(result[1][0] == "5");
        }

        SECTION("with escaped commas") {

            auto input = "a,\"1,2,3\",c";
            auto result = applyParser(input);

            REQUIRE(result[0][0] == "a");
            REQUIRE(result[0][1] == "1,2,3");
            REQUIRE(result[0][2] == "c");
        }

        SECTION("with empty last element") {

            auto input = "a,b,";
            auto result = applyParser(input);

            REQUIRE(result[0][0] == "a");
            REQUIRE(result[0][1] == "b");
            REQUIRE(result[0][2].empty());
        }

        SECTION("with empty middle element") {

            auto input = "a,,c";
            auto result = applyParser(input);

            REQUIRE(result[0][0] == "a");
            REQUIRE(result[0][1].empty());
            REQUIRE(result[0][2] == "c");
        }
    }
}// namespace fb
