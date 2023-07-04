
#include "parser/sanitizer/InputSanitizer.h"
#include "catch2/catch_test_macros.hpp"
#include "model/enums/Enums.h"
#include "model/input/InputModel.h"

#include <vector>

namespace fb {


    TEST_CASE("Input Sanitizer inserts missing tracks") {

        auto model = input::InputModel();


        model.nodes = std::vector{2, input::Node{}};

        auto &n0 = model.nodes[0];
        n0.code = "n0";
        n0.wb_tracks = {"a"};
        n0.eb_tracks = {"b"};


        auto &n1 = model.nodes[1];
        n1.code = "n1";
        n1.wb_tracks = {"a"};
        n1.eb_tracks = {"b"};

        model.schedule = std::vector{2, input::ScheduleItem{}};

        auto &s0 = model.schedule[0];
        s0.node = "n0";
        s0.course_id = "c0";
        s0.seq = 1;
        s0.rev_seq = 1;
        s0.track = "c";

        auto &s1 = model.schedule[1];
        s1.node = "n1";
        s1.course_id = "c1";
        s1.seq = 1;
        s1.rev_seq = 1;
        s1.track = "c";


        model.train_headers = std::vector{2, input::TrainHeader{}};
        auto &c0 = model.train_headers[0];
        c0.direction = Direction::WB;
        c0.train_course_id = "c0";

        auto &c1 = model.train_headers[1];
        c1.direction = Direction::EB;
        c1.train_course_id = "c1";

        std::map<std::string, input::Node *> map = {{"n0", &model.nodes[0]}, {"n1", &model.nodes[1]}};

        auto sanitizer = InputSanitizer(model);
        sanitizer.expandNodeTracks();


        std::vector<std::string> expected = {"a", "c"};
        REQUIRE(model.nodes[0].wb_tracks == expected);

        expected = {"b", "c"};
        REQUIRE(model.nodes[1].eb_tracks == expected);


        REQUIRE(sanitizer.statistics().inserted_tracks == 2);
    }

    TEST_CASE("Input Sanitizer corrects Minimum Runtimes") {

        auto model = input::InputModel();
        model.minimum_runtimes = std::vector(4, input::MinimumRuntime());

        for (auto &rt: model.minimum_runtimes) {
            rt.link_start_node = "Node1";
            rt.link_end_node = "Node2";
        }
        model.minimum_runtimes[0].start_activity = Activity::PASS;
        model.minimum_runtimes[0].end_activity = Activity::PASS;
        model.minimum_runtimes[0].minimum_runtime = 60;

        model.minimum_runtimes[1].start_activity = Activity::PASS;
        model.minimum_runtimes[1].end_activity = Activity::STOP;
        model.minimum_runtimes[1].minimum_runtime = 70;

        model.minimum_runtimes[2].start_activity = Activity::STOP;
        model.minimum_runtimes[2].end_activity = Activity::PASS;
        model.minimum_runtimes[2].minimum_runtime = 70;

        model.minimum_runtimes[3].start_activity = Activity::STOP;
        model.minimum_runtimes[3].end_activity = Activity::STOP;
        model.minimum_runtimes[3].minimum_runtime = 80;

        SECTION(" with single change from schedule") {
            model.schedule = std::vector(2, input::ScheduleItem());

            auto &si0 = model.schedule[0];
            si0.activity = Activity::STOP;
            si0.node = "Node1";
            si0.course_id = "Course1";
            si0.seq = 1;
            si0.rev_seq = 2;
            si0.departure_seconds = 1000;

            auto &si1 = model.schedule[1];
            si1.activity = Activity::PASS;
            si1.node = "Node2";
            si1.course_id = "Course1";
            si1.seq = 2;
            si1.rev_seq = 1;
            si1.arrival_seconds = 1065;// -> have a time of 65 < 70

            auto sanitizer = InputSanitizer(model);
            sanitizer.correctMinimalRuntimes();


            REQUIRE(model.minimum_runtimes[0].minimum_runtime == 65 - 20);
            REQUIRE(model.minimum_runtimes[1].minimum_runtime == 65);
            REQUIRE(model.minimum_runtimes[2].minimum_runtime == 65);
            REQUIRE(model.minimum_runtimes[3].minimum_runtime == 65 + 20);

            REQUIRE(sanitizer.statistics().changed_runtimes == 4);
        }

        SECTION("with multiple changes from schedule") {
            model.schedule = std::vector(4, input::ScheduleItem());

            auto &si0 = model.schedule[0];
            si0.activity = Activity::STOP;
            si0.node = "Node1";
            si0.course_id = "Course1";
            si0.seq = 1;
            si0.rev_seq = 2;
            si0.departure_seconds = 1000;

            auto &si1 = model.schedule[1];
            si1.activity = Activity::PASS;
            si1.node = "Node2";
            si1.course_id = "Course1";
            si1.seq = 2;
            si1.rev_seq = 1;
            si1.arrival_seconds = 1065;// -> have a time of 65 < 70


            auto &si2 = model.schedule[2];
            si2.activity = Activity::PASS;
            si2.node = "Node1";
            si2.course_id = "Course2";
            si2.seq = 1;
            si2.rev_seq = 2;
            si2.departure_seconds = 2000;

            auto &si3 = model.schedule[3];
            si3.activity = Activity::STOP;
            si3.node = "Node2";
            si3.course_id = "Course2";
            si3.seq = 2;
            si3.rev_seq = 1;
            si3.arrival_seconds = 2055;// -> have a time of 55 < 70

            auto sanitizer = InputSanitizer(model);
            sanitizer.correctMinimalRuntimes();


            REQUIRE(model.minimum_runtimes[0].minimum_runtime == 55 - 20);
            REQUIRE(model.minimum_runtimes[1].minimum_runtime == 55);
            REQUIRE(model.minimum_runtimes[2].minimum_runtime == 65);
            REQUIRE(model.minimum_runtimes[3].minimum_runtime == 65 + 20);

            REQUIRE(sanitizer.statistics().changed_runtimes == 4);
        }
    }


