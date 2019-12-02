#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

#include "menu.h"
#include "keypad.h"
#include "pots.h"
#include "lcd.h"

int main(void)
{
	/*Menu*/
        //Init LCD Display
        initDisplay();

        //Display initial message
        const char * msg1 = "Safe Challenge!";
        const char * msg2 = "(C) to continue";
        topDisplayStatic(msg1);
        bottomDisplayStatic(msg2);

		//Find Difficulty
		int difficultyMode = menuStartupDifficulty();

		//Init Timer and 7-Seg
		menuInit(difficultyMode);

	/*Challenge 1: Keypad Challenge*/
	configKeyboard(); //TODO name to keypadInit();

	/*Challenge 2: Pots Challenge*/
	potsInit();

}

