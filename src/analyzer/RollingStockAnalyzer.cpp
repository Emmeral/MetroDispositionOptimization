//
// Created by flobe on 15.03.2023.
//
#include <iostream>
#include "random.h"
#include "parser/DataParser.h"
#include "parser/sanitizer/InputSanitizer.h"

int other_main(){
    using namespace fb;

    std::string data_dir = "/home/flobe/Programming/masterthesis/implementation/ressources/data";

    auto sanitized_dir = "/home/flobe/Programming/masterthesis/implementation/ressources/sanitized";

    std::cout << "Parsing from: " << data_dir << std::endl;

    auto parser = DataParser();
    input::InputModel input_data = parser.parse(data_dir);

    auto sanitizer = InputSanitizer(input_data);

    // rolling_stock_analyses(input_data);

    sanitizer.sanitize();

    rolling_stock_analyses(input_data);
    return 0;
}
