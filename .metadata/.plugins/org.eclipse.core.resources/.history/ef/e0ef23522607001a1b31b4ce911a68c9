#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

//Function Declarations
int chooseDifficulty(void);

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
