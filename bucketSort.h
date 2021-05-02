#pragma once
#include <algorithm>

struct bucket {
   int count; // количество элементов в корзине
   int* data; // массив элементов корзины
};

int getMax(int* arr, int n) // функция для нахождения максимального элемента массива
{
    int max = arr[0];
    for (int i = 1; i < n; i++)
        if (arr[i] > max)
            max = arr[i];
    return max;
}

int getExp(int value)
{
    int exp = 1;

    while( value > 10)
    {
        value /= 10;
        exp *= 10;
    }

    return exp;
}
// именно этот метод в проекте не используется. Реализована версия, где корзина = файл
void bucketSort(int* arr, int n)
{
    struct bucket buckets[10];
    // вычисляем значение экспоненты
    int exp = getExp(getMax(arr, n));

    for (int i = 0; i < 10; i++)
    {
        buckets[i].count = 0;
        buckets[i].data = new int[n];
    }
    for (int i = 0; i < n; i++) {
        int bi =  arr[i] / exp; // вычисляем индекс корзины
        buckets[bi].data[buckets[bi].count++] = arr[i]; // добавляем элемент в корзину (карман)
    }

    // 4: После того как все элементы распределены, мы выполняем сортировку внутри каждого кармана
    // и соединяем их в один массив по порядку их индексов.
    for (int i=0; i < 10; i++)
        std::sort(buckets[i].data, buckets[i].data + buckets[i].count);

    for (int i=0, k = 0; i < 10; i++)
        for (int j=0; j < buckets[i].count; j++, k++)
            *(arr + k) = buckets[i].data[j];
}
