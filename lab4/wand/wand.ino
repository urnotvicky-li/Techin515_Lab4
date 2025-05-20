/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Includes ---------------------------------------------------------------- */
#include <Techin515_Lab4_inferencing.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h> // for sin()

// === LED Pins (板上标记 -> GPIO 编号) ===
#define RED_PIN    5    // D3 -> GPIO5
#define GREEN_PIN  4    // D2 -> GPIO4
#define BLUE_PIN   10   // D10 -> GPIO10

// === Button Pin ===
#define BUTTON_PIN 3    // D1 -> GPIO3

// === Gesture capture parameters ===
#define SAMPLE_RATE_MS    10
#define FEATURE_SIZE      EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE

// === LED fade parameters ===
#define FADE_STEPS        50   // 淡入淡出分多少步
#define FADE_DELAY        20   // 每步延时 (ms)

Adafruit_MPU6050 mpu;
bool capturing = false;
unsigned long last_sample_time = 0;
int sample_count = 0;
float features[FEATURE_SIZE];

// 从 features 数组读取数据给 Edge Impulse
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

void setup() {
    Serial.begin(115200);

    // === 设置 PWM 频率和分辨率 (ESP32 Arduino Core) ===
    analogWriteFrequency(RED_PIN,   5000);
    analogWriteFrequency(GREEN_PIN, 5000);
    analogWriteFrequency(BLUE_PIN,  5000);
    analogWriteResolution(RED_PIN,   8);
    analogWriteResolution(GREEN_PIN, 8);
    analogWriteResolution(BLUE_PIN,  8);

    // === 初始化 LED 引脚 ===
    pinMode(RED_PIN,   OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN,  OUTPUT);

    // === 初始化按钮 ===
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // === 初始化 MPU6050 ===
    Serial.println("Initializing MPU6050...");
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) delay(10);
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 initialized. Press button to start gesture recognition.");
}

void loop() {
    // 检测按钮按下（低电平触发）
    if (digitalRead(BUTTON_PIN) == LOW && !capturing) {
        Serial.println("Button pressed! Starting gesture capture...");
        sample_count = 0;
        capturing = true;
        last_sample_time = millis();
        delay(200);  // 去抖
    }

    if (capturing) {
        capture_accelerometer_data();
    }
}

void capture_accelerometer_data() {
    if (millis() - last_sample_time >= SAMPLE_RATE_MS) {
        last_sample_time = millis();

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        if (sample_count < FEATURE_SIZE / 3) {
            int idx = sample_count * 3;
            features[idx    ] = a.acceleration.x;
            features[idx + 1] = a.acceleration.y;
            features[idx + 2] = a.acceleration.z;
            sample_count++;
        }

        if (sample_count >= FEATURE_SIZE / 3) {
            capturing = false;
            Serial.println("Capture complete");
            run_inference();
        }
    }
}

void run_inference() {
    ei_impulse_result_t result = { 0 };
    signal_t features_signal;
    features_signal.total_length = FEATURE_SIZE;
    features_signal.get_data = &raw_feature_get_data;

    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
    if (res != EI_IMPULSE_OK) {
        Serial.print("Classifier failed: "); Serial.println(res);
        return;
    }

    // 打印所有分类概率
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        Serial.printf("  %s: %.2f%%\n",
            ei_classifier_inferencing_categories[i],
            result.classification[i].value * 100);
    }

    // 找到概率最高的标签
    float max_val = 0;
    int max_idx = -1;
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > max_val) {
            max_val = result.classification[i].value;
            max_idx = i;
        }
    }
    if (max_idx < 0) return;

    // 去除空白并大写
    String gesture = ei_classifier_inferencing_categories[max_idx];
    gesture.trim();
    gesture.toUpperCase();
    Serial.printf("Prediction: %s (%.2f%%)\n", gesture.c_str(), max_val * 100);

    // 根据手势调用淡入淡出
    if (gesture.equals("Z")) {
        Serial.println("  → fading RED");
        fadeLED(RED_PIN);
    } else if (gesture.equals("O")) {
        Serial.println("  → fading BLUE");
        fadeLED(BLUE_PIN);
    } else if (gesture.equals("V")) {
        Serial.println("  → fading GREEN");
        fadeLED(GREEN_PIN);
    } else {
        Serial.println("  → no matching gesture");
    }
}

// 平滑淡入淡出函数
void fadeLED(int pin) {
    for (int i = 0; i <= FADE_STEPS; i++) {
        float bell = sin(PI * i / FADE_STEPS);
        int value = (int)(bell * 255);
        analogWrite(pin, value);
        delay(FADE_DELAY);
    }
    delay(200);
    for (int i = FADE_STEPS; i >= 0; i--) {
        float bell = sin(PI * i / FADE_STEPS);
        int value = (int)(bell * 255);
        analogWrite(pin, value);
        delay(FADE_DELAY);
    }
    analogWrite(pin, 0);
}