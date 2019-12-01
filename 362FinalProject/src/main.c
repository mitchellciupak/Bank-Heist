#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

#include "menu.h"

//Function Declarations
int chooseDifficulty(void);

int8_t history[16] = {0};
int8_t lookup[16] = {1,4,7,0xe,2,5,8,0,3,6,9,0xf,0xa,0xb,0xc,0xd};
char char_lookup[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};

int main(void)
{
	/*Game Menu*/
	int difficulty = startupRoutine();
	//int difficulty = chooseDifficulty();


}


int chooseDifficulty(void) {
	//enable the clock to GPIOA
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	//Set PA0 to input mode (00)
	GPIOA->MODER &= 0b00;

	int diff = 0;
	while(diff == 0) {
		//Read input
		diff = GPIOA->IDR & 1;
	}
	return diff;
}
