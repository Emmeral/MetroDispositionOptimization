
#include "Runtimes.h"
#include "model/definitions.h"
#include "model/enums/Enums.h"

namespace fb {
    void Runtimes::set(Activity a1, Activity a2, uint32_t time, bool from_sched) {


        auto &old_val = get(a1, a2);

        if (!from_sched || old_val > time) {
            old_val = time;
            isFromSchedule(a1, a2) = from_sched;
        }
    }

    int Runtimes::fromScheduleCount() const { return fs_pass_pass + fs_pass_stop + fs_stop_pass + fs_stop_stop; }
    uint32_t &Runtimes::get(const Activity &a1, const Activity &a2) {
        if (a1 == Activity::PASS) {
            if (a2 == Activity::PASS) {
                return pass_pass;
            } else {
                return pass_stop;
            }
        } else {
            if (a2 == Activity::PASS) {
                return stop_pass;
            } else {
                return stop_stop;
            }
        }
    }
    bool &Runtimes::isFromSchedule(const Activity &a1, const Activity &a2) {
        if (a1 == Activity::PASS) {
            if (a2 == Activity::PASS) {
                return fs_pass_pass;
            } else {
                return fs_pass_stop;
            }
        } else {
            if (a2 == Activity::PASS) {
                return fs_stop_pass;
            } else {
                return fs_stop_stop;
            }
        }
    }
    uint32_t const &Runtimes::get(const Activity &a1, const Activity &a2) const {
        if (a1 == Activity::PASS) {
            if (a2 == Activity::PASS) {
                return pass_pass;
            } else {
                return pass_stop;
            }
        } else {
            if (a2 == Activity::PASS) {
                return stop_pass;
            } else {
                return stop_stop;
            }
        }
    }
    bool const &Runtimes::isFromSchedule(const Activity &a1, const Activity &a2) const {
        if (a1 == Activity::PASS) {
            if (a2 == Activity::PASS) {
                return fs_pass_pass;
            } else {
                return fs_pass_stop;
            }
        } else {
            if (a2 == Activity::PASS) {
                return fs_stop_pass;
            } else {
                return fs_stop_stop;
            }
        }
    }
}// namespace fb
