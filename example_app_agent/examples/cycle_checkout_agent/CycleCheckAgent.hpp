#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "CycleCheckAgent.generated.hpp"

namespace exampleModule {

    struct VisitedNode {
        ScAddr addr;
        int cond;
    };
    using VisitedNodeVector = std::vector<VisitedNode>;

    class CycleCheckAgent : public ScAgent {
        SC_CLASS(Agent, Event(Keynodes::question_check_cycles, ScEvent::Type::AddOutputEdge))
        SC_GENERATED_BODY()

        void formResultConstruction(ScAddrVector & answerElements, ScAddr const & inputGraph);
        void clearPreviousResults(ScAddr const & inputGraph);
        bool getCheckoutResult(ScAddr const & inputGraph);
        bool cycleCheckout(VisitedNodeVector & nodes, ScAddrVector & noorEdges,
                           VisitedNode & currentNode, ScAddr const & graph);    //dfs
        static ssize_t searchNodeByAddr(VisitedNodeVector const & vector, ScAddr const & addr);
    };

}