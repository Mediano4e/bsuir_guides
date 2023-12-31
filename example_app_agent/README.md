---

# Гайд по написанию агента в рамках example lab

У репозитория есть раздел Discussions, при желании можете заглянуть туда

Если уже написали агента, закидывайте в папки с агентами(желательно не без ридмика с описанием, что у вас в агенте происходит) пул реквестами.

Также можете корректировать содержимое самого гайда и кидать пул реквестами.

---


## Содержание
- [Источники и документация](#sources)
- [Для начала](#begin)
    - [Регистрация](#registration)
    - [Основные моменты](#moments)
- [Вызов агента, получение входных данных, вывод результата](#input)
- [Реализация алгоритма](#algorithm)
    - [Итераторы3 и Итераторы5](#iterators)
        - [Итераторы3](#iterator3)
        - [Итераторы5](#iterator5)
        - [Шаблоны](#pattern)
- [Таблица типов](#type_table)
- [Основные проблемы](#main_problems)
    - [Проблема неориенированных дуг](#noorient_problem)
    - [Проблема случайных результатов поиска](#random_neghbours)
	- [Хранение SC-объектов в контейнерах STL](#stl_containers)

---

## <a id="sources">Источники и документация</a>

- Вот [это](https://github.com/ostis-ai/ostis-web-platform/blob/develop/docs/main.pdf) просто почитать по желанию.
- А вот [это](http://ostis-dev.github.io/sc-machine/) более ли менее вменяемая документация.

---

## <a id="begin">Для начала</a>

---

### <a id="registration">Предполагается, что вы уже понимаете как регистировать агента, но пройдемся по основным пунктам:</a>

- В папке со спецификациями копируем папку любого агента, удаляем оттуда все кроме двух Scs файлов, переименовываем всё, в файлах заменяем все названия от других агентов на свои(ищите сами либо смотрите [тут](https://github.com/Mediano4e/bsuir/blob/main/%D0%A2%D1%80%D0%B5%D1%82%D0%B8%D0%B9%20%D1%81%D0%B5%D0%BC/%D0%9F%D0%9F%D0%9E%D0%98%D0%A1/agent/guide_%D0%BF%D0%BE_%D1%80%D0%B5%D0%B3%D0%B8%D1%81%D1%82%D1%80%D0%B0%D1%86%D0%B8%D0%B8_%D0%B2_call_of_dragons.pdf))

- В keynodes можно добавлять абсолютно все вершины, которые вы собираетесь использовать во время использования агента, но не собираетесь передавать как аргументы(что-либо типо узла результата, узла класса, которому должен принадлежать объект и тд), тут полная свобода. Все keynodes можно свободно использовать в агенте в любой его точке. Обязательно нужен только узел запроса, тк на нем завязана работа агента.

- в exapleModule.cpp непосредственно регистрируем агента

- создаем в папке с агентом, не поверите, нашего агента(cpp и hpp), для этого можно скопировать subdevidingAgent, но почистить всё содержимое внутри мейна + все функции, а так же заменив всё, что нужно на названия из своего агента.

---

### <a id="moments">Основные моменты</a>

- Любые объекты одного типа с одинаковым системным идентификатором будут являться одним объектом. Например в первом нашем графе была следующая кострукция: concept_cat <- animal_set; а во втором графе concept_cat <- pets_set; и если мы обратимся к этому узлу, мы получим обе связи. Это просто нужно держать в голове. [Это может вызывать некоторые проблемы](#random_neghbours)

- Вся логика веба представляется в виде графа и обрабатывается sc-машиной, поэтому все кнопки фактически являются вершинами графа интерфейса, и это будет прослеживаться много где

- Вершины и рёбра являются самостоятельными объектами. Взаимодействие с ними осуществляется с помощью типа данных ScAddr. Это можно сказать что-то типо указателей в с++, создавай переменную типа ScAddr мы указываем на объект, но фактически им не владеем(он где-то в sc-памяти). Поэтому взамодействие с ним осуществляется с помощью отдельных средств

- когда создаётся ребро, оно создается от PARENT-объекта к CHILD-объекту, поэтому проверяя, какие есть связи у имеющейся вершины, нужно учитывать, что если рёбра неориентированные, нужно будет выполнять проверки с нашей вершиной на позиции PARENT и на позиции CHILD поотдельности. [Это может вызывать некоторые проблемы](#noorient_problem)

---

## <a id="input">Вызов агента из веба, получение аргументов и завершение агениов с выводом результата</a>

Как было написано ранее весь интерфейс фактически является графом. В интерфейсе существует узел otherAddr, который фактически является контейнером для аргументов. Когда мы нажимаем пкм по индификатору объекта в вебе и выбираем булавку(закрепить), от узла otherAddr проводится простое ребро принадлежности(ScType::EdgeAccessConstPosPerm) к данному объекту и у нас снизу веба отобраджается идентификатор обЪекта. После этого при нажатии на кнопку вызова нашего агента, проведётся связь от узла, который входит в графовую структуру агента, под системным идентификатором listenAddr к otherAddr. Таким образом наши аргументы станут частью структуры нашего агента. До проведения этого ребра проверяется количество аргументов, связанных с otherAddr. Если их недостаточно(в спецификациях в файлике ui_menu мы запросили большее количество аргументов), связь проведена не будет. Если аргументов наоборот слишком много, будут взяты первые переданные в нужном количестве. Непосредственно проведение этого ребра и является событием, на которое реагирует наш агент(добавление исходящей дуги)

<p align="center">

  <img width="186" height="302" src="https://github.com/Mediano4e/bsuir_guides/assets/115105818/917f88c8-31a4-48ba-9c4e-deddf7330d20">

</p>

Теперь нам нужно получить переданные аргументы в самом агенте. Для этого нам надо обратиться непосредственно к otherAddr.

```c++
ScAddr actionNode = otherAddr; //сам по себе otherAddr не может быть использован
```
Если у нас передан один аргумент, то можем его получить через команду(фактически, забегая наперёд, getAnyFromSet использует простейший итератор-тройку):
```c++
ScAddr myInputObject = IteratorUtils::getAnyFromSet(ms_context.get(), actionNode); 
```

Если же у нас больше одного аргумента, то используем для этого цикл с итератором и вектор аргументов:
```c++
ScAddr actionNode = otherAddr;

ScAddrVector myInputObjects;

ScIterator3Ptr it3 = m_memoryCtx.Iterator3(actionNode, ScType::EdgeAccessConstPosPerm, ScType::Unknown);
while( it3->Next() )
    myInputOdjects.push_back( it3->Get(2) ) //добавляем наш объект неизвестного типа
```

---

Теперь разберёмся с завершением работы агента. 

Если нашему агенту не нужно выводить какие-либо результаты(допустим аргумент оказалась нерабочим), то нам нужно завершить его с флагом false и передать ему узел с вводными данными, чтобы он отчистил его:

```c++
ScAddr actionNode = otherAddr;
utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
```

Если наш агентом всё успешно выполнил и мы хотим вывести что-то в качестве ответа, то мы завершаем его с флагом true и передаём ему помимо узла с аргументами вектор всех Sc-объектов, которые мы хотим вывести помимо самих аргументов в качестве результата:

```c++
ScAddr actionNode = otherAddr;
ScAddrVector answerElements;
utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, answerElements, true);
```
!!! Важно !!! Выведется только то, что будет тем или иным образом иметь связи(и эти связи будут в векторе ответов) с аргументами либо узлами агента, иначе у агента с ними просто не будет никакой связи и он их просто не сможет подтянуть.

---

## <a id="algorithm">Реализация алгоритма</a>

Если вы будете писать в обычных редакторах кода типо VS или обычного блокнота, то для отладки советую использовать для вывода в консоль:
```c++
SC_LOG_ERROR("something to show in console");
```

Если хотите получить системный идентификатор строкой, то можно использовать(в данном случае название выведется в консоль):

```c++
SC_LOG_ERROR( m_memoryCtx.HelperGetSystemIdtf(yourVertex) );
```

Если запросить идентификатор у объекта без него, всё ляжет xdd.

Если же у вас Clion, то весь example-app можно открыть как проект и работать непосредственно с ним. Там все намного проще будет, только конфиг настроить придётся.

---

### __Главным инструментом для реализации алгоритма являются Итераторы и Шаблоны__

---

### <a id="iterators">Итераторы3 и Итераторы5</a>

<p align="left">

  <img width="411" height="135" src="https://ostis-dev.github.io/sc-machine/cpp/images/iterators_scheme.png">

</p>

Итераторы являются универсальным инструментом для перехода от вершины к вершине и для прочих манипуляций с графовыми структурами. Итераторы ищут маленькие структуры на 2 вершины и одну связь без узла отношений, либо с ним и соответствущим ему ребром. Для них нужно задавать типы элементов константными типами, чтобы получить обычные объекты.

---

### <a id="iterator3">__Итератор3__</a>

Чтобы наш шаблон заработал, нам нужно хотя бы один объект задать явно.

То есть вот синтаксис:

```c++
ScAddr actioNode = otherAddr;
ScAddr myInputNode = IteratorUtils::getAnyFromSet(ms_context.get(), actionNode); 

ScIterator3Ptr it3 = m_memoryCtx.Iterator3(
            myInputObject,            // ->Get(0)
            ScType::Unknown,          // ->Get(1)
            ScType::Unknown);         // ->Get(2)

while (it3->Next())
{
    SC_LOG_ERROR( m_memoryCtx.HelperGetSystemIdtf( it3->Get(2) ) );
}
```
В примере выше мы вернём все связи, в которых myInputNode является РОДИТЕЛЬСКИМ узлом, и соответствующие им объекты(это могут быть и узлы и связи). Через цикл while будут перебираться результаты поиска. Можно поставить myInputNode на позицию узла НАСЛЕДНИКА(позиция 2), и тогда результаты будут другими. Направление рёбер(стрелок) от узла родителя к узлу наследнику.

Каждому неизвестному можно задать тип(задать объект неявно), чтобы получить именно то, что нам нужно. Вот пример, который пройдется по всем конструкциям включающим myInputNode родительским узлом, некоторую связь и некоторый константный узел:  

```c++
ScAddr actioNode = otherAddr;
ScAddr myInputObject = IteratorUtils::getAnyFromSet(ms_context.get(), actionNode); 

ScIterator3Ptr it3 = m_memoryCtx.Iterator3(
            actionNode,               // ->Get(0)
            ScType::Unknown,          // ->Get(1)
            ScType::NodeConst);       // ->Get(2)
    
while (it3->Next())
{
    SC_LOG_ERROR( m_memoryCtx.HelperGetSystemIdtf( it3->Get(2) ) );
}
```

---

### <a id="iterator5">__Итератор5__</a>

По своей сути Итератор5 отличается от Итератора3 только тем, что ищет конструкции, у которых есть узел отношений(впринципе это может быть любой узел соответствующий данному шаблону).

Пример синтаксиса:
```c++
ScAddr actionNode = otherAddr;
ScAddr myInputNode = IteratorUtils::getAnyFromSet(ms_context.get(), actionNode); 

ScIterator5Ptr it5 = m_memoryCtx.Iterator5(
        myInputNode,                        // ->Get(0)
        ScType::Unknown,                    // ->Get(1)
        ScType::NodeConst,                  // ->Get(2)
        ScType::EdgeAccessConstPosPerm,     // ->Get(3)
        ScType::NodeConst);                 // ->Get(4)

while (it5->Next())
{
    SC_LOG_ERROR( m_memoryCtx.HelperGetSystemIdtf( it5->Get(2) ) );
    SC_LOG_ERROR( m_memoryCtx.HelperGetSystemIdtf( it5->Get(4) ) );
}
```

Т.к. это работает так же как и шаблоны, Итератор3 найдет конструкции и на пять элементов, но полностью проигнорирует наличие аргументов 3 и 4. А вот Итератор5 конструкции без узла отношений обходить не будет.

---

### <a id="pattern">__Шаблоны__</a>

Шаблоны отличаются от итераторов тем, что их можно увеличивать в размерах до бесконечности, а также выбранный шаблон можно искать в том числе только в необходимой структуре. Пример создания шаблона совподающего с итератором3:

```c++
ScTemplate templ;
templ.Triple(
  param1,
  ScType::EdgeAccessVarPosPerm,
  ScType::NodeVar
);
```

Стоит заметить, что все объекты задаются в переменной форме, чтобы найти константные(обычные) объекты.

Пример более большого шаблона:

```c++
ScTemplate templ;
templ.Triple(
  device_addr,    // sc-addr of device node
  ScType::EdgeAccessVarPosPerm,
  ScType::NodeVar >> "_device_instance"
);
templ.Triple(
  device_enabled_addr,    // sc-addr of device_enabled node
  ScType::EdgeAccessVarPosPerm,
  "_device_instance"
);
```

Если вы хотите указать, что какой-то объект должен быть одним и тем же, его надо будет назвать при первом упоминании и дальше использовать его как аргумент по его названию. В предыдущем примере был указан некоторый узел "_device_instance" как общий для двух разных частей шаблона.

Также можно увеличивать шаблон конструкциями вида итераторов5:

```c++
ScTemplate templ;
templ.TripleWithRelation(
  param1,
  ScType::EdgeDCommonVar,
  param2,
  ScType::EdgeAccessVarPosPerm,
  some_nrel
);
```

Теперь перейдём к получению результата поиска  
Результат поиска шаблоном является отдельным типом данных 
```c++
ScTemplateSearchResult searchResult;
```  
Выполнив поиск мы сохряняем ответ в созданную заранее структуру результатов 
```c++
ms_context->HelperSearchTemplate(templ, searchResult);
```  
После этого нам нужно проверить не вернулась ли нам пустая структура, тк в таком случае при обращении всё умрёт 
```c++
if(!searchResult.IsEmpty())
```
И теперь мы можем перебрать нашу структуру как массив. Если мы хотим получить адрес какого-то объекта в каждом ответе, то нужно будет его назвать в самом шаблоне. Тогда при переборе результатов мы сможем к нему обратится.

Пример:

```c++
ScAddrVector resultNodes;

ScTemplate templ;

templ.TripleWithRelation(
  myNode,
  ScType::EdgeDCommonVar,
  ScType::NodeVar >> "_result_node",
  ScType::EdgeAccessVarPosPerm,
  some_nrel
);
templ.Triple(
  hello_kitty_set,
  ScType::EdgeAccessVarPosPerm,
  "_result_node"
);


ScTemplateSearchResult searchResult;

ms_context->HelperSearchTemplate(templ, searchResult);

if(!searchResult.IsEmpty())
  for (size_t i = 0; i < searchResult.Size(); i++)
    resultNodes.push_back( searchResult[i]["_result_node"] );
``` 

Так же можно искать только те шаблоны, которые пренадлежат определённой структуре:
``` 
ms_context.HelperSearchTemplateInStruct(templ, myStructNode, searchResult);
``` 

---

##  <a id="type_table">Все типы объектов можно найти тут:</a>

<table>
  <tr>
    <th>Graphical (SCg)</th>
    <th>C</th>
    <th>C++</th>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node.png"></img></td>
    <td>sc_type_node</td>
    <td>ScType::Node</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_const.png"></img></td>
    <td>sc_type_node | sc_type_const</td>
    <td>ScType::NodeConst</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_var.png"></img></td>
    <td>sc_type_node | sc_type_var</td>
    <td>ScType::NodeVar</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_const_tuple.png"></img></td>
    <td>sc_type_node | sc_type_const | sc_type_node_tuple</td>
    <td>ScType::NodeConstTuple</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_var_tuple.png"></img></td>
    <td>sc_type_node | sc_type_var | sc_type_node_tuple</td>
    <td>ScType::NodeVarTuple</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_const_struct.png"></img></td>
    <td>sc_type_node | sc_type_const | sc_type_node_struct</td>
    <td>ScType::NodeConstStruct</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_var_struct.png"></img></td>
    <td>sc_type_node | sc_type_var | sc_type_node_struct</td>
    <td>ScType::NodeVarStruct</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_const_role.png"></img></td>
    <td>sc_type_node | sc_type_const | sc_type_node_role</td>
    <td>ScType::NodeConstRole</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_var_role.png"></img></td>
    <td>sc_type_node | sc_type_var | sc_type_node_role</td>
    <td>ScType::NodeVarRole</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_const_norole.png"></img></td>
    <td>sc_type_node | sc_type_const | sc_type_node_norole</td>
    <td>ScType::NodeConstNorole</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_var_norole.png"></img></td>
    <td>sc_type_node | sc_type_var | sc_type_node_norole</td>
    <td>ScType::NodeVarNorole</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_const_class.png"></img></td>
    <td>sc_type_node | sc_type_const | sc_type_node_class</td>
    <td>ScType::NodeConstClass</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_var_class.png"></img></td>
    <td>sc_type_node | sc_type_var | sc_type_node_class</td>
    <td>ScType::NodeVarClass</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_const_abstract.png"></img></td>
    <td>sc_type_node | sc_type_const | sc_type_node_abstract</td>
    <td>ScType::NodeConstAbstract</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_var_abstract.png"></img></td>
    <td>sc_type_node | sc_type_var | sc_type_node_abstract</td>
    <td>ScType::NodeVarAbstract</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_const_material.png"></img></td>
    <td>sc_type_node | sc_type_const | sc_type_node_material</td>
    <td>ScType::NodeConstMaterial</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_node_var_material.png"></img></td>
    <td>sc_type_node | sc_type_var | sc_type_node_material</td>
    <td>ScType::NodeVarMaterial</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_common.png"></img></td>
    <td>sc_type_edge_common</td>
    <td>ScType::EdgeUCommon</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_common_orient.png"></img></td>
    <td>sc_type_arc_common</td>
    <td>ScType::EdgeDCommon</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_const_common.png"></img></td>
    <td>sc_type_edge_common | sc_type_const</td>
    <td>ScType::EdgeUCommonConst</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_var_common.png"></img></td>
    <td>sc_type_edge_common | sc_type_var</td>
    <td>ScType::EdgeUCommonVar</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_const_common_orient.png"></img></td>
    <td>sc_type_arc_common | sc_type_const</td>
    <td>ScType::EdgeDCommonConst</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_var_common_orient.png"></img></td>
    <td>sc_type_arc_common | sc_type_var</td>
    <td>ScType::EdgeDCommonVar</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_access.png"></img></td>
    <td>sc_type_arc_access</td>
    <td>ScType::EdgeAccess</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_const_pos_perm.png"></img></td>
    <td>sc_type_arc_access | sc_type_const | sc_type_arc_pos | sc_type_arc_perm</td>
    <td>ScType::EdgeAccessConstPosPerm</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_var_pos_perm.png"></img></td>
    <td>sc_type_arc_access | sc_type_var | sc_type_arc_pos | sc_type_arc_perm</td>
    <td>ScType::EdgeAccessVarPosPerm</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_const_neg_perm.png"></img></td>
    <td>sc_type_arc_access | sc_type_const | sc_type_arc_neg | sc_type_arc_perm</td>
    <td>ScType::EdgeAccessConstNegPerm</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_var_neg_perm.png"></img></td>
    <td>sc_type_arc_access | sc_type_var | sc_type_arc_neg | sc_type_arc_perm</td>
    <td>ScType::EdgeAccessVarNegPerm</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_const_fuz_perm.png"></img></td>
    <td>sc_type_arc_access | sc_type_const | sc_type_arc_fuz | sc_type_arc_perm</td>
    <td>ScType::EdgeAccessConstFuzPerm</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_var_fuz_perm.png"></img></td>
    <td>sc_type_arc_access | sc_type_var | sc_type_arc_fuz | sc_type_arc_perm</td>
    <td>ScType::EdgeAccessVarFuzPerm</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_const_pos_temp.png"></img></td>
    <td>sc_type_arc_access | sc_type_const | sc_type_arc_pos | sc_type_arc_temp</td>
    <td>ScType::EdgeAccessConstPosTemp</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_var_pos_temp.png"></img></td>
    <td>sc_type_arc_access | sc_type_var | sc_type_arc_pos | sc_type_arc_temp</td>
    <td>ScType::EdgeAccessVarPosPerm</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_const_neg_temp.png"></img></td>
    <td>sc_type_arc_access | sc_type_const | sc_type_arc_neg | sc_type_arc_temp</td>
    <td>ScType::EdgeAccessConstNegTemp</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_var_neg_temp.png"></img></td>
    <td>sc_type_arc_access | sc_type_var | sc_type_arc_neg | sc_type_arc_temp</td>
    <td>ScType::EdgeAccessVarNegPerm</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_const_fuz_temp.png"></img></td>
    <td>sc_type_arc_access | sc_type_const | sc_type_arc_fuz | sc_type_arc_temp</td>
    <td>ScType::EdgeAccessConstFuzTemp</td>
  </tr>

  <tr>
    <td><img src="https://ostis-dev.github.io/sc-machine/images/scg/scg_edge_var_fuz_temp.png"></img></td>
    <td>sc_type_arc_access | sc_type_var | sc_type_arc_fuz | sc_type_arc_temp</td>
    <td>ScType::EdgeAccessVarFuzPerm</td>
  </tr>

</table>

---

## Создание объектов

Чтобы создать непосредственно объекты, которые попадут в вектор выходных, либо просто для дальнейшего использования можно использовать:

```c++
ScAddrVector answerElements;

//создание пустого узла определённого типа 
ScAddr nodeForAnswer = m_memoryCtx.CreateNode(ScType::NodeConst);  

//создание дуги опредённого типа от родительского узла к узлу наследнику
ScAddr edgeForAnswer1 = m_memoryCtx.CreateEdge(ScType::EdgeDCommonConst, myParentNode, myChildNode); 

//создание дуги опредённого типа от родительского узла к другому ребру
ScAddr edgeForAnswer2 = m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm, nodeForAnswer, edgeForAnswer1); 

//И добавляем всё, что мы использовали, но что не является одним из изначальных аргументов, в вектор ответов
answerElements.insert(answerElements.end(), {nodeForAnswer, myParentNode, myChildNode, edgeForAnswer1, edgeForAnswer2});
```

Если же вам нужны узлы с определёнными идентификаторами, задекларируйте их в Keynodes, их можно будет использовать в агенте в виде Keynodes::my_node, его тоже нужно будет добавлять в вектор ответов.

Если вам нужно удалить какой-либо объект, то это можно сделать так:

```c++
m_memoryCtx.EraseElement(objectForDelete);
```

---

## <a id="main_problems">Основные проблемы при реализации агента</a>

Здесь описанны основные неприятные моменты при написании агентов. 

---

### <a id="noorient_problem">Проблема неориентированных дуг</a>

У любых дуг есть родительский узел и узел наследник, и пока мы используем ориентированные дуги проблем не возникает.  
Однако также существуект в SС одна неориентированная дуга, но она тоже попадает под эту концепцию.

Если мы хотим получить все узлы, с которыми у данного узла неориентированная связь, придётся использовать два итератора(один с искомым узлом на позиции родителя и второй -- на позиции наследника):

```c++
ScAddrVector neghbourNodes;

ScIterator3Ptr it3_1 = m_memoryCtx.Iterator3(
            myNode,              
            ScType::EdgeUCommonConst,          
            ScType::NodeConst);      
    
while (it3_1->Next())
{
    neghbourNodes.insert(neghbourNodes.end(), it3_1->Get(2))
}

ScIterator3Ptr it3_2 = m_memoryCtx.Iterator3(
            ScType::NodeConst,              
            ScType::EdgeUCommonConst,          
            myNode);      
    
while (it3_2->Next())
{
    neghbourNodes.insert(neghbourNodes.end(), it3_2->Get(0))
}
```
[К началу](#begin)

---

### <a id="random_neghbours">Проблема случайных результатов поиска</a>

Учитывая, что у нас все узлы с одинаковыми системными идентификаторами объединяются в один узел, у наших узлов могут возникнуть куча непонятных связей, которые нам нужно как-то обрабатывать.

Можно выполнять поиск с помощью шаблонов в определенной структуре. Это будет работать, тк мы передаём наш граф по идентификатору контура, являющегося в свою очередь структурой, которой пренадлежит всё, что внутри.

```c++
ScTemplate templ;
templ.Triple(
  param1,
  ScType::EdgeAccessVarPosPerm,
  ScType::NodeVar
);

ScTemplateSearchResult searchResult;

ms_context->HelperSearchTemplate(templ, myStruct, searchResult);

if(!searchResult.IsEmpty())
  for (size_t i = 0; i < searchResult.Size(); i++)
    ...

```
[К началу](#begin)

---

### <a id="stl_containers">Использование контейнеров STL с SC-объектами</a>

В процессе реализации агента может встать вопрос эффективного хранения большого количества данных.

Очевидным решением является использование хеш-таблиц, которые в STL представлены классами unordered_set, unordered_map. Здесь мы столкнёмся с неочевидной проблемой.

```c++
// Этот код не скомпилируется
std::unordered_map<ScAddr, ScAddr> testMap; 

```

Благодаря интуитивным логам компиляции C++ у вас, скорее всего, не получится догадаться, почему создать указанный объект проблематично.

Для использования ассоциативных контейнеров STL требуется указать используемую хеш-функцию. Такая функция (а вернее, две, так как отличаются длиной хеша) уже реализованы в ScAddr.cpp.
Хеш-функция указывается шаблонным аргументом при создании объекта. Требуемый код будет выглядеть следующим образом:

```c++
// Корректное использование unordered_map с типом ScAddr
std::unordered_map<ScAddr, ScAddr, ScAddrHashFunc<uint32_t>> testMap; 

// Корректное использование unordered_set с типом ScAddr
std::unordered_set<ScAddr, ScAddrHashFunc<uint32_t>> testSet; 

```

Похожая проблема может встретиться при использовании упорядоченных контейнеров, например std::set.

```c++
// Этот код не скомпилируется
std::set<ScAddr> testSet; 

```

Здесь же для корректной работы контейнера требуется указать функцию сравнения (less). Она также реализована в ScAddr.cpp. Требуемый код будет выглядеть так:

```c++
// Корректное использование set с типом ScAddr

// Создание объекта класса компаратора
ScAddrLessFunc lessFunc;

// Передача его в качестве шаблонного аргумента
std::set<ScAddr, lessFunc> testMap; 

```

Стоит отметить, что явное указание компаратора также необходимо для работы некоторых алгоритмов из стандартной библиотеки.

```c++
// Использование стандартных алгоритмов с ScAddrVector

// Задание вектора, который требуется обработать
ScAddrVector nodes;

// Создание объекта класса компаратора
ScAddrLessFunc lessFunc;

// Функция сортировки требует указание компаратора
std::sort(nodes.begin(), nodes.end(), lessFunc);

// Функция перестановки требует указание компаратора
std::next_permutation(sourceNodes.begin(), sourceNodes.end(), lessFunc)

```

todo: Описать хранение ссылок на ScAddr в контейнерах STL.

[К началу](#begin)

