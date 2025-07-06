#include "PowerMeter.h"
#include <esp_log.h>
#include <cmath>
#include <cstring>

#include "MedianFilter.h"

static const char *TAG = "PowerMeter";

// Инициализация статических членов
const adc1_channel_t PowerMeter::channels[PowerMeter::ADC_CHANNELS] = {
    ADC1_CHANNEL_1, // GPIO2 (ток)
    ADC1_CHANNEL_3, // GPIO3 (напряжение)
    ADC1_CHANNEL_4  // GPIO4 (смещение)
};

float PowerMeter::v_rms = 0.0f;
float PowerMeter::i_rms = 0.0f;
float PowerMeter::p = 0.0f;
float PowerMeter::cos_phi = 0.0f;
esp_adc_cal_characteristics_t PowerMeter::adc_chars = {};
adc_digi_output_data_t PowerMeter::result_buffer[PowerMeter::ADC_BUFFER_SIZE];

bool PowerMeter::adc_init() {
    // Калибровка АЦП // Two Point value used for characterization
    esp_err_t ret = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);

    // Настройка непрерывного режима АЦП
    adc_digi_init_config_t adc_dma_config = {
        .max_store_buf_size = ADC_BUFFER_SIZE * sizeof(adc_digi_output_data_t),
        .conv_num_each_intr = ADC_BUFFER_SIZE,
        .adc1_chan_mask = BIT(channels[0]) | BIT(channels[1]),
        .adc2_chan_mask = 0,
    };

    ret = adc_digi_initialize(&adc_dma_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ADC: %s", esp_err_to_name(ret));
        return false;
    }

    adc_digi_pattern_config_t adc_pattern[ADC_CHANNELS];
    for (int i = 0; i < ADC_CHANNELS; i++) {
        adc_pattern[i].atten = ADC_ATTEN_DB_12;
        adc_pattern[i].channel = channels[i];
        adc_pattern[i].unit = 0;
        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
    }

    adc_digi_configuration_t digi_cfg = {
        .conv_limit_en = false,
        .conv_limit_num = 0,
        .pattern_num = ADC_CHANNELS,
        .adc_pattern = adc_pattern,
        .sample_freq_hz = ADC_FREQ,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    };

    ret = adc_digi_controller_configure(&digi_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC controller: %s", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "ADC initialized successfully");
    return true;
}

bool PowerMeter::measure() {
    MedianFilter<FILTER_SAMPLES> filterVoltage;
    MedianFilter<FILTER_SAMPLES> filterCurrent;
    MedianFilter<FILTER_SAMPLES> filterOffset;

    // Инициализация АЦП
    if (!adc_init()) {
        ESP_LOGE(TAG, "Measurement failed: ADC not initialized");
        return false;
    }

    v_rms = i_rms = p = cos_phi = 0.0f;
    float sum_V = 0.0f, sum_I = 0.0f, sum_P = 0.0f;

    // Запуск АЦП
    esp_err_t ret = adc_digi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start ADC: %s", esp_err_to_name(ret));
        return false;
    }

    int samples_needed = ADC_BUFFER_SIZE;
    int samples_collected = 0;

    // Чтение данных
    while (samples_needed > 0) {
        uint32_t ret_num = 0;
        ret = adc_digi_read_bytes((uint8_t *)&result_buffer[samples_collected],
                                 samples_needed * sizeof(adc_digi_output_data_t),
                                 &ret_num, ADC_TIMEOUT_MS);
        if (ret != ESP_OK || ret_num == 0) {
            ESP_LOGE(TAG, "ADC read error: %s", esp_err_to_name(ret));
            adc_digi_stop();
            adc_digi_deinitialize();
            return false;
        }
        samples_collected += ret_num / sizeof(adc_digi_output_data_t);
        samples_needed -= ret_num / sizeof(adc_digi_output_data_t);
    }

    // Остановка АЦП
    ret = adc_digi_stop();
    assert(ret == ESP_OK);
    ret = adc_digi_deinitialize();
    assert(ret == ESP_OK);

    // Обработка данных
    for (int i = 0; i < ADC_BUFFER_SIZE; i += ADC_CHANNELS) {
        adc_digi_output_data_t *current_data = &result_buffer[i];
        adc_digi_output_data_t *voltage_data = &result_buffer[i + 1];
        adc_digi_output_data_t *offset_data = &result_buffer[i + 2];

        // Присваиваем данные проверяя порядок каналов
        uint32_t current_raw, voltage_raw, offset_raw;
        if (current_data->type2.channel == channels[0]) {
            // Порядок правильный: ток, напряжение, смещение
            current_raw = current_data->type2.data;
            voltage_raw = voltage_data->type2.data;
            offset_raw = offset_data->type2.data;
        } else {
            ESP_LOGE(TAG, "Sample order error!");
            return false;
        }

        voltage_raw = filterVoltage.filter(voltage_raw);
        current_raw = filterCurrent.filter(current_raw);
        offset_raw = filterOffset.filter(offset_raw) + 2;

        if(i == ADC_CHANNELS * SKIP_SAMPLES) {
            sum_V = 0.0f, sum_I = 0.0f, sum_P = 0.0f;
        }

        //ESP_LOGE(TAG, "%u, %u, %u", voltage_raw, current_raw, offset_raw);

        // Переводим сырые данные в напряжение с учётом калибровки (в милливольтах)
        float current_with_offset = esp_adc_cal_raw_to_voltage(current_raw, &adc_chars);
        float voltage_with_offset = esp_adc_cal_raw_to_voltage(voltage_raw, &adc_chars);
        float offset = esp_adc_cal_raw_to_voltage(offset_raw, &adc_chars);

        // Убираем смещение и учитываем коэффициенты усиления
        float current = (current_with_offset - offset) / 1000.0f / CURRENT_GAIN; // Переводим в амперы
        float voltage = (voltage_with_offset - offset) / 1000.0f / VOLTAGE_GAIN; // Переводим в вольты

        sum_I += current * current;
        sum_V += voltage * voltage;
        sum_P += current * voltage;
    }

    // Расчёт RMS и cos(phi)
    v_rms = std::sqrt(sum_V / ADC_SAMPLES);
    i_rms = std::sqrt(sum_I / ADC_SAMPLES);
    p = sum_P / ADC_SAMPLES;
    cos_phi = (v_rms * i_rms > 0) ? p / (v_rms * i_rms) : 0.0f;

    return true;
}
