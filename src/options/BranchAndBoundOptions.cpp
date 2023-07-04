
#include "BranchAndBoundOptions.h"
#include <iostream>
namespace fb {

    std::ostream &operator<<(std::ostream &stm, const BranchAndBoundOptions &opt) {

        stm << "BranchAndBoundOptions:\n";
        stm << "  -Initial Selection  : " << opt.initial_selection << "\n";
        stm << "  -Timeout            : " << opt.timeout << "\n";
        stm << "  -Skipped Stop Impact: " << opt.skipped_stop_impact_factor << "\n";
        stm << "  -Relation Impact Fct: " << opt.relation_impact_factor << "\n";
        stm << "  -State Config       : " << opt.state_config << "\n";
        stm << "  -Store              : " << opt.store << "\n";
        stm << "  -Store Max Load     : " << opt.store_max_load << "\n";
        stm << "  -BOL Threshold      : " << opt.bol_threshold << "\n";
        stm << "  -DistanceToEndNorm  : " << opt.distance_to_end_normalization << "\n";
        return stm;
    }

    std::istream &operator>>(std::istream &in, BranchAndBoundOptions::StoreOption &store) {
        std::string token;
        in >> token;

        if (token == "LIFO") {
            store = BranchAndBoundOptions::StoreOption::LIFO;
        } else if (token == "BOL") {
            store = BranchAndBoundOptions::StoreOption::BOL;
        } else if (token == "PO") {
            store = BranchAndBoundOptions::StoreOption::PO;
        } else {
            in.setstate(std::ios_base::failbit);
        }
        return in;
    }
    std::ostream &operator<<(std::ostream &stm, BranchAndBoundOptions::StoreOption const &store) {

        switch (store) {
            case BranchAndBoundOptions::StoreOption::BOL:
                stm << "BOL";
                break;
            case BranchAndBoundOptions::StoreOption::LIFO:
                stm << "LIFO";
                break;
            case BranchAndBoundOptions::StoreOption::PO:
                stm << "PO";
                break;
            default:
                stm.setstate(std::ios_base::failbit);
        }
        return stm;
    }
    std::istream &operator>>(std::istream &in, BranchAndBoundOptions::InitialSelection &initial_selection) {
        std::string token;
        in >> token;

        if (token == "ORIGINAL") {
            initial_selection = BranchAndBoundOptions::InitialSelection::ORIGINAL;
        } else if (token == "FCFS") {
            initial_selection = BranchAndBoundOptions::InitialSelection::FCFS;
        } else if (token == "SIMPLE_FCFS") {
            initial_selection = BranchAndBoundOptions::InitialSelection::SIMPLE_FCFS;
        } else if (token == "NOOP") {
            initial_selection = BranchAndBoundOptions::InitialSelection::NOOP;
        } else {
            in.setstate(std::ios_base::failbit);
        }
        return in;
    }
    std::ostream &operator<<(std::ostream &stm, const BranchAndBoundOptions::InitialSelection &initial_selection) {
        switch (initial_selection) {
            case BranchAndBoundOptions::InitialSelection::ORIGINAL:
                stm << "ORIGINAL";
                break;
            case BranchAndBoundOptions::InitialSelection::FCFS:
                stm << "FCFS";
                break;
            case BranchAndBoundOptions::InitialSelection::SIMPLE_FCFS:
                stm << "SIMPLE_FCFS";
                break;
            case BranchAndBoundOptions::InitialSelection::NOOP:
                stm << "NOOP";
                break;
            default:
                stm.setstate(std::ios_base::failbit);
        }
        return stm;
    }
    template<>
    std::string fb::toJson(const BranchAndBoundOptions::InitialSelection &t) {
        std::ostringstream ss;
        ss << t;
        return toJson(ss.str());
    }
    template<>
    std::string fb::toJson(const BranchAndBoundOptions::StoreOption &t) {
        std::ostringstream ss;
        ss << t;
        return toJson(ss.str());
    }
    template<>
    std::string toJson(const BranchAndBoundOptions &t) {
        std::string str = "{";
        str += "\"initial_selection\":" + toJson(t.initial_selection) + ",";
        str += "\"timeout\":" + toJson(t.timeout) + ",";
        str += "\"skipped_stop_impact_factor\":" + toJson(t.skipped_stop_impact_factor) + ",";
        str += "\"relation_impact_factor\":" + toJson(t.relation_impact_factor) + ",";
        str += "\"state_config\":" + toJson(t.state_config) + ",";
        str += "\"store\":" + toJson(t.store) + ",";
        str += "\"store_max_load\":" + toJson(t.store_max_load) + ",";
        str += "\"bol_threshold\":" + toJson(t.bol_threshold);
        str += "\"distance_to_end_normalization\":" + toJson(t.distance_to_end_normalization);
        str += "}";
        return str;
    }
}// namespace fb
