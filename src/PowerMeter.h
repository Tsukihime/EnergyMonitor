#ifndef POWER_METER_H
#define POWER_METER_H

#include <driver/adc.h>
#include <esp_adc_cal.h>

class PowerMeter {
public:
    // Измерение мощности
    static bool measure();

    // Геттеры для измеренных значений
    static float getVrms() { return v_rms; }
    static float getIrms() { return i_rms; }
    static float getP() { return p; }
    static float getCosPhi() { return cos_phi; }

private:
    // Инициализация и настройка АЦП
    static bool adc_init();

    // Конфигурация АЦП
    static const uint16_t FILTER_SAMPLES = 15;
    static const uint16_t SKIP_SAMPLES = FILTER_SAMPLES + 100;
    static const uint16_t ADC_SAMPLES = SKIP_SAMPLES + 1000; // Количество выборок за цикл (33 мс при 30 кГц, 100 мс на 3 канала, 5 периодов)
    static const uint8_t ADC_CHANNELS = 3;    // Три канала: ток, напряжение и смещение
    static const uint32_t ADC_FREQ = 30000;   // Частота выборки 30 кГц
    static const uint32_t ADC_BUFFER_SIZE = ADC_SAMPLES * ADC_CHANNELS; // 3000 выборок
    static const uint32_t ADC_TIMEOUT_MS = 120; // Таймаут 120 мс
    static const uint8_t OFFSET_VOLTAGE_SAMPLES = 64; // Количество измерений для усреднения напряжения смещения

    // Коэффициенты усиления схем
    static constexpr float VOLTAGE_GAIN = 0.00366666666f / 1.03894334632;  // Коэффициент усиления (напряжение)
    static constexpr float CURRENT_GAIN = 2.0625f * (1.0f / 900) * 10 / 1.16462990295;

    // Пины и каналы АЦП
    static const adc1_channel_t channels[ADC_CHANNELS];
    static const adc1_channel_t OFFSET_VOLTAGE_CH;  // Канал для измерения напряжения смещения

    // Параметры АЦП
    // static constexpr float ADC_CENTER_VOLTAGE = 1.446f; // Смещение АЦП // 2048 code // to 1470 mv calibrated
    
    // Измеренные значения
    static float v_rms;    // Среднеквадратичное напряжение
    static float i_rms;    // Среднеквадратичный ток
    static float p;        // Активная мощность
    static float cos_phi;  // cos(phi)

    // Калибровка АЦП
    static esp_adc_cal_characteristics_t adc_chars;

    // Буфер для данных АЦП
    static adc_digi_output_data_t result_buffer[ADC_BUFFER_SIZE];
};

#endif // POWER_METER_H
