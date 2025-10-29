#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

// ตั้งค่า ADC calibration
static esp_adc_cal_characteristics_t adc_chars;

// ตั้งค่าพิน GPIO สำหรับ Transistor ที่ควบคุม Buzzer
#define TRANSISTOR_PIN GPIO_NUM_18  // IO18 ต่อกับขา Base ของ 2N3904 ผ่าน R1 10k

// ตั้งค่าความถี่เสียง Buzzer
#define BEEP_DELAY_MS 100  // ระยะเวลาเสียง beep (100ms = เสียงสั้นๆ)
#define BEEP_PAUSE_MS 100  // ระยะเวลาหยุดระหว่างเสียง beep

// ตั้งค่า ADC สำหรับ LDR
#define LDR_ADC_CHANNEL ADC1_CHANNEL_7  // IO35 (GPIO35) สำหรับอ่านค่าแรงดันจาก LDR
#define LIGHT_THRESHOLD 2000  // ปรับค่าตามความเหมาะสม (0.06V ~ 500k ohm ถึง 3.26V ~ 100 ohm)
#define LIGHT_THRESHOLD 2000  // ค่าแสงที่จะทริกเกอร์ Buzzer (ปรับตามความเหมาะสม)

// ฟังก์ชันสำหรับสร้างเสียง beep
void beep(int count) {
    for(int i = 0; i < count; i++) {
        // สร้างเสียงโดยการแกว่งสัญญาณเร็วๆ
        for(int j = 0; j < 100; j++) {  // แกว่งสัญญาณ 100 ครั้งต่อ beep
            gpio_set_level(TRANSISTOR_PIN, 1);
            vTaskDelay(1);  // หน่วงเวลาสั้นๆ 1ms
            gpio_set_level(TRANSISTOR_PIN, 0);
            vTaskDelay(1);
        }
        
        // หยุดพักระหว่าง beep
        vTaskDelay(pdMS_TO_TICKS(BEEP_PAUSE_MS));
    }
}

void app_main(void)
{
    // กำหนดค่าเริ่มต้นสำหรับ GPIO ที่ควบคุมทรานซิสเตอร์
    gpio_reset_pin(TRANSISTOR_PIN);
    gpio_set_direction(TRANSISTOR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TRANSISTOR_PIN, 0); // เริ่มต้นให้ Buzzer ปิด

    // ทดสอบ Buzzer ตอนเริ่มต้น
    printf("Testing Buzzer...\n");
    beep(3);  // ส่งเสียง beep 3 ครั้ง

    // กำหนดค่าเริ่มต้นสำหรับ ADC (LDR)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc_chars);

    while (1) {
        // อ่านค่า ADC จาก LDR และแปลงเป็นมิลลิโวลต์
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(LDR_ADC_CHANNEL), &adc_chars);
        printf("LDR Voltage: %ld mV\n", voltage);

        // เช็คค่าแสงและควบคุม Buzzer
        if (voltage < LIGHT_THRESHOLD) {
            printf("Light level LOW (%ld mV) - Buzzer ON\n", voltage);
            beep(1);  // ส่งเสียง beep 1 ครั้ง
        } else {
            printf("Light level HIGH (%ld mV) - Buzzer OFF\n", voltage);
        }

        // หน่วงเวลารอบการทำงาน
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
