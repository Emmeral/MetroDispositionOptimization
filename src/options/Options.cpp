
#include "Options.h"
#include <sstream>

namespace fb {


    std::ostream &operator<<(std::ostream &stm, const Options &opt) {
        stm << "General Options:\n";
        stm << "  -Model Folder  : " << opt.model << "\n";
        stm << "  -Problem Folder: " << opt.problem << "\n";
        stm << "  -Sanitize      : " << opt.sanitize << "\n";
        stm << "  -Validate      : " << opt.validate << "\n";
        stm << "  -Mode          : " << opt.mode << "\n";

        stm << opt.graph;
        stm << opt.bab;

        return stm;
    }
    std::istream &operator>>(std::istream &in, Options::Mode &store) {

        std::string token;
        in >> token;

        if (token == "Iterated") {
            store = Options::Mode::Iterated;
        } else if (token == "Single") {
            store = Options::Mode::Single;
        } else if (token == "FCFS") {
            store = Options::Mode::FCFS;
        } else {
            in.setstate(std::ios_base::failbit);
        }
        return in;
    }
    std::ostream &operator<<(std::ostream &stm, const Options::Mode &store) {
        switch (store) {
            case Options::Mode::Iterated:
                stm << "Iterated";
                break;
            case Options::Mode::Single:
                stm << "Single";
                break;
            case Options::Mode::FCFS:
                stm << "FCFS";
                break;
            default:
                stm.setstate(std::ios_base::failbit);
        }
        return stm;
    }
    template<>
    std::string toJson(const Options::Mode &t) {
        std::ostringstream ss{};
        ss << t;
        return toJson(ss.str());
    }

    template<>
    std::string toJson(const Options &t) {

        std::string str = "{";
        str += "\"model\":" + toJson(t.model) + ",";
        str += "\"problem\":" + toJson(t.problem) + ",";
        str += "\"solution\":" + toJson(t.solution) + ",";
        str += "\"name\":" + toJson(t.name) + ",";
        str += "\"sanitize\":" + toJson(t.sanitize) + ",";
        str += "\"validate\":" + toJson(t.validate) + ",";
        str += "\"batch\":" + toJson(t.batch) + ",";
        str += "\"mode\":" + toJson(t.mode) + ",";
        str += "\"graph\":" + toJson(t.graph) + ",";
        str += "\"bab\":" + toJson(t.bab);
        str += "}";
        return str;
    }
}// namespace fb
