По условию задачи, нельзя прочитать крупный файл разом, а значит, 
память - это критический ресурс.
В основе реализации лежит карманная сортировка. Сначала генерируется 
крупный файл (до 2ГБайт), заполненный случайными числами от 0 до 99, 
находится максимальное значение в этом файле, создается 10 файлов-карманов, 
в каждый из которых будут записываться значения исходного файла 
соответствующего диапазона. Значения внутри каждого кармана сортируются и 
объединяются в общий файл (исходный файл, который предварительно очищен). 
В результате исходный файл содержит те же значения, только отсортированные 
по возрастанию.

