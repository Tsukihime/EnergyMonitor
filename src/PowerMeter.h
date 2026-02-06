#ifndef POWER_METER_H
#define POWER_METER_H

#include <driver/adc.h>
#include <esp_adc_cal.h>

class PowerMeter {
public:
    // Измерение мощности
    static bool measure(float voltageCalibrationFactor = 1.0f, float currentCalibrationFactor = 1.0f);

    // Геттеры для измеренных значений
    static float getVrms() { return v_rms; }
    static float getIrms() { return i_rms; }
    static float getPower() { return p; }
    static float getCosPhi() { return cos_phi; }
    static float getFrequency() { return frequency; }

private:
    // Инициализация и настройка АЦП
    static bool adc_init();

    // Конфигурация АЦП
    // get real adc freq (adc.h: ADC digital controller settings -> adc_digi_configuration_t.sample_freq_hz)
    // interval = 5000000 / (2 * 30000)    -> Частота выборки 30 кГц, interval = 83
    // ADC_FREQ = 5000000 / (interval * 2) -> Реальная частота = 30120 Hz
    static const uint32_t ADC_FREQ = 30120;
    static const uint16_t FILTER_SAMPLES = 15;
    static const uint16_t SKIP_SAMPLES = FILTER_SAMPLES + 100;
    // 0.1 sec * ADC_FREQ / ADC_CHANNELS = 1004 семпла -> 5 периодов за 100 мс
    static const uint16_t ADC_SAMPLES = SKIP_SAMPLES + 1004;
    static const uint8_t ADC_CHANNELS = 3; // Три канала: ток, напряжение и смещение
    static const uint32_t ADC_BUFFER_SIZE = ADC_SAMPLES * ADC_CHANNELS;
    static const uint32_t ADC_TIMEOUT_MS = 120;

    // Коэффициенты усиления схем
    static constexpr float DEFAULT_VOLTAGE_GAIN = 1.0f / 0.00352922676;
    static constexpr float DEFAULT_CURRENT_GAIN = 1.0f / 0.01967719515;

    // Пины и каналы АЦП
    static const adc1_channel_t channels[ADC_CHANNELS];

    // Измеренные значения
    static float v_rms;    // Среднеквадратичное напряжение
    static float i_rms;    // Среднеквадратичный ток
    static float p;        // Активная мощность
    static float cos_phi;  // cos(phi)
    static float frequency;// Частота в Гц

    // Калибровка АЦП
    static esp_adc_cal_characteristics_t adc_chars;

    // Буфер для данных АЦП
    static adc_digi_output_data_t result_buffer[ADC_BUFFER_SIZE];
};

#endif // POWER_METER_H
