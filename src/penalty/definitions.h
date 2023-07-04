#pragma once


namespace fb {
    // TODO: should this be a floating point number?
    typedef unsigned int penalty_t;

    const int DELAY_THRESHOLD = 60 * 3;
    const int DELAY_COST_PER_MINUTE = 125;
    const int FREQUENCY_MISS_COST_PER_MINUTE = 150;


    const int BIGGEST_SFF = 35;
    const int SECOND_BIGGEST_SFF = 15;
    const int DEFAULT_SFF = 1;
}// namespace fb
