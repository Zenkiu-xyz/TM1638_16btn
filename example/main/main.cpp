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


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TM1638_16btn.h"
#include "esp_log.h"
#include "string.h"

#define DEBOUNCE_DELAY_MS   500      
#define LONG_PRESS_THRESHOLD_MS 20

extern "C" void app_main() {

    esp_log_level_set("*", ESP_LOG_NONE);
    
    TM1638_16btn display(GPIO_NUM_19, GPIO_NUM_18, GPIO_NUM_21); 

    ButtonPressInfo press_info;
    
    display.init_gpio();  
    display.clear_all();  
    display.set_brightness(15);  // from 0 to 15

    
    //display.show_decimal(12.34536);  
    //vTaskDelay(500);
    //display.clear_all(); 
    //vTaskDelay(500);
    
    //display.show_number(656);
    //vTaskDelay(500);
    display.show_text("HELLO ");

    //display.show_text("hola");

    //display.show_decimal(12.34536, 4, true);   // using true, show number rounded "012.3454"


    vTaskDelay(2000 / portTICK_PERIOD_MS);
    display.show_number(656,1); // show "65.1"
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display.show_number(656,0); // show "656"
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    display.show_text("INPUT");

    //uint16_t last_keys = 0;

    while (true) {
        
         press_info = display.detect_button_press(DEBOUNCE_DELAY_MS,LONG_PRESS_THRESHOLD_MS);  
        
        // Check if a button is pressed
        if (press_info.button == 0) {
            vTaskDelay(100 / portTICK_PERIOD_MS);  // If no button is pressed, perform a delay and continue
            continue;
        }

        
        
        char display_text[3] = {0};

        // Assign the text to display depending on the button pressed
        if (press_info.button & BUTTON_0) {
            strcpy(display_text, press_info.is_long_press ? "00" : "0");
        } else if (press_info.button & BUTTON_1) {
            strcpy(display_text, press_info.is_long_press ? "11" : "1");
        } else if (press_info.button & BUTTON_2) {
            strcpy(display_text, press_info.is_long_press ? "22" : "2");
        } else if (press_info.button & BUTTON_3) {
            strcpy(display_text, press_info.is_long_press ? "33" : "3");
        } else if (press_info.button & BUTTON_4) {
            strcpy(display_text, press_info.is_long_press ? "44" : "4");
        } else if (press_info.button & BUTTON_5) {
            strcpy(display_text, press_info.is_long_press ? "55" : "5");
        } else if (press_info.button & BUTTON_6) {
            strcpy(display_text, press_info.is_long_press ? "66" : "6");
        } else if (press_info.button & BUTTON_7) {
            strcpy(display_text, press_info.is_long_press ? "77" : "7");
        } else if (press_info.button & BUTTON_8) {
            strcpy(display_text, press_info.is_long_press ? "88" : "8");
        } else if (press_info.button & BUTTON_9) {
            strcpy(display_text, press_info.is_long_press ? "99" : "9");
        } else if (press_info.button & BUTTON_A) {
            strcpy(display_text, press_info.is_long_press ? "AA" : "A");
        } else if (press_info.button & BUTTON_B) {
            strcpy(display_text, press_info.is_long_press ? "BB" : "B");
        } else if (press_info.button & BUTTON_C) {
            strcpy(display_text, press_info.is_long_press ? "CC" : "C");
        } else if (press_info.button & BUTTON_D) {
            strcpy(display_text, press_info.is_long_press ? "DD" : "D");
        } else if (press_info.button & BUTTON_DOT) {
            strcpy(display_text, press_info.is_long_press ? ".." : ".");
        } else if (press_info.button & BUTTON_ZERO) {
            // If the zero button is pressed, clear the display
            display.clear_all();
            continue;  
        }

        
        display.show_text(display_text);


        vTaskDelay(200 / portTICK_PERIOD_MS);  
    }
}


 