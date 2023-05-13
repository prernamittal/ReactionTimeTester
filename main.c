#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int i;
unsigned long LED = 0x00000010;

void lcd_init(void);
void lcd_comdata(int, int);
void lcd_puts(const unsigned char *);
void delay_lcd(unsigned int);
void write_lcd(int, int);
void clear_ports(void);
void delayMS(unsigned int milliseconds);

#define LCD_RS_PIN 27
#define LCD_E_PIN 28
#define LCD_D4_PIN 23
#define BUTTON_PIN 12

int main(void) {
    uint32_t start_time, end_time, reaction_time;
    char reaction_str[16];
    LPC_PINCON->PINSEL0 &= 0xFF0000FF; //Configure Port0 PINS P0.4-P0.11 as GPIO function
    LPC_GPIO0->FIODIR |= 0x00000FF0; //Configure P0.4-P0.11 as output port
    LPC_GPIO2->FIODIR &= ~(1 << BUTTON_PIN);
    lcd_init();

    while (1) {
        uint32_t delay = rand() % 5 + 1; // 1-5 seconds
        LPC_GPIO0->FIOSET |= (1 << LED); // Turn on LED
        for (i = 0; i < delay * 10000; i++);
        LPC_GPIO0->FIOCLR |= (1 << LED); // Turn off LED
				delayMS(1000);
        start_time = LPC_TIM0->TC; // Start time
        while (LPC_GPIO2->FIOPIN & (1 << BUTTON_PIN)); // Wait for button press
				delayMS(1000);
        end_time = LPC_TIM0->TC; // End time
        reaction_time = end_time - start_time; // Calculate reaction time

        sprintf(reaction_str, "Reaction time: %dms", reaction_time / 1000); // Convert reaction time to string
        lcd_comdata(0x80, 0); // Set cursor to first line
        delay_lcd(800);
        lcd_comdata(0x01,0);// Clear first line
        lcd_comdata(0x80, 0); // Set cursor to first line
        delay_lcd(800);
        lcd_puts((const unsigned char *)reaction_str); // Print reaction time
    }
}

void lcd_init(void) {
    LPC_PINCON->PINSEL1 &= 0xFC003FFF; //P0.23 to P0.28
    LPC_GPIO0->FIODIR |= (1 << LCD_RS_PIN) | (1 << LCD_E_PIN) | (0x0F << LCD_D4_PIN); //P0.23 to P0.28
    delay_lcd(3200);
    lcd_comdata(0x33, 0);
    delay_lcd(30000);
    lcd_comdata(0x32, 0);
    delay_lcd(30000);
    lcd_comdata(0x28, 0); //function set
    delay_lcd(30000);
    lcd_comdata(0x0C, 0);//display on cursor off
    delay_lcd(800);
    lcd_comdata(0x06, 0);//entry mode set
    delay_lcd(800);
    lcd_comdata(0x01, 0);//clear display
}

void lcd_comdata(int temp1, int type){
	int temp2 = temp1 & 0xf0; //move data (26-8+1) times : 26 - HN place, 4 - Bits
	temp2 = temp2 << 19; //data lines from 23 to 26
	write_lcd(temp2, type);
	temp2 = temp1 & 0x0f; //26-4+1
	temp2 = temp2 << 23; 
	write_lcd(temp2, type);
	delay_lcd(1000);
	return;
}

void lcd_puts(const unsigned char *buf1){
	unsigned int i=0;
	unsigned int temp3;
	while(buf1[i]!='\0'){ // loop until null character
		temp3 = buf1[i];
		lcd_comdata(temp3, 1);
		i++;
		if(i==16){ // if i=16, go to next line
			lcd_comdata(0xc0, 0);
		}
	}
	return;
}

void delay_lcd(unsigned int r1){
    unsigned int r;
    for(r=0;r<r1;r++);
    return;
}

void write_lcd(int temp2, int type){ //write to command/data reg 
    clear_ports();
    LPC_GPIO0->FIOPIN = temp2; // Assign the value to the data lines 
    if(type==0)
    LPC_GPIO0->FIOCLR = 1<<27; // clear bit RS for Command
    else
    LPC_GPIO0->FIOSET = 1<<27; // set bit RS for Data
    LPC_GPIO0->FIOSET = 1<<28; // EN=1
    delay_lcd(25);
    LPC_GPIO0->FIOCLR = 1<<28; // EN =0
    return;
}

void clear_ports(void){
    // Clearing the lines at power on
    LPC_GPIO0->FIOCLR = 0x0F<<23; //Clearing data lines
    LPC_GPIO0->FIOCLR = 1<<27; //Clearing RS line
    LPC_GPIO0->FIOCLR = 1<<28; //Clearing Enable line
    return;
}

void delayMS(unsigned int milliseconds){ //Using Timer0
	LPC_TIM0->CTCR = 0x0; //Timer mode
	LPC_TIM0->PR = 2; //Increment TC at every 3 pclk
	LPC_TIM0->TCR = 0x02; //Reset Timer
	LPC_TIM0->TCR = 0x01; //Enable timer
	while(LPC_TIM0->TC < milliseconds); //wait until timer counter reaches the desired delay
	LPC_TIM0->TCR = 0x00; //Disable timer
}
