
#pragma once

#include "InitialSelectionFinder.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/Selection.h"

namespace fb {

    class NoopInitialSelectionFinder : public InitialSelectionFinder {

    public:
        Result calculateInitialSelectionWithMeta(const graph::GraphWithMeta &gwm) override;
    };

}// namespace fb
