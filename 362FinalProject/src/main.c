#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

//Function Declarations
int chooseDifficulty(void);

int8_t history[16] = {0};
int8_t lookup[16] = {1,4,7,0xe,2,5,8,0,3,6,9,0xf,0xa,0xb,0xc,0xd};
char char_lookup[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};

int main(void)
{
	/*Game Menu*/
	int difficulty = chooseDifficulty();


}


int chooseDifficulty(void) {
	//enable the clock to GPIOA
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	//Set PA0 to input mode (00)
	GPIOA->MODER &= 0b00;

	int diff = 0;
	while(diff == 0) {
		//Read input
		diff = GPIOA->IDR[0];
	}
	return diff;
}

int get_key_pressed() {
	int key = get_key_press();
	while(key != get_key_release());
	return key;
}

char get_char_key() {
	int index = get_key_pressed();
	return char_lookup[index];
}

int get_key_press() {
	while(1){
		for(int i = 0; i < 16; i++){
			if (history[i] == 1){
				return i;
			}
		}
	}
}
int get_key_release() {
	while(1){
			for(int i = 0; i < 16; i++){
				if (history[i] == -2){
					return i;
				}
			}
		}
}

void setup_gpio() {
	// onfigs keypad input and
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~0x3fffff;
	GPIOA->MODER |= 0x2a0055
	GPIOA->AFR[1] &= ~0xf;
	GPIOA->AFR[1] |= 0x2;
	GPIOA->AFR[1] &= ~0xf0;
	GPIOA->AFR[1] |= 0x20;
	GPIOA->AFR[1] &= ~0xf00;
	GPIOA->AFR[1] |= 0x200;
	GPIOA->PUPDR &= ~0xff00;
	GPIOA->PUPDR |= 0xaa00;
}

void setup_timer6() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = (240-1);
    TIM6->ARR = (200-1);
    TIM6->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] = 1<<TIM6_DAC_IRQn;
    TIM6->CR1 |= TIM_CR1_CEN;
}
void TIM6_DAC_IRQHandler() {
	TIM6->SR &= ~TIM_SR_UIF;
	int row = (GPIOA->IDR >> 4) & 0xf;
	int index = col << 2;
	for (int i = 0; i < 4; i++){
		history[index+i] = history[index+i] << 1;
		int e = (row >> i) & 0x1;
		history[index+i] |= e;
	}
	col++;
	if (col > 3){
		col = 0;
	}
	GPIOA->ODR = (1 << col);

}

void configKeyboard(void) {
   setup_timer6();
   setup_gpio();
}
