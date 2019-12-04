#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>


#define SPI_DELAY 1337

// Extern declarations of function pointers in main.c.
void (*cmd)(char b) = 0;
void (*data)(char b) = 0;
void (*display1)(const char *);
void (*display2)(const char *);

void generic_lcd_startup(void);

void CMD(char);
void Data_func(char);
void nondma_display1(const char*);
void nondma_display2(const char*);
void init_lcd(void);
void sendByte(int);
void sendBit(int);
void initDisplay(void);
void topDisplayStatic();
void topDisplayScroll(const char *);
void bottomDisplayScroll();
void bottomDisplayStatic();

void (*display1)(const char *) = 0;
void (*display2)(const char *) = 0;
int offset = 0;

// This array will be used with dma_display1() and dma_display2() to mix
// commands that set the cursor location at zero and 64 with characters.
uint16_t dispmem[34] = {
        0x080 + 0,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x080 + 64,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
};

extern const char * msg1;
extern const char * msg2;
extern const char * msg3;
extern const char * msg4;
int move = 0;

void initDisplay(){
    cmd = CMD;
    data = Data_func;
    display1 = nondma_display1;
    display2 = nondma_display2;

    // Initialize the display.
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    TIM14->PSC = 4799;
    TIM14->ARR = 999;
    TIM14->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] = 1<<(TIM14_IRQn);
    TIM14->CR1 |= TIM_CR1_CEN;

    init_lcd();
}

void TIM14_IRQHandler(){
    TIM14->SR &= ~TIM_SR_UIF;
    bottomDisplayScroll();
}

void topDisplayStatic(){
    display1(msg1);
}

void topDisplayScroll(const char *msg){
    int offset1 = 0;
    while(1){
        display1(&msg[offset]);
        nano_wait(100000000);
        offset1 += 1;
        if (offset1 == 32)
            offset1 = 0;
    }
}

void bottomDisplayScroll(){
    //nano_wait(100000000);
    if(move == 1){
        if(offset < 30){
            display2(&msg2[offset]);
        }else if(offset < 32){
            display2(&msg3[offset-24]);
        }
        offset += 1;
        if (offset == 32)
            offset = 0;
    }else if(move == 0){
        display2(&msg2[offset]);
        offset = 0;
    }else if(move == 2){
        if(offset < 30){
            display2(&msg2[offset]);
        }else if(offset < 32){
            display2(&msg3[offset-24]);
        }else if(offset < 60){
            display2(&msg4[offset-32]);
        }
        offset += 1;
        if (offset == 60)
            offset = 0;
    }else{
        display2("");
    }

}

void bottomDisplayStatic(){
    display2(msg2);
}

void CMD(char b) {
    const int NSS = 1<<12;
    GPIOB->BRR = NSS; // NSS low
    nano_wait(SPI_DELAY);
    sendBit(0); // RS = 0 for command.
    sendBit(0); // R/W = 0 for write.
    sendByte(b);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = NSS; // set NSS back to high
    nano_wait(SPI_DELAY);
}


void Data_func(char b) {
    const int NSS = 1<<12;
    GPIOB->BRR = NSS; // NSS low
    nano_wait(SPI_DELAY);
    sendBit(1); // RS = 1 for data.
    sendBit(0); // R/W = 0 for write.
    sendByte(b);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = NSS; // set NSS back to high
    nano_wait(SPI_DELAY);
}

void nondma_display1(const char *s) {
    // put the cursor on the beginning of the first line (offset 0).
    cmd(0x80 + 0);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

void nondma_display2(const char *s) {
    // put the cursor on the beginning of the second line (offset 64).
    cmd(0x80 + 64);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x] != '\0')
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

void sendBit(int b) {
    const int SCK = 1<<13;
    const int MOSI = 1<<15;
    // We do this slowly to make sure we don't exceed the
    // speed of the device.
    GPIOB->BRR = SCK;
    if (b)
        GPIOB->BSRR = MOSI;
    else
        GPIOB->BRR = MOSI;
    //GPIOB->BSRR = b ? MOSI : (MOSI << 16);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = SCK;
    nano_wait(SPI_DELAY);
}

void sendByte(int b) {
    int x;
    // Send the eight bits of a byte to the SPI channel.
    // Send the MSB first (big endian bits).
    for(x=8; x>0; x--) {
        sendBit(b & 0x80);
        b <<= 1;
    }
}

void init_lcd(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->BSRR = 1<<12; // set NSS high
    GPIOB->BRR = (1<<13) + (1<<15); // set SCK and MOSI low
    // Now, configure pins for output.
    GPIOB->MODER &= ~(3<<(2*12));
    GPIOB->MODER |=  (1<<(2*12));
    GPIOB->MODER &= ~( (3<<(2*13)) | (3<<(2*15)) );
    GPIOB->MODER |=    (1<<(2*13)) | (1<<(2*15));

    generic_lcd_startup();
}


void generic_lcd_startup(void) {
    nano_wait(100000000); // Give it 100ms to initialize
    cmd(0x38);  // 0011 NF00 N=1, F=0: two lines
    cmd(0x0c);  // 0000 1DCB: display on, no cursor, no blink
    cmd(0x01);  // clear entire display
    nano_wait(6200000); // clear takes 6.2ms to complete
    cmd(0x02);  // put the cursor in the home position
    cmd(0x06);  // 0000 01IS: set display to increment
}
