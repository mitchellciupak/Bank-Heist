#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>
#include "audio.h"

extern const char * msg1;
extern const char * msg2;
extern const char * msg3;
extern const char * msg4;
extern int move;

/*
 * Function:  potsInit(void)
 * --------------------
 * Description:  THE POTENTIOMETER SHOULD BE WIRED TO 3V AND GROUND
 *               OUTPUT OF THE POTENTIOMETER GOES TO PA0
 *              TEST LED PLUGGED INTO PA1
 * Returns:void
 * Example: potsInit();
 * Updates:
 *      - 01/12/19 Colin Cassens - Init
 *
 *
 */

void potIntro(void){
    // SCROLL NOTE:
    //  instead of automatically scrolling
    //  offset stimulated by user keypad press
    //  for the sake of program flow
    move = 0;
    msg1 = " ";
    msg2 = " ";
    topDisplayStatic();
    char * key = 0;
    char mess [26];
    for (int i = 0; i < 26; i++){
        mess[i] = ' ';
    }
    move = 0;
    char * send = mess;
    msg1 = send;
    msg2 = ("");
    topDisplayStatic();
    nano_wait(1000000000); // 2000ms 2s
    mess[0] = ' ';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[0] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[0] = ' ';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[0] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[0] = 'c';
    mess[1] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[1] = 'h';
    mess[2] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[2] = 'a';
    mess[3] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[3] = 'l';
    mess[4] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[4] = 'l';
    mess[5] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[5] = 'e';
    mess[6] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[6] = 'n';
    mess[7] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[7] = 'g';
    mess[8] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[8] = 'e';
    mess[9] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(150000000);
    mess[9] = '2';
    mess[10] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(100000000);
    mess[10] = ' ';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[10] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[10] = ' ';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[10] = '_';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[10] = ' ';
    send = mess;
    topDisplayStatic();
    nano_wait(500000000);
    mess[10] = '_';
    send = mess;
    topDisplayStatic();
    move = 0;
    msg1 = " ";
    msg2 = " ";
    topDisplayStatic();
}

void potsInit(void) {

    //GPIO SHIT
    //SET PA4 as input and pa5 as output
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~GPIO_MODER_MODER5;
    GPIOA->MODER |= GPIO_MODER_MODER5 | GPIO_MODER_MODER5_0;


    //Enable ADC
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC->CR2 & RCC_CR2_HSI14RDY));
    ADC1->CR |= ADC_CR_ADEN;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    while(ADC1->CR & ADC_CR_ADSTART);
    //float * array[10];
    potIntro();
    msg1 = "Lock Challenge:";
    topDisplayStatic();
    msg4 = "          (C) to Start!     ";
    msg2 = "         Turn the lock to win";
    move = 2;
    char c = get_char_key();
    int lock = 0;
    stopAudio();
    playAudio(POTCHAL);
    potUpdate(0);
    if(c == 'C'){
        msg1 ="Turn the lock!";
        topDisplayStatic();
        move = 0;
        lock = 0;
        while(lock == 0){
            msg2 = "Turns to Pick: 3";
            ADC1->CHSELR = 0;
            ADC1->CHSELR |= ADC_CHSELR_CHSEL5;
            while(!(ADC1->ISR & ADC_ISR_ADRDY));
            ADC1->CR |= ADC_CR_ADSTART;
            while(!(ADC1->ISR & ADC_ISR_EOC));
            float x = ADC1->DR * (3/4095.0);
            //Step 1
            lock = 0;
            if(x > 1.9 && x < 2.1){
                //GPIOA->BSRR |= GPIO_BSRR_BS_5;//0x2;
                potUpdate(1);

                //Do something to say its been picked correctly
                nano_wait(500000000 * 4);// Wait 2 seconds
                if(x > 1.9 && x < 2.1){
                    lock = 1;
                }
                potUpdate(0);
            }
        }
        while(lock == 1){
            msg2 = "Turns to Pick: 2";
            ADC1->CHSELR = 0;
            ADC1->CHSELR |= ADC_CHSELR_CHSEL5;
            while(!(ADC1->ISR & ADC_ISR_ADRDY));
            ADC1->CR |= ADC_CR_ADSTART;
            while(!(ADC1->ISR & ADC_ISR_EOC));
            float x = ADC1->DR * 3/4095.0;
            if(x > 2.8 && x < 3.0){
                //GPIOA->BSRR |= GPIO_BSRR_BS_5;//0x2;
                potUpdate(2);

                //Do something to say its been picked correctly
                nano_wait(500000000 * 4);// Wait 2 seconds
                if(x > 2.8 && x < 3.0){
                    lock = 2;
                }
                potUpdate(0);
            }
        }
        while(lock == 2){
            msg2 = "Turns to Pick: 1";
            ADC1->CHSELR = 0;
            ADC1->CHSELR |= ADC_CHSELR_CHSEL5;
            while(!(ADC1->ISR & ADC_ISR_ADRDY));
            ADC1->CR |= ADC_CR_ADSTART;
            while(!(ADC1->ISR & ADC_ISR_EOC));
            float x = ADC1->DR * 3/4095.0;
            if(x > .8 && x < 1.0){
                //GPIOA->BSRR |= GPIO_BSRR_BS_5;//0x2;
                potUpdate(3); // play sound

                //Do something to say its been picked correctly
                nano_wait(500000000 * 4);// Wait 2 seconds
                if(x > .8 && x < 1.0){
                    lock = 3;
                }
                potUpdate(0);
            }
        }
    }
    else if(c != 'C'){
        //Invalid Input
        move = 0;
        msg2 = "Invalid Input!      ";
        msg4 = "                    ";
    }
    if(lock == 3){
        move = 2;
        msg1 = "Safe Unlocked!      ";
        topDisplayStatic();
        msg2 = "      Challenge Complete!      ";
        msg4 = "      Enter The Safe!          ";
        nano_wait(500000000 * 4);
        playAudio(PIRATE);
    }
}
