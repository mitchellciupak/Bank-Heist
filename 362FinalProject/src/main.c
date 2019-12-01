#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

#include "menu.h"
#include "keypad.h"
#include "pots.h"

int main(void)
{
	/*Menu and Timer Init*/
		//Find Difficulty
		int difficultyMode = menuStartupDifficulty();

		//Init Timer and 7-Seg
		menuInit(difficultyMode);

	/*Challenge 1: Keypad Challenge*/
	keypadInit();

	/*Challenge 2: Pots Challenge*/
	potsInit();

}

