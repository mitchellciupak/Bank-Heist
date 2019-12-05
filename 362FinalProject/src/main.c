#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

#include "menu.h"
#include "keyboard.h"
#include "pots.h"
#include "lcd.h"

const char * msg1;
const char * msg2;
const char * msg3 = "                    "; ;
const char * msg4;
extern int move;

int main(void)
{
	/*Menu*/
    //Init LCD Display
    initDisplay();
    setup_gpio();
    setup_timer3();

    //Display initial message
    msg1 = "Safe Challenge!";
    msg4 = "          (C) to continue     ";
    msg2 = "        Main Menu:            ";
    topDisplayStatic();
    move = 2;
    char c = get_char_key();

    if(c == 'C'){
        msg1 = "SelectDifficulty:";
        msg2 = "   Red(Hard) : Yellow(Med) : Green(Easy)";
        msg4 = "          Pull Wire to Continue  ";
        topDisplayStatic();
    }
  
    //Find Difficulty
    int difficultyMode = menuStartupDifficulty();

    //Init Timer and 7-Seg
    //menuInit(difficultyMode);

	/*Challenge 1: Keypad Challenge*/
    keypadChallenge(difficultyMode);

	/*Challenge 2: Pots Challenge*/
	potsInit();

}
