#pragma once


#include <array>
#include <cmath>
#include <map>
#include <sstream>
#include <string>
#include <vector>
namespace fb {


    template<typename T>
    std::string toJson(T const &t);

    template<typename T>
        requires std::is_integral_v<T>
    std::string toJson(T const &t) {
        return std::to_string(t);
    }

    template<typename T>
        requires std::is_floating_point_v<T>
    std::string toJson(T const &t) {
        if (std::isnan(t)) {
            return "null";
        }
        return std::to_string(t);
    }

    // for vector
    template<typename E>
    std::string toJson(std::vector<E> const &t) {
        std::string result = "[";
        for (auto const &e: t) {
            result += toJson(e) + ",";
        }
        if (result.size() > 1) {
            result.pop_back();
        }
        result += "]";
        return result;
    }
    // for array
    template<typename E, size_t N>
    std::string toJson(std::array<E, N> const &t) {
        std::string result = "[";
        for (auto const &e: t) {
            result += toJson(e) + ",";
        }
        if (result.size() > 1) {
            result.pop_back();
        }
        result += "]";
        return result;
    }
    template<typename E>
    std::string toJson(std::map<std::string, E> const &map) {
        std::string result = "{";
        for (auto const &e: map) {
            result += "\"" + e.first + "\":" + toJson(e.second) + ",";
        }
        if (result.size() > 1) {
            result.pop_back();
        }
        result += "}";
        return result;
    }

    template<>
    std::string toJson(std::string const &t);


    template<>
    std::string toJson(bool const &t);


}// namespace fb
