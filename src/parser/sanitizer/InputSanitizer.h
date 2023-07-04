
#pragma once

#include "model/input/InputModel.h"
#include <map>
namespace fb {

    class InputSanitizer {

    public:
        struct Statistics {
            uint16_t inserted_tracks{0};
            uint32_t changed_runtimes{0};
            uint32_t changed_headways{0};
        };
        explicit InputSanitizer(input::InputModel &input);


        void sanitize();
        void expandNodeTracks();
        void overrideWrongIndividualData();

        void correctMinimalRuntimes();

        void correctMinimalHeadways();


        Statistics const &statistics() { return statistics_; }

    private:
        Statistics statistics_{};
        input::InputModel &input_;
        std::map<input::id_t, input::Node *> id_to_node_map_;
        std::map<input::id_t, std::vector<const input::ScheduleItem *>> schedule_map_;
    };

}// namespace fb
