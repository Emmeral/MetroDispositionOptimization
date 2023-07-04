
#include "NoopInitialSelectionFinder.h"
#include "model/graph/Selection.h"

namespace fb {
    InitialSelectionFinder::Result
    NoopInitialSelectionFinder::calculateInitialSelectionWithMeta(const graph::GraphWithMeta &gwm) {

        return InitialSelectionFinder::Result(graph::Selection(gwm.graph));
    }
}// namespace fb
