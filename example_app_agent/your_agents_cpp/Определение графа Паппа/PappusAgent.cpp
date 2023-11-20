#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>

#include "PappusAgent.hpp"

using namespace std;
using namespace utils;

namespace exampleModule
{

SC_AGENT_IMPLEMENTATION(PappusAgent)
{
  ScAddr actionNode = otherAddr;
  ScAddr inputGraphStruct = IteratorUtils::getAnyFromSet(ms_context.get(), actionNode);
  
  if (!inputGraphStruct.IsValid())
  {
    SC_LOG_ERROR("None members of set");
    utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }
  
  ScAddrVector answerElements;
  
  bool response = pappusCheckout(inputGraphStruct);
  formResponseConstruction(inputGraphStruct, answerElements, response);
  
  utils::AgentUtils::finishAgentWork(ms_context.get(), actionNode, answerElements, true);
  SC_LOG_DEBUG("My agent's done everything he wanted and even successfully");
  
  return SC_RESULT_OK;
}

bool PappusAgent::pappusCheckout(ScAddr const & inputGraphStruct)
{
  ColorVerticesVector verticesOfGraph;
  
  if (!verticesAmountCheckout(verticesOfGraph, inputGraphStruct))
    return false;

  SC_LOG_ERROR("After size");

  if (!vertexDegreeCheckout(inputGraphStruct, verticesOfGraph))
    return false;
    
  coloringVertices(inputGraphStruct, verticesOfGraph, verticesOfGraph[0], false);
  
  for (auto vertex : verticesOfGraph)
  {
    if (!vertex.visited)
      return false;
  }  
  
  for (ColorizedVertex & vertex : verticesOfGraph)
  {
    vertex.visited = false;
  }
  
  if (!vertexColorCheckout(inputGraphStruct, verticesOfGraph, verticesOfGraph[0], false))
    return false;  
   
  return true;
}

bool PappusAgent::vertexDegreeCheckout(ScAddr const & inputGraphStruct, ColorVerticesVector & verticesOfGraph)
{
  for (auto vertex : verticesOfGraph)
  {
    SC_LOG_ERROR("New cycle");
    size_t vertexOutputDegree = 0;

    ScIterator5Ptr it5 = m_memoryCtx.Iterator5(
            vertex.addr,
            ScType::EdgeUCommonConst,
            ScType::NodeConst,
            ScType::EdgeAccessConstPosPerm,
            inputGraphStruct);

    while (it5->Next())
    {
      SC_LOG_ERROR("Before loop checkout");
      if (vertex.addr == it5->Get(2))
      {
        SC_LOG_ERROR("Same node");
        return false;
      }
      SC_LOG_ERROR("After loop checkout");  
        
      if (findInd(verticesOfGraph, it5->Get(2)) == verticesOfGraph.size() + 1)
      {
        SC_LOG_ERROR("Not from our graph");  
        continue;
      }
      
      if (multipleEdgesCheckout(inputGraphStruct, vertex.addr, it5->Get(2)))
      {
        return false;
      }
        
      SC_LOG_ERROR(" + 1");    
      vertexOutputDegree++;
    }
    
    ScIterator5Ptr it5_2 = m_memoryCtx.Iterator5(
            ScType::NodeConst,
            ScType::EdgeUCommonConst,
            vertex.addr,
            ScType::EdgeAccessConstPosPerm,
            inputGraphStruct);

    while (it5_2->Next())
    {
      SC_LOG_ERROR("Before loop checkout.2");
      if (vertex.addr == it5_2->Get(0))
      {
         SC_LOG_ERROR("Same node.2");
        return false;
      }
      SC_LOG_ERROR("After loop checkout.2");  
        
      if (findInd(verticesOfGraph, it5_2->Get(0)) == verticesOfGraph.size() + 1)
      {
        SC_LOG_ERROR("Not from our graph.2");  
        continue;
      }
      
      if (multipleEdgesCheckout(inputGraphStruct, vertex.addr, it5_2->Get(0)))
      {
      SC_LOG_ERROR(" ");
        return false;
      }
      
      SC_LOG_ERROR(" + 1.2");    
      vertexOutputDegree++;
    }
    
    SC_LOG_ERROR(vertexOutputDegree);
    
    if (vertexOutputDegree != 3)
      return false;
      
     SC_LOG_ERROR("After degree checkout"); 
  }
  
  return true;
}

bool PappusAgent::multipleEdgesCheckout(ScAddr const & inputGraphStruct, ScAddr const & firstVertex, ScAddr const & secondVertex)
{
  SC_LOG_ERROR(" ");
  int edgesCount = 0;
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(firstVertex));
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(secondVertex));
  ScIterator5Ptr it5 = m_memoryCtx.Iterator5(
        firstVertex,
        ScType::EdgeUCommonConst,
        secondVertex,
        ScType::EdgeAccessConstPosPerm,
        inputGraphStruct);
  while (it5->Next())
  {
    edgesCount++;
  }
  
  ScIterator5Ptr it5_2 = m_memoryCtx.Iterator5(
        secondVertex,
        ScType::EdgeUCommonConst,
        firstVertex,
        ScType::EdgeAccessConstPosPerm,
        inputGraphStruct);
  while (it5_2->Next())
  {
    edgesCount++;
  }
  
  SC_LOG_ERROR("Edges amount");
  SC_LOG_ERROR(edgesCount);
  
  if (edgesCount <= 1)
  {
    return false;
  }
    
  return true;
}