    TEST_CASE("Input Sanitizer corrects headways ") {

        auto model = input::InputModel();

        model.headways = std::vector(1, input::Headway());

        auto &hw0 = model.headways[0];
        hw0.link_start_node = "Node1";
        hw0.link_end_node = "Node2";
        hw0.start_activity_train_front = Activity::STOP;
        hw0.end_activity_train_front = Activity::STOP;
        hw0.start_activity_train_behind = Activity::STOP;
        hw0.end_activity_train_behind = Activity::STOP;
        hw0.minimum_headway_seconds = 100;

        model.schedule = std::vector(4, input::ScheduleItem());

        auto &si0 = model.schedule[0];
        si0.activity = Activity::STOP;
        si0.node = "Node1";
        si0.course_id = "Course1";
        si0.seq = 1;
        si0.rev_seq = 2;
        si0.departure_seconds = 1000;

        auto &si1 = model.schedule[1];
        si1.activity = Activity::STOP;
        si1.node = "Node2";
        si1.course_id = "Course1";
        si1.seq = 2;
        si1.rev_seq = 1;


        auto &si2 = model.schedule[2];
        si2.activity = Activity::STOP;
        si2.node = "Node1";
        si2.course_id = "Course2";
        si2.seq = 1;
        si2.rev_seq = 2;
        si2.departure_seconds = 1080;// 80 < 100

        auto &si3 = model.schedule[3];
        si3.activity = Activity::STOP;
        si3.node = "Node2";
        si3.course_id = "Course2";
        si3.seq = 2;
        si3.rev_seq = 1;


        SECTION(" normal") {
            auto sanitizer = InputSanitizer(model);
            sanitizer.correctMinimalHeadways();

            REQUIRE(model.headways[0].minimum_headway_seconds == 80);
            REQUIRE(sanitizer.statistics().changed_headways == 1);
        }

        SECTION(" but respects min headways of 30s") {
            si2.departure_seconds = 1001;

            auto sanitizer = InputSanitizer(model);
            sanitizer.correctMinimalHeadways();

            REQUIRE(model.headways[0].minimum_headway_seconds == 30);
            REQUIRE(sanitizer.statistics().changed_headways == 1);
        }
    }

}// namespace fb
