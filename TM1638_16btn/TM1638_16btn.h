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


#ifndef TM1638_16BTN_H
#define TM1638_16BTN_H

#include <stdint.h>
#include "driver/gpio.h"  



#define BUTTON_0   0x0010
#define BUTTON_DOT 0x0020
#define BUTTON_ZERO 0x0040
#define BUTTON_1   0x0001
#define BUTTON_2   0x0002
#define BUTTON_3   0x0004
#define BUTTON_4   0x1000
#define BUTTON_5   0x2000
#define BUTTON_6   0x4000
#define BUTTON_7   0x0100
#define BUTTON_8   0x0200
#define BUTTON_9   0x0400
#define BUTTON_A   0x0800
#define BUTTON_B   0x8000
#define BUTTON_C   0x0008
#define BUTTON_D   0x0080



#define DEBOUNCE_DELAY_MS   50      
#define DEBOUNCE_SAMPLES    3       
#define DEBOUNCE_LOG        false   



struct ButtonPressInfo {
    uint16_t button;  
    bool is_long_press;  
};


class TM1638_16btn {
public:
    

    TM1638_16btn(gpio_num_t strobe_pin, gpio_num_t clock_pin, gpio_num_t data_pin);
    void init_gpio();
    void set_brightness(uint8_t brightness);
    void clear_all();
    void show_number(int32_t number, int decimalPoint = -1);       
    void show_decimal(float number, int decimal_places = 4,bool should_round = true);  
    void show_text(const char* text);        
      
    ButtonPressInfo detect_button_press(int long_press_threshold_ms, int debounce_delay_ms);
    

private:

    uint8_t HDIGIT[8];  // Buffer of digits to display
    uint8_t TDIGIT[8];  // Transposed buffer for the display

    gpio_num_t strobe_pin;  
    gpio_num_t clock_pin;   
    gpio_num_t data_pin;    
    
    uint16_t current_keys = 0;
    
    void send_bit(uint8_t bit);
    void send_byte(uint8_t data);
    void send_command(uint8_t command);
    

    void transpose();
    void t_digit_output();

    uint16_t read_keys(bool should_log = false);  
    uint16_t debounce_keys(TM1638_16btn& display, int debounce_delay = DEBOUNCE_DELAY_MS, int samples = DEBOUNCE_SAMPLES, bool should_log = DEBOUNCE_LOG);
    void update_keys();
    bool is_button_held(uint16_t button_number, int hold_time_ms);
    bool is_button_pressed(uint16_t button_number);

    uint8_t get_segment_for_char(char c);    
    uint8_t receive_byte();    
};

#endif 
