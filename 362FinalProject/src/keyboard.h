#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern const char * msg1;
extern const char * msg2;
extern const char * msg3;
extern const char * msg4;
extern int move;

int8_t history[16] = {0};
int8_t lookup[16] = {1,4,7,0xe,2,5,8,0,3,6,9,0xf,0xa,0xb,0xc,0xd};
char char_lookup[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};
int col = 0;

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
	char key = NULL;
	while(key != 'C') {
		msg1 = "Keypad Challenge!";
		msg2 = "Press (C) to continue";
		move = 0;
	    //display1('I see you want to crack this safe');
		//display2('Press c to continue');
		key = get_char_key();
	}
	while(key != 'c') {
		//display1('This part of the security system requires you enter values into the keypad');
		//display2('Press c to continue');
        key = get_char_key();
	}
	while(key != 'c') {
		//display1('Once you continue from here, the timer will start');
		//display2('Press c to continue');
        key = get_char_key();
	}
	char userI[4];

	// GENERAL THOUGH PROCESSES:
	//	no back space... just fill up 4 char array and if wrong will have sound to show improper val and will clear
	int i = 0;
	while(key != 'C') {
		// maybe generate values later...
		if (i == 4){
			if (userI == "b846")
				break;
			//display1("Wrong!");
			//display2("try again...");
			nano_wait(500000000); // 500ms
			i = 0;
		}
		//display1('XOR this into 0x47b9 to make 0xffff');
		char str [6];
		strcpy(str, '0x');
		strcat(str, userI);
		//display2(str);
        key = get_char_key();

		userI[i] = key;
		i++;
	}
	i = 0;
	while(key != 'c') {
		// maybe generate values later...
		if (i == 3){
			if (userI == "1011")
				break;
			//display1("Wrong!");
			//display2("try again...");
			nano_wait(500000000); // 500ms
			i = 0;
		}
		//display1('Convert 0d11 to binary');
		char str [6];
		strcpy(str, '0b');
		strcat(str, userI);
		//display2(str);
        key = get_char_key();

		userI[i] = key;
		i++;
	}
	i = 0;
	while(key != 'c') {
		// maybe generate values later...
		if (i == 3){
			if (userI == "0021")
				break;
			msg1 = "Wrong!";
			msg2 = "Try Again...";
			move = 0;
			nano_wait(500000000); // 500ms
			i = 0;
		}
		//display1('ay whas 9 + 10');
		char str [6];
		strcpy(str, '0x');
		strcat(str, userI);
		//display2(str);
        key = get_char_key();

		userI[i] = key;
		i++;
	}
}
