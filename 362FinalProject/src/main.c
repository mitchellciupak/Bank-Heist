#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

#include "menu.h"
#include "keyboard.h"
#include "pots.h"
#include "lcd.h"
#include "audio.h"

const char * msg1;
const char * msg2;
const char * msg3 = "                    "; ;
const char * msg4;
extern int move;

int main(void)
{
	/*Menu*/
    //Init LCD Display
	segInit();
	segClear();
	playAudio(PIRATE);
    initDisplay();
    keypad_gpio();
    setup_timer3();

    //Display initial message
    msg1 = "___THE HEIST___";
    msg4 = "          (C) to continue     ";
    msg2 = "          (C) to continue     ";
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
    menuInit(difficultyMode);

	/*Challenge 1: Keypad Challenge*/
    keypadChallenge(difficultyMode);

	/*Challenge 2: Pots Challenge*/
	potsInit();


	//game win display
	// needs to stop timer
	// acknowledges state to user
	gameEnd_Success();
	//game fail display
	// needs to interrupt game state
	// acknowledges state to user

	// testing


}
