# Агент проверки на цикличность

## Полуение входных данных

В данном случае у нас только один аргумент попадает на вход, поэтому вектор аргументов не потребуется. Обращаемся к otherAddr и достаем наш аргумент:

```с++
ScAddr actionNode = otherAddr;
ScAddr inputGraph = IteratorUtils::getAnyFromSet(ms_context.get(), actionNode);
```

Сразу же добавим проверку нашего аргумента и завершение работы агента, если наш аргумент не подходит:

```с++
if (!inputGraph.IsValid()) {
    SC_LOG_ERROR("CycleCheckAgent:  Invalid argument");
    utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
}
```

## Формирование ответа агента

Создадим вектор наших ответов, в который позже будем добавлять объекты для ответа:

```с++
ScAddrVector answerElements;
```

Для формирования ответа агента создадим отдельную функцию, в которой уже вызовем непосредственно проверку графа. Добавим в заголовочный файл:

```с++
void formResultConstruction(ScAddrVector & answerElements, ScAddr const & inputGraph);
```

Определяемся с видом ответа нашего графа. Задача заключается в нахождении простого ответа да/нет, конструкция будет представлять из себя узел класса циклических графов(добавить в keynodes), непосредственно наш аргумент и дугу(положительную либо отрицательную в зависимости от ответа) от первого ко второму.

Из переменных тут только дуга. Объявим её:

```с++
ScAddr checkoutResult;
```

Создадим if для создания необходимой дуги, где flag это результат нашей проверки(true - в графе нет циклов, false - есть как минимум один цикл):

```с++
if ( flag )
        checkoutResult = m_memoryCtx.CreateEdge(ScType::EdgeAccessConstNegPerm, Keynodes::cycle_graph_set, inputGraph);
    else
        checkoutResult = m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm, Keynodes::cycle_graph_set, inputGraph);

```

Добавим элементы ответа в вектор ответов:

```с++
answerElements.push_back(Keynodes::cycle_graph_set);
answerElements.push_back(checkoutResult);
```

Однако нам нужно для начала отчистить ответы созданные этим же агентом в прошлый раз, поэтому создадим доп функцию для удаления сформерованных ранее подобных дуг. Для этого воспользуемся итератором3 для их нахождения. Направление узла мы знаем, parent-узлом будет выступать узел класса, а child-узлом будет непосредственно наш аргумент.

Сама итоговая функция:

```с++
void CycleCheckAgent::clearPreviousResults(ScAddr const & inputGraph) {
    ScIterator3Ptr it3 = m_memoryCtx.Iterator3(
            Keynodes::cycle_graph_set,
            ScType::Unknown,
            inputGraph);

    while (it3->Next())
        m_memoryCtx.EraseElement(it3->Get(1));
}
```

Добавление в заголовочный файл будут игнорировать(и так понятно, что это нужно сделать)

Добавим вызов функции очистки в нашу функцию формирования результата:

```с++
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
```

## Проверка графа на цикличность

Для проверки графа на цикличность нам потребуется отдельная структура данных, тк в соотоветствие каждой нашей вершине будет ставиться его состояние:

```c++

struct VisitedNode {
    ScAddr addr;
    int cond;
};
using VisitedNodeVector = std::vector<VisitedNode>;
```

Чтобы в будущем нам находить нашу вершину в векторе нам потребуется функция поиска:

```c++
ssize_t CycleCheckAgent::searchNodeByAddr(VisitedNodeVector const & vector, ScAddr const & addr) {
    for (ssize_t i = 0; i < vector.size(); i++)
        if (vector[i].addr == addr)
            return i;

    return -1;
}
```

Сохраним все вершины нашего графа в вектор структур с cond = 0;

Сам алгоритм проверки примерно таков: с помощью поиска в глубину обходим все вершины, как только заходим в вершину проверяем её статус(0 - ещё не участвовала в обходе, 1 - зашли, но не вышли; 2 - проверили и вышли). Вызываем эту функцию рекурсивно для всех соседних, если они ещё не посещены.

Возникает проблема уже описанная гайде(проблем неориенитрованных дуг). Если мы поставим два итератора для двух позиций(child/parent) изначального угла, то во время дфса мы перейдем в узел, и потом сразу же вернёмся обратно. Ну и это доставит нам дохуя проблем и наебнёт наш алгоритм. Чтобы избежать подобного создадим вектор, в который будем добавлять неориентированные дуги, по которым мы уже перешли. Затем в каждом итераторе будет проверка на на уже пройденные неор дуги(их будем пропускать).

