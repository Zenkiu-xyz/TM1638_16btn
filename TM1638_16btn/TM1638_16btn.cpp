// TM1638_16btn
// https://github.com/Zenkiu-xyz/TM1638_16btn
//
// MIT License
//
// Copyright (c) 2024 Zenkiu.xyz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "TM1638_16btn.h"
#include "driver/gpio.h"
#include <string.h>  
#include <stdio.h>  
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>  


// Define the commands for TM1638
#define CMD1  0x44  // Fixed address writing command
#define CMD2  0x42  // Read keys command
#define CMD3  0x89  // Command to turn ON and adjust the brightness


// Definir valores para los números y letras
const uint8_t char_to_segment[36] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // B
    0b00111001, // C
    0b01011110, // D
    0b01111001, // E
    0b01110001, // F
    0b01111101, // G
    0b01110110, // H
    0b00110000, // I
    0b00011110, // J
    0b01110110, // K (Mapped as H)
    0b00111000, // L
    0b00010101, // M (Mapped as A)
    0b01010100, // N
    0b00111111, // O
    0b01110011, // P
    0b01100111, // Q (Mapped as O)
    0b01010000, // R (Mapped as P)
    0b01101101, // S
    0b01111000, // T
    0b00111110, // U
    0b00111110, // V (Mapped as U)
    0b00101010, // W
    0b01110110, // X (Mapped as H)
    0b01101110, // Y
    0b01011011  // Z (Mapped as 2)
};


TM1638_16btn::TM1638_16btn(gpio_num_t strobe_pin, gpio_num_t clock_pin, gpio_num_t data_pin)
    : strobe_pin(strobe_pin), clock_pin(clock_pin), data_pin(data_pin)  
{
    for (int i = 0; i < 8; i++) {
        HDIGIT[i] = 0x00;
    }
}

