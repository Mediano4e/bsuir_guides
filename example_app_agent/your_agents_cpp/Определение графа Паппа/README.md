# Агент определения графа Паппа

Данный агент в очень сыром виде, много чего нужно переделать, поэтому там может быть куча странных/тупых и тд моментиков. Однако он работает)

---

## Алгоритм

1. Сохранение всех константных вершин в вектор и проверка его размера(должна быть 18)
    - Вектор структур для расскраски графа
2. Проверка количества соседних вершин для каждой вершины из векора + паралелльно проверки на циклы, не пренадлежащие к нашему графы вершины и множественные связи между двумя вершинами.
3. Раскраска графа с помощью поиска вглубину(dfs)
4. Проверка графа на двудольность(тоже dfs) - если две соседние вершины одного цвета = кринж + не двудольный граф
