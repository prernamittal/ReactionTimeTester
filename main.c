#include "LPC17xx.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LCD_RS_PIN 2
#define LCD_E_PIN 4
#define LCD_D4_PIN 5
#define LED_PIN 1
#define BUTTON_PIN 2

unsigned int i;

void lcd_write_nibble(uint8_t nibble) { 
    LPC_GPIO1->FIOSET |= (1 << LCD_E_PIN);  
    LPC_GPIO1->FIOCLR |= (0x0F << LCD_D4_PIN); 
    LPC_GPIO1->FIOSET |= ((nibble & 0x0F) << LCD_D4_PIN); 
    LPC_GPIO1->FIOCLR |= (1 << LCD_E_PIN); 
}

void lcd_data(char data) {
    LPC_GPIO1->FIOSET |= (1 << LCD_RS_PIN);
    lcd_write_nibble((data >> 4) & 0x0F); //send upper nibble first
    lcd_write_nibble(data & 0x0F); //send lower nibble
    for (i = 0; i < 40000; i++);//delay
}

static void lcd_command(uint8_t command) {
    LPC_GPIO1->FIOCLR = (1 << LCD_RS_PIN); 
    lcd_write_nibble(command >> 4);//send upper nibble first
    lcd_write_nibble(command);//send lower nibble
    for (i = 0; i < 40000; i++);
}

void lcd_init(uint8_t rs_pin, uint8_t e_pin, uint8_t d4_pin) {//initialize the LCD
    LPC_GPIO1->FIODIR |= (1 << rs_pin) | (1 << e_pin) | (1 << d4_pin);//set pins as output
    lcd_command(0x28);//4-bit mode, 2 lines, 5x7 font
    lcd_command(0x08);//display off, cursor off, blinking off
    lcd_command(0x01);//clear display
    lcd_command(0x06);//increment cursor (shift cursor to right)
    lcd_command(0x0C);//display on, cursor off, blinking off
}

void lcd_clear() {//clear the LCD
    lcd_command(0x01);//clear display
    lcd_command(0x02);//return cursor to home
}

void lcd_gotoxy(uint8_t x, uint8_t y) {//move cursor to position (x,y)
    uint8_t addr = 0x40 * y + x;//calculate address
    lcd_command(0x80 | addr);//set DDRAM address
}

void lcd_puts(const char* str) {//print string on LCD
    for (i = 0; str[i]; i++) {
        lcd_data(str[i]);
    }
}

int main(void) {
    unsigned int start_time, end_time, reaction_time;
    char reaction_str[16];
    LPC_GPIO1->FIODIR |= (1 << LED_PIN);//set LED pin as output
    LPC_GPIO2->FIODIR &= ~(1 << BUTTON_PIN);//set button pin as input
    lcd_init(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN);

    while (1) {
        uint32_t delay = rand() % 5 + 1;//random delay between 1 and 5 seconds
        LPC_GPIO1->FIOSET |= (1 << LED_PIN);//turn on LED
        for (i = 0; i < delay * 1000000; i++);
        LPC_GPIO1->FIOCLR |= (1 << LED_PIN);//turn off LED

        while (!(LPC_GPIO2->FIOPIN & (1 << BUTTON_PIN)));//wait for button press
        start_time = LPC_TIM0->TC;//start timer
        while (LPC_GPIO2->FIOPIN & (1 << BUTTON_PIN));//wait for button release
        end_time = LPC_TIM0->TC;//stop timer
        reaction_time = end_time - start_time;//calculate reaction time

        sprintf(reaction_str, "Reaction time: %dms", reaction_time / 1000);
        lcd_gotoxy(0, 0);
        lcd_puts(reaction_str);
    }
}
