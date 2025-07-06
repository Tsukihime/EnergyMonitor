#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <cstdint>

// Шаблонный класс для медианного фильтра
template <uint16_t WINDOW_SIZE>
class MedianFilter {
private:
    uint16_t buffer[WINDOW_SIZE]; // Статический буфер фиксированного размера
    int index;                    // Текущий индекс в буфере
    int count;                    // Количество заполненных элементов

    // Простая сортировка пузырьком для окна
    void sort_window(uint16_t* window, int size) {
        for (int i = 0; i < size - 1; i++) {
            for (int j = 0; j < size - i - 1; j++) {
                if (window[j] > window[j + 1]) {
                    uint16_t temp = window[j];
                    window[j] = window[j + 1];
                    window[j + 1] = temp;
                }
            }
        }
    }

public:
    // Конструктор
    MedianFilter() : index(0), count(0) {
        // Инициализация буфера нулями
        for (uint16_t i = 0; i < WINDOW_SIZE; i++) {
            buffer[i] = 0;
        }
    }

    // Функция фильтрации: принимает одно значение, возвращает отфильтрованное
    uint16_t filter(uint16_t input) {
        // Проверка, что размер окна нечётный
        static_assert(WINDOW_SIZE % 2 == 1, "WINDOW_SIZE must be odd");

        // Добавляем новое значение в буфер
        buffer[index] = input;
        index = (index + 1) % WINDOW_SIZE;
        if (count < WINDOW_SIZE) {
            count++;
        }

        // Копируем буфер для сортировки
        uint16_t temp[WINDOW_SIZE];
        for (int i = 0; i < count; i++) {
            temp[i] = buffer[i];
        }

        // Сортируем и возвращаем медиану
        sort_window(temp, count);
        return temp[count / 2];
    }
};

#endif // MEDIAN_FILTER_H
