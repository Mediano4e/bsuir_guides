#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "PappusAgent.generated.hpp"

namespace exampleModule
{

struct ColorizedVertex 
{
  ScAddr addr;
  bool visited;
  bool color;
};

using ColorVerticesVector = std::vector<ColorizedVertex>;

class PappusAgent : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_find_pappus_graph, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()
  
  void formResponseConstruction(ScAddr const & inputGraphStruct, ScAddrVector & answerElements, bool response);
                     
  bool pappusCheckout(ScAddr const & inputGraphStruct);
  
  bool verticesAmountCheckout(ColorVerticesVector & verticesOfGraph, ScAddr const & inputGraphStruct);
  
  bool vertexDegreeCheckout(ScAddr const & inputGraphStruct, ColorVerticesVector & verticesOfGraph);
  
  bool multipleEdgesCheckout(ScAddr const & inputGraphStruct, ScAddr const & firstVertex, ScAddr const & secondVertex);
  
  void coloringVertices( ScAddr const & inputGraphStruct, ColorVerticesVector & verticesOfGraph,
                         ColorizedVertex & vertexForColoring, bool color);
  
  bool vertexColorCheckout( ScAddr const & inputGraphStruct, ColorVerticesVector & verticesOfGraph,
                            ColorizedVertex & vertexForChecking, bool color);
  
  static long unsigned int findInd(ColorVerticesVector & vector, ScAddr addr);
};

}
