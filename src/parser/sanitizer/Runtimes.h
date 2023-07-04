
#pragma once

#include "model/definitions.h"
#include "model/enums/Enums.h"
#include "model/input/InputModel.h"

namespace fb {

    struct Runtimes {
        Runtimes(input::id_t startNode, input::id_t endNode)
            : start_node(std::move(startNode)), end_node(std::move(endNode)) {}

        input::id_t start_node;
        input::id_t end_node;

        uint32_t pass_pass{};
        uint32_t pass_stop{};
        uint32_t stop_pass{};
        uint32_t stop_stop{};

        bool fs_pass_pass = false;
        bool fs_pass_stop = false;
        bool fs_stop_pass = false;
        bool fs_stop_stop = false;


        /**
         * Sets the runtime. If the value comes from the schedule only overrides it if there is an improvement.
         * @param a1 activity at start node
         * @param a2 activity at end node
         * @param time the time to set
         * @param from_sched whether to note this information an override only if there is an improvement
         */
        void set(Activity a1, Activity a2, uint32_t time, bool from_sched = false);


        bool fromSchedule(Activity a1, Activity a2) const;

        int fromScheduleCount() const;

        uint32_t &get(Activity const &a1, Activity const &a2);
        uint32_t const &get(Activity const &a1, Activity const &a2) const;

        bool &isFromSchedule(Activity const &a1, Activity const &a2);
        bool const &isFromSchedule(Activity const &a1, Activity const &a2) const;
    };

}// namespace fb
