#pragma once

#include "model/graph/abstractions/NodeInterface.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"

namespace fb {


    static void addNameHints(std::vector<std::string> &name_hints, const graph::NodeInterface &in,
                             const std::string &interface_name) {

        if (isValid(in.min)) {
            if (!name_hints[in.min].empty()) {
                name_hints[in.min] += " ";
            }
            name_hints[in.min] += interface_name + "-MIN";
        }

        if (isValid(in.stop)) {
            if (!name_hints[in.stop].empty()) {
                name_hints[in.stop] += " ";
            }
            name_hints[in.stop] += interface_name + "-STOP";
        }
        if (isValid(in.pass)) {
            if (!name_hints[in.pass].empty()) {
                name_hints[in.pass] += " ";
            }
            name_hints[in.pass] += interface_name + "-PASS";
        }
    }

    static void addNameHints(std::vector<std::string> &name_hints, const graph::TrainAtStationAbstraction &tas) {

        if (isValid(tas.getDepartureTimeOutNode())) {
            if (!name_hints[tas.getDepartureTimeOutNode()].empty()) {
                name_hints[tas.getDepartureTimeOutNode()] += " ";
            }
            name_hints[tas.getDepartureTimeOutNode()] += "Departure";
        }
        if (isValid(tas.getArrivalTimeOutNode())) {
            if (!name_hints[tas.getArrivalTimeOutNode()].empty()) {
                name_hints[tas.getArrivalTimeOutNode()] += " ";
            }
            name_hints[tas.getArrivalTimeOutNode()] += "Arrival";
        }
    }

}// namespace fb
