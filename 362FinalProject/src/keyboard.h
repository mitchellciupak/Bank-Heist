#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>



int8_t history[16] = {0};
int8_t lookup[16] = {1,4,7,0xe,2,5,8,0,3,6,9,0xf,0xa,0xb,0xc,0xd};
char char_lookup[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};
int col = 0;
extern const char * msg1;
extern const char * msg2;
extern const char * msg3;
extern const char * msg4;
extern int move;

void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
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

int get_key_pressed() {
    int key = get_key_press();
    while(key != get_key_release());
    return key;
}

char get_char_key() {
    int index = get_key_pressed();
    return char_lookup[index];
}

void setup_gpio() {
	// configs keypad input
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER &= ~0xffff;
	GPIOC->MODER |= 0x0055; //4 pull downinputs (00) 4 outputs (01)
	GPIOC->PUPDR &= ~0xff00;
	GPIOC->PUPDR |= 0xaa00;
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
	int row = (GPIOC->IDR >> 4) & 0xf;
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
	GPIOC->ODR = (1 << col);
}



void keypadChallenge(void){
	setup_timer6();
    setup_gpio();
	// SCROLL NOTE:
	//	instead of automatically scrolling
	//	offset stimulated by user keypad press
	//	for the sake of program flow
	char * key = 0;
	msg1 = "-_-_-_-_-_-_-_-_";
	msg4 = "          (C) to continue     ";
	msg2 = "        Dare to crack me?          ";
	topDisplayStatic();
	move = 2;

	key = get_char_key();
	if(key == 'C') {
		//display1('This part of the security system requires you enter values into the keypad');
		//display2('Press c to continue');
		msg1 = "-_-_-_-_-_-_-_-_";
		msg4 = "          (C) to continue     ";
		msg2 = "        this is the keypad challenge       ";
		topDisplayStatic();
		key = get_char_key();
	}
	if(key == 'C') {
		//display1('Once you continue from here, the timer will start');
		//display2('Press c to continue');
		msg1 = "-_-_-_-_-_-_-_-_";
		msg4 = "          (C) to continue     ";
		msg2 = "       you must convert and enter values      ";
		topDisplayStatic();
		key = get_char_key();
	}
	char userI [6] = "0x    ";
	char comp [6] = "0xB846";
	// GENERAL THOUGH PROCESSES:
	//	no back space... just fill up 4 char array and if wrong will have sound to show improper val and will clear
	int i = 2;
	int solved = 0;
	while(!solved) {
		// maybe generate values later...
		if (i == 6){
			if (userI[2] == 'B' & userI[3] == '8' & userI[4] == '4' & userI[5] == '6'){
				move = 0;
				msg1 = ("Correct!");
				msg2 = ("now another...");
				userI[0] = '0';
				userI[1] = 'b';
				userI[2] = ' ';
				userI[3] = ' ';
				userI[4] = ' ';
				userI[5] = ' ';
				topDisplayStatic();
				nano_wait(2000000000); // 2000ms 2s
				solved = 1;
				break;
			}
			else{
				move = 0;
				msg1 = ("Wrong!");
				msg2 = ("try again...");
				userI[0] = '0';
				userI[1] = 'x';
				userI[2] = ' ';
				userI[3] = ' ';
				userI[4] = ' ';
				userI[5] = ' ';
				topDisplayStatic();
				nano_wait(2000000000); // 2000ms 2s
				move = 2;
			}
			i = 2;
		}
		char * m1 = userI;
		//display1('XOR this into 0x47b9 to make 0xffff');
		msg1 = m1;
		msg4 = "      XOR this into 0x47b9 to make 0xffff     ";
		msg2 = "      XOR this into 0x47b9 to make 0xffff     ";
		topDisplayStatic();
		key = get_char_key();
		userI[i] = key;
		topDisplayStatic();
		i++;
	}
	move = 2;
	i = 2;
	solved = 0;
	while(!solved) {
		// maybe generate values later...
		if (i == 6){
					if (userI[2] == '1' & userI[3] == '0' & userI[4] == '1' & userI[5] == '1'){
						move = 0;
						msg1 = ("Correct!");
						msg2 = ("");
						userI[0] = '0';
						userI[1] = 'b';
						userI[2] = ' ';
						userI[3] = ' ';
						userI[4] = ' ';
						userI[5] = ' ';
						topDisplayStatic();
						nano_wait(2000000000); // 2000ms 2s
						solved = 1;
						break;
					}
					else{
						move = 0;
						msg1 = ("Wrong!");
						msg2 = ("try again...");
						userI[0] = '0';
						userI[1] = 'b';
						userI[2] = ' ';
						userI[3] = ' ';
						userI[4] = ' ';
						userI[5] = ' ';
						topDisplayStatic();
						nano_wait(2000000000); // 2000ms 2s
						move = 2;
					}
					i = 2;
				}
		char * m1 = userI;
		//display1('XOR this into 0x47b9 to make 0xffff');
		msg1 = m1;
		msg4 = "      What is 11 in binary     ";
		msg2 = "      What is 11 in binary     ";
		topDisplayStatic();
		key = get_char_key();
		userI[i] = key;
		topDisplayStatic();
		i++;
	}
	move = 2;
	for (int n = 0; n < 7; n++){
		msg1 = ("You beat me!");
		msg2 = ("-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_");
		msg4 = ("-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_");
		topDisplayStatic();
		nano_wait(1000000000); // 2000ms 2s
	}
	for (int n = 0; n < 7; n++){
		msg1 = ("Now the next...");
		msg2 = ("-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_");
		msg4 = ("-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_");
		topDisplayStatic();
		nano_wait(1000000000); // 2000ms 2s
	}
}
