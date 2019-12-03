#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>


/*
 * Function:  potsInit(void)
 * --------------------
 * Description:  THE POTENTIOMETER SHOULD BE WIRED TO 3V AND GROUND
 * 				 OUTPUT OF THE POTENTIOMETER GOES TO PA0
 *				TEST LED PLUGGED INTO PA1
 * Returns:void
 * Example: potsInit();
 * Updates:
 *  	- 01/12/19 Colin Cassens - Init
 */
void potsInit(void) {

    //GPIO SHIT
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~GPIO_MODER_MODER1;
    GPIOA->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1_0;


    //Enable ADC
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC->CR2 & RCC_CR2_HSI14RDY));
    ADC1->CR |= ADC_CR_ADEN;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    while(ADC1->CR & ADC_CR_ADSTART);
    float * array[10];
    int t = 0;
    while(1){
        ADC1->CHSELR = 0;
        ADC1->CHSELR |= ADC_CHSELR_CHSEL0;
        while(!(ADC1->ISR & ADC_ISR_ADRDY));
        ADC1->CR |= ADC_CR_ADSTART;
        while(!(ADC1->ISR & ADC_ISR_EOC));
        float x = ADC1->DR * 3/4095.0;
        t++;
        if(x > 1.8 & x < 2.2){
            GPIOA->BSRR |= GPIO_BSRR_BS_1;//0x2;
        }else{
            GPIOA->BSRR |= GPIO_BSRR_BR_1;//~0X2;
        }
    }
} 