void TM1638_16btn::init_gpio() {
    gpio_set_direction(strobe_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(clock_pin, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(data_pin, GPIO_PULLUP_ONLY);  
    gpio_set_direction(data_pin, GPIO_MODE_INPUT_OUTPUT_OD);
}

void TM1638_16btn::send_bit(uint8_t bit) {
    gpio_set_level(data_pin, bit);
    gpio_set_level(clock_pin, 1);
    gpio_set_level(clock_pin, 0);
}

void TM1638_16btn::send_byte(uint8_t data) {
    gpio_set_direction(data_pin, GPIO_MODE_OUTPUT); 
    esp_rom_delay_us(5);
    for (int i = 0; i < 8; i++) {
        send_bit(data & 0x01);  
        data >>= 1;
    }
}


void TM1638_16btn::send_command(uint8_t command) {
    gpio_set_level(strobe_pin, 0);
    send_byte(command);
    gpio_set_level(strobe_pin, 1);
}

void TM1638_16btn::clear_all() {
    
    for (int i = 0; i < 8; i++) {
        HDIGIT[i] = 0x00;  
    }

    transpose();

    for (uint8_t i = 0; i < 8; i++) {
        gpio_set_level(strobe_pin, 0);
        send_byte(0xC0 + (i * 2));  
        send_byte(TDIGIT[i]);       
        gpio_set_level(strobe_pin, 1);
    }
}

void TM1638_16btn::set_brightness(uint8_t brightness) {
    
    if (brightness > 15) {
        brightness = 15;
    }
   
    uint8_t command = 0x88 | brightness;
    send_command(command);  
}

void TM1638_16btn::transpose() {
    uint8_t m = 0x01;
    for (uint8_t i = 0; i < 8; i++) {
        TDIGIT[i] = 0;
        for (uint8_t j = 0; j < 8; j++) {
            if (HDIGIT[7 - j] & m) {  
                TDIGIT[i] |= (1 << j);
            }
        }
        m <<= 1;
    }
}

void TM1638_16btn::t_digit_output() {
    send_command(CMD1);
    for (uint8_t j = 0; j < 8; j++) {
        gpio_set_level(strobe_pin, 0);
        send_byte(0xC0 + (j * 2));  
        send_byte(TDIGIT[j]);  
        gpio_set_level(strobe_pin, 1);
    }
    
}

uint8_t TM1638_16btn::get_segment_for_char(char c) {
    if (c >= '0' && c <= '9') {
        return char_to_segment[c - '0'];
    } else if (c >= 'A' && c <= 'Z') {
        return char_to_segment[c - 'A' + 10];
    } else {
        return 0x00;  
    }
}

void TM1638_16btn::show_number(int32_t number, int decimalPoint) {
    clear_all();
    
    char numStr[9];  
    
    
    if (number > 99999999) {
        number = 99999999;
    } else if (number < -9999999) {
        number = -9999999;
    }

    snprintf(numStr, sizeof(numStr), "%08" PRId32, number);

    int firstNonZero = 0;
    bool is_negative = (number < 0);
  
    if (is_negative) {
        number = -number;
    }

    for (int i = 0; i < 8; i++) {
        if (numStr[i] != '0') {
            firstNonZero = i;
            break;
        }
    }

    if (number == 0) {
        firstNonZero = 7 - decimalPoint;
    }

    for (int i = 0; i < 8; i++) {
        if (i < firstNonZero) {
            HDIGIT[i] = 0x00;  
        } else {
            HDIGIT[i] = get_segment_for_char(numStr[i]);
        }

        if (7 - i == decimalPoint) {
            HDIGIT[i] |= 0x80;  
        }
    }

    if (is_negative) {
        if (firstNonZero > 0) {
            HDIGIT[firstNonZero - 1] = 0b01000000;  
        } else {
            
            for (int i = 7; i > 0; i--) {
                HDIGIT[i] = HDIGIT[i-1];
            }
            HDIGIT[0] = 0b01000000;  
        }
    }

    transpose();
    t_digit_output();
}

void TM1638_16btn::show_decimal(float number, int decimal_places, bool should_round) {
    clear_all();

    if (decimal_places > 7) {
        decimal_places = 7;
    }

    char format_str[16];
    snprintf(format_str, sizeof(format_str), "%%.%df", decimal_places);  

    char number_str[12];  

    if (should_round) {
        snprintf(number_str, sizeof(number_str), format_str, number);
    } else {

        float trunc_factor = pow(10, decimal_places);
        number = static_cast<int>(number * trunc_factor) / trunc_factor;
        snprintf(number_str, sizeof(number_str), format_str, number);
    }
  
    char* dot = strchr(number_str, '.');  
    if (dot != nullptr) {
        *dot = '\0';  

        
        int integer_part = atoi(number_str);  
        int integer_places = 8 - decimal_places;  
        for (int i = integer_places - 1; i >= 0; i--) {
            HDIGIT[i] = get_segment_for_char('0' + (integer_part % 10));
            integer_part /= 10;
        }
       
        HDIGIT[integer_places - 1] |= 0x80;
        
        int decimal_part = atoi(dot + 1);  
        for (int i = 7; i >= integer_places; i--) {
            HDIGIT[i] = get_segment_for_char('0' + (decimal_part % 10));
            decimal_part /= 10;
        }
    }
    
    transpose();
    t_digit_output();
}






void TM1638_16btn::show_text(const char* text) {
    
    int length = strlen(text);
    if (length > 8) {
        length = 8;  
    }
  
    for (int i = 0; i < length; i++) {
        char current_char = text[i];
        
        if (current_char >= '0' && current_char <= '9') {
            HDIGIT[i] = char_to_segment[current_char - '0'];  
        } else if (current_char >= 'A' && current_char <= 'Z') {
            HDIGIT[i] = char_to_segment[current_char - 'A' + 10];  
        } else if (current_char == '.'){
            HDIGIT[i] |= 0x80;  
        }
            else {
            HDIGIT[i] = 0x00;  
        }
    }
   
    for (int i = length; i < 8; i++) {
        HDIGIT[i] = 0x00;  
    }
   
    transpose();
   
    t_digit_output();
}

uint16_t TM1638_16btn::read_keys(bool should_log){
    uint16_t keys = 0;

    gpio_set_level(strobe_pin, 0);  
    send_byte(CMD2);                
    esp_rom_delay_us(5);            

    uint8_t received_bytes[4] = {0};  

    for (int i = 0; i < 4; i++) {
        received_bytes[i] = receive_byte();
    }

    if (received_bytes[2] & 0x02) keys |= BUTTON_0;    
    if (received_bytes[2] & 0x20) keys |= BUTTON_DOT;  
    if (received_bytes[3] & 0x02) keys |= BUTTON_ZERO; 
    if (received_bytes[0] & 0x02) keys |= BUTTON_1;    
    if (received_bytes[0] & 0x20) keys |= BUTTON_2;    
    if (received_bytes[1] & 0x02) keys |= BUTTON_3;    
    if (received_bytes[2] & 0x04) keys |= BUTTON_4;    
    if (received_bytes[2] & 0x40) keys |= BUTTON_5;    
    if (received_bytes[3] & 0x04) keys |= BUTTON_6;    
    if (received_bytes[0] & 0x04) keys |= BUTTON_7;    
    if (received_bytes[0] & 0x40) keys |= BUTTON_8;    
    if (received_bytes[1] & 0x04) keys |= BUTTON_9;    
    if (received_bytes[1] & 0x40) keys |= BUTTON_A;    
    if (received_bytes[3] & 0x40) keys |= BUTTON_B;    
    if (received_bytes[1] & 0x20) keys |= BUTTON_C;    
    if (received_bytes[3] & 0x20) keys |= BUTTON_D;    

    if (should_log && (received_bytes[0] != 0x00 || received_bytes[1] != 0x00 ||
     received_bytes[2] != 0x00 || received_bytes[3] != 0x00)) {
        
        for (int i = 0; i < 4; i++) {
            ESP_LOGI("TAG", "Byte %d received: 0x%02X", i + 1, received_bytes[i]);
        }
        ESP_LOGI("TAG", "Pressed buttons: 0x%04X", keys);
        
        char bin_str[17];  
        for (int i = 0; i < 16; i++) {
            bin_str[15 - i] = (keys & (1 << i)) ? '1' : '0';
        }
        bin_str[16] = '\0';  

        ESP_LOGI("TAG", "Pressed buttons (binary): %s", bin_str);
    }

    gpio_set_level(strobe_pin, 1);  
    esp_rom_delay_us(5);            

    return keys;
}

uint8_t TM1638_16btn::receive_byte() {
    uint8_t data = 0;
    gpio_set_direction(data_pin, GPIO_MODE_INPUT);  
    for (int i = 0; i < 8; i++) {
        gpio_set_level(clock_pin, 1);  
        esp_rom_delay_us(5);           
        data >>= 1;
        if (gpio_get_level(data_pin)) {
            data |= 0x80;  
        }
        gpio_set_level(clock_pin, 0);  
        esp_rom_delay_us(5);           
    }
    return data;
}

uint16_t TM1638_16btn::debounce_keys(TM1638_16btn& _16btn, int debounce_delay, int samples, 
bool should_log) {
    
    uint16_t stable_keys = 0;
    int consistent_count = 0;

    for (int i = 0; i < samples; i++) {
        uint16_t current_keys = _16btn.read_keys(should_log);  
        
        if (current_keys == stable_keys) {
            consistent_count++;
        } else {
            stable_keys = current_keys;
            consistent_count = 1;  
        }
       
        vTaskDelay(debounce_delay / portTICK_PERIOD_MS);
    }
   
    return (consistent_count >= (samples / 2)) ? stable_keys : 0;
}

bool TM1638_16btn::is_button_pressed(uint16_t button_number) {

    switch (button_number) {
       
        case BUTTON_0: return current_keys & BUTTON_0;
        case BUTTON_1: return current_keys & BUTTON_1;
        case BUTTON_2: return current_keys & BUTTON_2;
        case BUTTON_3: return current_keys & BUTTON_3;
        case BUTTON_4: return current_keys & BUTTON_4;
        case BUTTON_5: return current_keys & BUTTON_5;
        case BUTTON_6: return current_keys & BUTTON_6;
        case BUTTON_7: return current_keys & BUTTON_7;
        case BUTTON_8: return current_keys & BUTTON_8;
        case BUTTON_9: return current_keys & BUTTON_9;
        case BUTTON_A: return current_keys & BUTTON_A;
        case BUTTON_B: return current_keys & BUTTON_B;
        case BUTTON_C: return current_keys & BUTTON_C;
        case BUTTON_D: return current_keys & BUTTON_D;
        case BUTTON_DOT: return current_keys & BUTTON_DOT;  
        case BUTTON_ZERO: return current_keys & BUTTON_ZERO;  
        default: return false;   
    }
}

void TM1638_16btn::update_keys() {
    current_keys = debounce_keys(*this, DEBOUNCE_DELAY_MS, DEBOUNCE_SAMPLES, DEBOUNCE_LOG);
}

bool TM1638_16btn::is_button_held(uint16_t button_number, int hold_time_ms) {
    int elapsed_time = 0;

    update_keys();  
    uint16_t stable_keys = current_keys;  

    if (!(stable_keys & button_number)) {
        return false;
    }

    ESP_LOGI("TAG", "Button initially pressed, waiting for long press...");

    while (stable_keys & button_number) {  
        vTaskDelay(50 / portTICK_PERIOD_MS);  
        elapsed_time += 50;

        if (elapsed_time >= hold_time_ms) {
            ESP_LOGI("TAG", "Long press detected on the button");
            return true;  
        }

        update_keys();
        stable_keys = current_keys;  
        ESP_LOGI("TAG", "Checking press...");
    }

    ESP_LOGI("TAG", "Released with return");
    return false;  
}

ButtonPressInfo TM1638_16btn::detect_button_press(int long_press_threshold_ms, int debounce_delay_ms) {
    static bool last_was_long_press = false; 
    
    if (last_was_long_press) {
        vTaskDelay(150 / portTICK_PERIOD_MS);  
        last_was_long_press = false;  
    }

    ButtonPressInfo press_info = {0, false};  

    uint16_t stable_keys = debounce_keys(*this, debounce_delay_ms, DEBOUNCE_SAMPLES, false);
    
    if (stable_keys != 0) {
        
        ESP_LOGI("TAG", "Button pressed, waiting to determine if it is a short or long press...");

        int elapsed_time = 0;
        uint16_t pressed_button = stable_keys;  

        while (stable_keys == pressed_button) {
            vTaskDelay(50 / portTICK_PERIOD_MS);  
            elapsed_time += 50;

            if (elapsed_time >= long_press_threshold_ms) {
                press_info.button = pressed_button;
                press_info.is_long_press = true;
                ESP_LOGI("TAG", "Long press detected");
                last_was_long_press = true;
                return press_info;
            }

            stable_keys = debounce_keys(*this, debounce_delay_ms, DEBOUNCE_SAMPLES, false);
        }

        press_info.button = pressed_button;
        press_info.is_long_press = false;
        ESP_LOGI("TAG", "Short press detected");
        return press_info;
    }

    return press_info;  
}