bool PappusAgent::verticesAmountCheckout(ColorVerticesVector & verticesOfGraph, ScAddr const & inputGraphStruct)
{
  ScIterator3Ptr it3 = m_memoryCtx.Iterator3(
            inputGraphStruct,
            ScType::EdgeAccessConstPosPerm,
            ScType::NodeConst);
  while (it3->Next())
  {
    verticesOfGraph.insert(verticesOfGraph.end(), {it3->Get(2), false, false});
  }
  
  if (verticesOfGraph.size() != 18)
  {
    SC_LOG_ERROR(verticesOfGraph.size());
    return false;
  }
  
  return true;
}

void PappusAgent::coloringVertices( ScAddr const & inputGraphStruct, ColorVerticesVector & verticesOfGraph,
                                    ColorizedVertex & vertexForColoring, bool color)
{
  if (vertexForColoring.visited)
    return;
  
  vertexForColoring.visited = true;  
  vertexForColoring.color = color;
  
  if (color)
    color = false;
  else
    color = true;
  
  ScIterator5Ptr it5 = m_memoryCtx.Iterator5(
            vertexForColoring.addr,
            ScType::EdgeUCommonConst,
            ScType::NodeConst,
            ScType::EdgeAccessConstPosPerm,
            inputGraphStruct);
    while (it5->Next())
    {
      ScAddr newVertex = it5->Get(2);
      long unsigned int ind = findInd(verticesOfGraph, newVertex);
      if (ind == verticesOfGraph.size() + 1)
        continue;
      
      coloringVertices( inputGraphStruct, verticesOfGraph, verticesOfGraph[ind], color);
    }
    
    ScIterator5Ptr it5_2 = m_memoryCtx.Iterator5(
            ScType::NodeConst,
            ScType::EdgeUCommonConst,
            vertexForColoring.addr,
            ScType::EdgeAccessConstPosPerm,
            inputGraphStruct);
    while (it5_2->Next())
    {
      ScAddr newVertex = it5_2->Get(0);
      long unsigned int ind = findInd(verticesOfGraph, newVertex);
      if (ind == verticesOfGraph.size() + 1)
        continue;
        
      coloringVertices(inputGraphStruct, verticesOfGraph, verticesOfGraph[ind], color);
    }
  
}

bool PappusAgent::vertexColorCheckout( ScAddr const & inputGraphStruct, ColorVerticesVector & verticesOfGraph,
                                       ColorizedVertex & vertexForCheckout, bool color)
{
SC_LOG_ERROR("**********************before color comparison");
  if (vertexForCheckout.color != color)
    return false;
  SC_LOG_ERROR("**********************after color comparison");
  if (vertexForCheckout.visited)  
    return true;
  
  vertexForCheckout.visited = true;  
    
  if (color)
  {
    color = false;
  }
  else
  {
    color = true;
  }
    
    SC_LOG_ERROR("**********************before first iterator");
    
  ScIterator5Ptr it5 = m_memoryCtx.Iterator5(
            vertexForCheckout.addr,
            ScType::EdgeUCommonConst,
            ScType::NodeConst,
            ScType::EdgeAccessConstPosPerm,
            inputGraphStruct);
    while (it5->Next())
    {
      ScAddr newVertex = it5->Get(2);
      long unsigned int ind = findInd(verticesOfGraph, newVertex);
      if (ind == verticesOfGraph.size() + 1)
        continue;
      
      if (!vertexColorCheckout(inputGraphStruct, verticesOfGraph, verticesOfGraph[ind], color))
        return false;
    }
     SC_LOG_ERROR("**********************before second iterator");
     
    ScIterator5Ptr it5_2 = m_memoryCtx.Iterator5(
            ScType::NodeConst,
            ScType::EdgeUCommonConst,
            vertexForCheckout.addr,
            ScType::EdgeAccessConstPosPerm,
            inputGraphStruct);
    while (it5_2->Next())
    {
      ScAddr newVertex = it5_2->Get(0);
      long unsigned int ind = findInd(verticesOfGraph, newVertex);
      if (ind == verticesOfGraph.size() + 1)
        continue;
        
      if (!vertexColorCheckout(inputGraphStruct, verticesOfGraph, verticesOfGraph[ind], color))
        return false;
    }
     SC_LOG_ERROR("**********************after all");
     
  return true;
}

void PappusAgent::formResponseConstruction(
    ScAddr const & inputGraphStruct,
    ScAddrVector & answerElements, bool response)
{
  ScAddr searchResultRelationPair;
  if (response)
  {
    searchResultRelationPair = m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm, Keynodes::pappus_graph, inputGraphStruct);
  }
  else
  {
    searchResultRelationPair = m_memoryCtx.CreateEdge(ScType::EdgeAccessConstNegPerm, Keynodes::pappus_graph, inputGraphStruct );
  }

  answerElements.insert(answerElements.end(), {searchResultRelationPair, Keynodes::pappus_graph});
}

long unsigned int PappusAgent::findInd(ColorVerticesVector & vector, ScAddr addr)
{
   for (long unsigned int i = 0; i < vector.size(); i++)
     if (vector[i].addr == addr)
       return i;
   
   return vector.size() + 1;
}

}
