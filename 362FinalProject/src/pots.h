#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

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
 */
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
    float * array[10];
    msg1 = "Lock Challenge:";
    topDisplayStatic();
    msg4 = "          (C) to Start!     ";
    msg2 = "         Turn the lock to win";
    move = 2;
    char c = get_char_key();
    int lock = 0;
    if(c == 'C'){
        msg1 ="Turn the lock!";
        topDisplayStatic();
        move = 0;
        while(lock == 0){
            msg2 = "Locks to Pick: 3";
            ADC1->CHSELR = 0;
            ADC1->CHSELR |= ADC_CHSELR_CHSEL5;
            while(!(ADC1->ISR & ADC_ISR_ADRDY));
            ADC1->CR |= ADC_CR_ADSTART;
            while(!(ADC1->ISR & ADC_ISR_EOC));
            float x = ADC1->DR * 3/4095.0;
            //Step 1
            if(x > 1.8 && x < 2.2){
                //GPIOA->BSRR |= GPIO_BSRR_BS_5;//0x2;
                //Play sound

                //Do something to say its been picked correctly
                nano_wait(500000000 * 4);// Wait 2 seconds
                if(x > 1.8 && x < 2.2){
                    lock = 1;
                }
            }
        }
        while(lock == 1){
            msg2 = "Locks to Pick: 2";
            ADC1->CHSELR = 0;
            ADC1->CHSELR |= ADC_CHSELR_CHSEL5;
            while(!(ADC1->ISR & ADC_ISR_ADRDY));
            ADC1->CR |= ADC_CR_ADSTART;
            while(!(ADC1->ISR & ADC_ISR_EOC));
            float x = ADC1->DR * 3/4095.0;
            if(x > 2.5 && x < 3.0){
                //GPIOA->BSRR |= GPIO_BSRR_BS_5;//0x2;
                //Play sound

                //Do something to say its been picked correctly
                nano_wait(500000000 * 4);// Wait 2 seconds
                if(x > 2.5 && x < 3.0){
                    lock = 2;
                }
            }
        }
        while(lock == 2){
            msg2 = "Locks to Pick: 1";
            ADC1->CHSELR = 0;
            ADC1->CHSELR |= ADC_CHSELR_CHSEL5;
            while(!(ADC1->ISR & ADC_ISR_ADRDY));
            ADC1->CR |= ADC_CR_ADSTART;
            while(!(ADC1->ISR & ADC_ISR_EOC));
            float x = ADC1->DR * 3/4095.0;
            if(x > .7 && x < 1.0){
                //GPIOA->BSRR |= GPIO_BSRR_BS_5;//0x2;
                //Play sound

                //Do something to say its been picked correctly
                nano_wait(500000000 * 4);// Wait 2 seconds
                if(x > .7 && x < 1.0){
                    lock = 3;
                }
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
        msg1 = "Safe Unlocked!";
        topDisplayStatic();
        msg2 = "      Challenge Complete!      ";
        msg4 = "      (C) to continue          ";
    }
}