Всего нам потребуется три итератора5(пятёрки, тк нам нужно переходить только по связям из нашего графа см проблему одного узла). Два для неориентированных дуг, тк нужно чекнуть оба направления, и один на ориентированные тк там все норм(идём всегда в одном направлении). И в каждый итератор по проверке увторого узла в кадом итераторе на пренадлежность нашему графу.

Итоговая рекурсивная функция будет иметь вид:

```c++
bool CycleCheckAgent::cycleCheckout(VisitedNodeVector & nodes, ScAddrVector & noorEdges,
                                    VisitedNode & currentNode, ScAddr const & graph)
{
    if (currentNode.cond == 2)
        SC_LOG_ERROR("Checked node");
        return true;
    if (currentNode.cond == 1)
        return false;

    currentNode.cond = 1;

    ScIterator5Ptr it5np = m_memoryCtx.Iterator5(
            currentNode.addr,
            ScType::EdgeUCommonConst,
            ScType::NodeConst,
            ScType::EdgeAccessConstPosPerm,
            graph);

    while ( it5np->Next() ) {
        if ( find( noorEdges.begin(), noorEdges.end(), it5np->Get(1) ) != noorEdges.end() )
            continue;

        noorEdges.push_back( it5np->Get(1) );

        ssize_t ind = searchNodeByAddr( nodes, it5np->Get(2) );
        if (ind == -1)
            continue;

        if ( !cycleCheckout(nodes, noorEdges, nodes[ind], graph) )
            return false;
    }

    ScIterator5Ptr it5nc = m_memoryCtx.Iterator5(
            ScType::NodeConst,
            ScType::EdgeUCommonConst,
            currentNode.addr,
            ScType::EdgeAccessConstPosPerm,
            graph);

    while ( it5nc->Next() ) {
        if ( find( noorEdges.begin(), noorEdges.end(), it5nc->Get(1) ) != noorEdges.end() )
            continue;

        noorEdges.push_back( it5nc->Get(1) );

        ssize_t ind = searchNodeByAddr( nodes, it5nc->Get(0) );
        if (ind == -1)
            continue;

        if ( !cycleCheckout(nodes, noorEdges, nodes[ind], graph) ) {
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
        if ( find( noorEdges.begin(), noorEdges.end(), it5->Get(1) ) != noorEdges.end() )
            continue;
            
        ssize_t ind = searchNodeByAddr( nodes, it5->Get(2) );
        if (ind == -1)
            continue;

        if ( !cycleCheckout(nodes, noorEdges, nodes[ind], graph) ) {
            return false;
        }
    }

    currentNode.cond = 2;
    return true;
}
}
```

Также нам нужно предусмотреть случай, когда наш граф несвязный. Как вариант, можно просто пройтись циклом по вектору наших структур после вызова дфса для одного узла и проверить их текущее состояние. Если узел остался непосещённым, то вызовем проверку для него отдельно и так далее:

```с++
bool CycleCheckAgent::getCheckoutResult(const ScAddr & inputGraph) {
    SC_LOG_ERROR("Checkout starts");
    VisitedNodeVector graphNodes;
    ScAddrVector noorEdges;
    
    //Добавляем вершины в вектор
    ScIterator3Ptr it3 = m_memoryCtx.Iterator3(
            inputGraph,
            ScType::EdgeAccessConstPosPerm,
            ScType::NodeConst);
    while ( it3->Next() )
        graphNodes.push_back( {it3->Get(2), 0} );

    //Запускаем проверку для всез непроверенных узлов
    for (auto & node : graphNodes) {
        if (node.cond != 0)
            continue;
        
        //Если хоть одна проверка не прошла, возвращаем ответ false везде
        if ( !cycleCheckout(graphNodes, noorEdges, node, inputGraph) )
            return false;
    }

    return true;
}
```

И теперь добавим в функцию формирования ответа строчку:

```c++
flag = getCheckoutResult(inputGraph);
```

И в мейне завершаем выполнение агента с флагом true и передаём в ответ вектор ответов.

Данон

[Сам агент]()
