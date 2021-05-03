#include <iostream>
#include <fstream>
#include <cstdlib> // для получения рандомных чисел
#include <ctime>
#include <string>
#include <memory>
#include <chrono>
#include "bucketSort.h"

// Прочитать массив из файла
void read_arr(const std::string& filename, int* arr, const int n)
{
    std::fstream fs;

    fs.open(filename, std::fstream::in);
    if(fs.is_open()) // проверяем, что файл успешно открыт
    {
       for (int i = 0; i < n; i++)
           fs >> arr[i];

       fs.close(); // закрываем файл
    }
}
// Сгенерим большой файл в STEPS шагов. Метод возвращает суммарный размер int-массива, который в этот файл был записан
int GenerateBigFile(const std::string& SRC_FILENAME)
{
    constexpr int BIG_INT_SIZE {2000000000 / sizeof(int)}; // размер массива int-чисел, который будет занимать до 2 ГБайт.

    std::fstream fs;
    fs.open(SRC_FILENAME, std::fstream::out);
    if (fs.is_open())
    {
        fs << ""; // очистили файл
        fs.close();
    }
    fs.open(SRC_FILENAME, std::fstream::app);

    srand(time(nullptr)); // используем текущее время, чтобы сгенерировать рандомные значения
    int lef_border = 0;
    int range_len = 100; // правая граница = range_len + left_border
    int rand_value {0};

    for (int i=0; i < BIG_INT_SIZE; i++)
    {
        rand_value = lef_border + rand() % range_len; // генерируем число в указанном диапазоне и записываем в массив

        fs << rand_value << ' '; // записываем значения через пробел
    }
    fs << '\n';
    if (fs.is_open())
        fs.close();

    std::cout <<"BigIntSize = " << BIG_INT_SIZE << std::endl;

    return BIG_INT_SIZE;
}
// Найти максимальное значение в файле
int GetMax(const std::string& filename)
{
    int max {0};
    std::fstream fs;
    if (!fs.is_open())
        fs.open(filename, std::ios_base::in);
    int value {0};
    while (!fs.eof())
    {
        fs >> value;

        if (value > max)
            max = value;
    }
    fs.close();
    return max;
}
// Распихиваем все значения из файла filename по карманам (файлам).
// Возвращаем массив размером buckets_cnt, храняющий количества элементов каждого кармана
std::unique_ptr<int[]> ShoveInBuckets(const std::string& filename,
                                      const int exp, // экспонента
                                      const std::string bucket_names[], const int buckets_cnt)
{
    std::unique_ptr<int[]> counts{new int[buckets_cnt]};
    std::fill(counts.get(), counts.get() + buckets_cnt, 0); // обнулить все элементы
    std::fstream fs_in; // поток для чтения исходного файла
    if (!fs_in.is_open())
        fs_in.open(filename, std::ios_base::in);

    std::unique_ptr<std::fstream[]> fs_out_arr{new std::fstream[buckets_cnt]}; // массив потоков для записи
    for (int i=0; i < buckets_cnt; i++)
    { // открыть все "карманные" файлы для записи:
        if (!fs_out_arr[i].is_open())
            fs_out_arr[i].open(bucket_names[i], std::ios_base::out);
    }

    int value {0};
    int bi {0}; // индекс кармана
    while (!fs_in.eof())
    {
        fs_in >> value;
        bi = value / exp; // вычисляем индекс корзины
        counts[bi]++; // подсчет элементов в этой корзине (кармане)
        fs_out_arr[bi] << value << ' ';
    }
    fs_in.close(); // закрыть исходный файл
    for (int i=0; i < buckets_cnt; i++) // закрыть "карманные" файлы
    {
        fs_out_arr[i] << '\n';
        fs_out_arr[i].close();
    }
    for (int i=0; i < buckets_cnt; i++)
        std::cout << "counts[" << i << "] = " << counts[i] << std::endl;
    return counts;
}
// отсортировать элементы в каждом кармане и объединить их, записав в исходный файл (его можно стереть и перезаписать заново,
//                                                                                   он больше не нужен):
void SortBucketsAndMerge(const std::string* bucket_names, const int buckets_cnt,
                         const int* counts,  // counts - массив количеств элементов в каждом кармане
                         const std::string& src_filename)
{
    std::fstream out;
    if (!out.is_open())
        out.open(src_filename, std::ios_base::out);

    for (int bi=0; bi < buckets_cnt; bi++)
    {
        std::cout << ": Sorting and writing from bucket " << bi << std::endl;
        std::unique_ptr<int[]> bucket_arr{new int[counts[bi]]}; // массив, который будет прочитан из файла-кармана
        read_arr(bucket_names[bi], bucket_arr.get(), counts[bi]);
        // сортируем элементы из этого кармана, с замером времени:
        auto start = std::chrono::system_clock::now();  // измерение времени сортировки каждого кармана
        std::sort(bucket_arr.get(), bucket_arr.get() + counts[bi]); // скорость: O(n*log2(n)), согласно документации: https://en.cppreference.com/w/cpp/algorithm/sort
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        std::cout << "sorting took " << elapsed_seconds.count() << " sec\n";

        // Запись в исходный файл:
        for (int i=0; i < counts[bi]; i++)
            out << bucket_arr[i] << ' ';
    }
    out << '\n';
    out.close();
}
// По условию задачи, наиболее ценный ресурс - это память (раз мы не можем прочитать враз 2 ГБайта),
// значит придётся пожертвовать вычислительным временем процессора, чтобы не переполнять ОЗУ.
int main()
{
    // 1. Сгенерировать большой файл.
    const std::string SRC_FILENAME {"source_file.txt"}; // имя большого файла
    std::cout << "Creating large file..." << std::endl;
    GenerateBigFile(SRC_FILENAME); // возвращаемое значение - суммарный int-ый размер массива, записанного в файл
    // 2. Поиск максимального значения:
    std::cout << "Searching maximum value:";
    std::cout.flush();
    int max = GetMax(SRC_FILENAME); // нашли максимальное значение в файле - нужно для разбивки на карманы
    std::cout << " success: max = " << max << std::endl;
    // 3. Создаём 10 карманов = 10 файлов, каждый из которых будет хранить значения своего диапазона:
    constexpr int BUCKETS_COUNT {10};
    std::string bucket_names[BUCKETS_COUNT];
    for (int i=0; i < BUCKETS_COUNT; i++)
    {
        bucket_names[i] = "bucket_";
        bucket_names[i] += std::to_string(i);
    }
    // вычисляем значение экспоненты
    int exp = getExp(max);
    // 4. Формируем "карманные" файлы - распихиваем элементы исходного файла по карманам-диапазонам:
    std::cout << "Shoving in buckets..." << std::endl;
    auto counts = ShoveInBuckets(SRC_FILENAME, exp, bucket_names, BUCKETS_COUNT);
    // 5. Сортируем элементы каждого кармана с помощью std::sort (время O(n*log(n), согласно документации):
    std::cout << "Sorting and merging: " << std::endl;
    SortBucketsAndMerge(bucket_names, BUCKETS_COUNT, counts.get(), SRC_FILENAME);
    std::cout << " success" << std::endl;
    // Удалить файлы-карманы, они больше не нужны:
    for (int i=0; i < BUCKETS_COUNT; i++)
        std::remove(bucket_names[i].c_str());

    return 0;
}
