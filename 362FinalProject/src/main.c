#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

#include "menu.h"
#include "keypad.h"
#include "pots.h"
#include"lcd.h"
#include "seg.h"

const char * msg1;
const char * msg2;

int main(void)
{
	/*Menu*/
        //Init LCD Display and 7 seg
		segInit();
		segClear();
        initDisplay();

        //Display initial message
        msg1 = "Safe Challenge!";
        msg2 = "(C) to continue";
        topDisplayStatic();
        bottomDisplayStatic();

		//Find Difficulty
		int difficultyMode = menuStartupDifficulty();

		//Init Timer
		menuInit(difficultyMode);

	/*Challenge 1: Keypad Challenge*/
	configKeyboard(); //TODO name to keypadInit();
	  
  // Else return success.
  return SUCCESS;
}
