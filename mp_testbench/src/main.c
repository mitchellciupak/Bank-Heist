/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

int red_pin = 9;
int green_pin = 7;
int yellow_pin = 8;

// Configure red, yellow and green led to output
void setup_gpio(){
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(3<<(2 * green_pin) |3<<(2 * red_pin) | 3<<(2 * yellow_pin));
    GPIOC->MODER |= (1<<(2 * green_pin) | 1<<(2 * red_pin) | 1<<(2 * yellow_pin));
}

// YOU CAN TOUCH THIS
int challenge_1(){
    for(int i = 0; i < 100000; i++);
    return 0;
}

int main(void)
{
    setup_gpio();
    GPIOC->ODR &= ~(1<<yellow_pin | 1<<green_pin | 1<<red_pin);
    GPIOC->BSRR = 1<<yellow_pin;
    int is_success = challenge_1();
    GPIOC->BRR =(1<<yellow_pin);

    if(is_success == 1){
	GPIOC->BSRR = (1<<green_pin);
    }
    else{
	GPIOC->BSRR = (1<<red_pin);
    }
	for(;;);
}
