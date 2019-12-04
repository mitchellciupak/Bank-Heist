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
    setup_timer6();

    //Display initial message
    keypadChallenge();
    move = 0;
	msg1 = ("Guess you win");
	msg2 = ("      :(     ");
	userI[0] = '0';
	userI[1] = 'b';
	userI[2] = ' ';
	userI[3] = ' ';
	userI[4] = ' ';
	userI[5] = ' ';
	topDisplayStatic();
//    msg1 = "Safe Challenge!";
//    msg4 = "          (C) to continue     ";
//    msg2 = "        Main Menu:            ";
//    topDisplayStatic();
//    move = 2;
//    char c = get_char_key();
//    move = 0;
//    if(c == 'C'){
//        msg1 = "Select Difficulty:";
//        msg2 = "         Red(Hard) : Yellow(Med) : Green(Easy)";
//        topDisplayStatic();
//        move = 1;
//        //bottomDisplayScroll(msg2);
//    }
    //Find Difficulty
    int difficultyMode = menuStartupDifficulty();

    //Init Timer and 7-Seg
    menuInit(difficultyMode);

	/*Challenge 1: Keypad Challenge*/
    keypadChallenge();

	/*Challenge 2: Pots Challenge*/
	potsInit();

}
