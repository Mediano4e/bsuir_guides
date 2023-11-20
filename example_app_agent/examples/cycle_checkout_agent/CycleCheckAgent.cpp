#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>

#include "CycleCheckAgent.hpp"

using namespace std;
using namespace utils;

namespace exampleModule
{

SC_AGENT_IMPLEMENTATION(CycleCheckAgent)
{
    ScAddr actionNode = otherAddr;
    ScAddr inputGraph = IteratorUtils::getAnyFromSet(ms_context.get(), actionNode);

    if (!inputGraph.IsValid())
    {
        SC_LOG_ERROR("CycleCheckAgent:  Invalid argument");
        utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
        return SC_RESULT_ERROR_INVALID_PARAMS;
    }

    ScAddrVector answerElements;

    try
    {
        SC_LOG_ERROR("Trying to form response construction");
        formResultConstruction(answerElements, inputGraph);
    }
    catch (exception & exc)
    {
        SC_LOG_ERROR("CycleCheckAgent:  " << exc.what());
        utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
        return SC_RESULT_ERROR;
    }

    utils::AgentUtils::finishAgentWork(ms_context.get(), actionNode, answerElements, true);
    return SC_RESULT_OK;
}

void CycleCheckAgent::clearPreviousResults(ScAddr const & inputGraph) {
    ScIterator3Ptr it3 = m_memoryCtx.Iterator3(
            Keynodes::cycle_graph_set,
            ScType::Unknown,
            inputGraph);

    while (it3->Next())
        m_memoryCtx.EraseElement(it3->Get(1));
}

void CycleCheckAgent::formResultConstruction(ScAddrVector & answerElements, const ScAddr & inputGraph) {
    clearPreviousResults(inputGraph);

    ScAddr checkoutResult;
    if ( getCheckoutResult(inputGraph) )
        checkoutResult = m_memoryCtx.CreateEdge(ScType::EdgeAccessConstNegPerm, Keynodes::cycle_graph_set, inputGraph);
    else
        checkoutResult = m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm, Keynodes::cycle_graph_set, inputGraph);

    answerElements.push_back(Keynodes::cycle_graph_set);
    answerElements.push_back(checkoutResult);
}

bool CycleCheckAgent::getCheckoutResult(const ScAddr & inputGraph) {
    SC_LOG_ERROR("Checkout starts");
    VisitedNodeVector graphNodes;
    ScAddrVector noorEdges;

    SC_LOG_ERROR("Forming vector of vertices");
    ScIterator3Ptr it3 = m_memoryCtx.Iterator3(
            inputGraph,
            ScType::EdgeAccessConstPosPerm,
            ScType::NodeConst);
    while ( it3->Next() )
        graphNodes.push_back( {it3->Get(2), 0} );
    SC_LOG_ERROR("Vertex amount:");
    SC_LOG_ERROR(graphNodes.size());

    for (auto & node : graphNodes) {
        SC_LOG_ERROR("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        SC_LOG_ERROR("Looping checkout of node=");
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(node.addr));

        if (node.cond != 0) {
            SC_LOG_ERROR("Vertex checked");
            continue;
        }

        if ( !cycleCheckout(graphNodes, noorEdges, node, inputGraph) ) {
            SC_LOG_ERROR("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
            return false;
        }
        SC_LOG_ERROR("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    }

    return true;
}

bool CycleCheckAgent::cycleCheckout(VisitedNodeVector & nodes, ScAddrVector & noorEdges,
                                    VisitedNode & currentNode, ScAddr const & graph)
{
    SC_LOG_ERROR("start-----------------------------------------------------------------------");
    SC_LOG_ERROR("Dfs checkout of node ");
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(currentNode.addr));
    SC_LOG_ERROR("Node condition");
    SC_LOG_ERROR(currentNode.cond);

    if (currentNode.cond == 2) {
        SC_LOG_ERROR("Checked node");
        return true;
    }
    if (currentNode.cond == 1) {
        SC_LOG_ERROR("Not finished checkout found");
        return false;
    }

    currentNode.cond = 1;

    ScIterator5Ptr it5np = m_memoryCtx.Iterator5(
            currentNode.addr,
            ScType::EdgeUCommonConst,
            ScType::NodeConst,
            ScType::EdgeAccessConstPosPerm,
            graph);

    while ( it5np->Next() ) {
        SC_LOG_ERROR("Parent noor iterator");
        if ( find( noorEdges.begin(), noorEdges.end(), it5np->Get(1) ) != noorEdges.end() ) {
            SC_LOG_ERROR("Noor edge already visited");
            continue;
        }

        noorEdges.push_back( it5np->Get(1) );
        SC_LOG_ERROR("Size of vector of noor edges");
        SC_LOG_ERROR(noorEdges.size());

        ssize_t ind = searchNodeByAddr( nodes, it5np->Get(2) );
        if (ind == -1) {
            SC_LOG_ERROR("Vertex not from our graph");
            continue;
        }

        if ( !cycleCheckout(nodes, noorEdges, nodes[ind], graph) ) {
            SC_LOG_ERROR("false-----------------------------------------------------------------------");
            return false;
        }
    }

    ScIterator5Ptr it5nc = m_memoryCtx.Iterator5(
            ScType::NodeConst,
            ScType::EdgeUCommonConst,
            currentNode.addr,
            ScType::EdgeAccessConstPosPerm,
            graph);

    while ( it5nc->Next() ) {
        SC_LOG_ERROR("Child noor iterator");
        if ( find( noorEdges.begin(), noorEdges.end(), it5nc->Get(1) ) != noorEdges.end() ) {
            SC_LOG_ERROR("Noor edge already visited");
            continue;
        }

        noorEdges.push_back( it5nc->Get(1) );
        SC_LOG_ERROR("Size of vector of noor edges");
        SC_LOG_ERROR(noorEdges.size());

        ssize_t ind = searchNodeByAddr( nodes, it5nc->Get(0) );
        if (ind == -1) {
            SC_LOG_ERROR("Vertex not from our graph");
            continue;
        }

        if ( !cycleCheckout(nodes, noorEdges, nodes[ind], graph) ) {
            SC_LOG_ERROR("false-----------------------------------------------------------------------");
            return false;
        }
    }

    ScIterator5Ptr it5 = m_memoryCtx.Iterator5(
            currentNode.addr,
            ScType::Unknown,
            ScType::NodeConst,
            ScType::EdgeAccessConstPosPerm,
            graph);

    while ( it5->Next() ) {
        SC_LOG_ERROR("Unknown iterator");
        if ( find( noorEdges.begin(), noorEdges.end(), it5->Get(1) ) != noorEdges.end() ) {
            SC_LOG_ERROR("Noor edge already visited");
            continue;
        }

        ssize_t ind = searchNodeByAddr( nodes, it5->Get(2) );
        if (ind == -1) {
            SC_LOG_ERROR("Vertex not from our graph");
            continue;
        }

        if ( !cycleCheckout(nodes, noorEdges, nodes[ind], graph) ) {
            SC_LOG_ERROR("false-----------------------------------------------------------------------");
            return false;
        }
    }

    currentNode.cond = 2;
    SC_LOG_ERROR("true-----------------------------------------------------------------------");
    return true;
}

ssize_t CycleCheckAgent::searchNodeByAddr(VisitedNodeVector const & vector, ScAddr const & addr) {
    for (ssize_t i = 0; i < vector.size(); i++)
        if (vector[i].addr == addr)
            return i;

    return -1;
}

}
