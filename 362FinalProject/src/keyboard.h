/*
 * keyboard.h
 *
 *  Created on: Dec 1, 2019
 *      Author: emasunag
 */

int8_t history[16] = {0};
int8_t lookup[16] = {1,4,7,0xe,2,5,8,0,3,6,9,0xf,0xa,0xb,0xc,0xd};
char char_lookup[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};


int get_key_pressed() {
	int key = get_key_press();
	while(key != get_key_release());
	return key;
}

char get_char_key() {
	int index = get_key_pressed();
	return char_lookup[index];
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

void setup_gpio() {
	// onfigs keypad input and
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

void configKeyboard(void) {
   setup_timer6();
   setup_gpio();
}

void keypadChallenge(void){
	// SCROLL NOTE:
	//	instead of automatically scrolling
	//	offset stimulated by user keypad press
	//	for the sake of program flow
	int key = 0;
	while(key != 'c') {
		display1('I see you want to crack this safe');
		display2('Press c to continue');
		int index = get_key_pressed();
		key = lookup[index];
	}
	while(key != 'c') {
		display1('This part of the security system requires you enter values into the keypad');
		display2('Press c to continue');
		int index = get_key_pressed();
		key = lookup[index];
	}
	while(key != 'c') {
		display1('Once you continue from here, the timer will start');
		display2('Press c to continue');
		int index = get_key_pressed();
		key = lookup[index];
	}
	int pos = 0;
	char userI[4];

	// GENERAL THOUGH PROCESSES:
	//	no back space... just fill up 4 char array and if wrong will have sound to show improper val and will clear
	while(key != 'c') {
		// maybe generate values later...
		display1('XOR this into 0x47b9 to make 0xffff');
		display2('0x' + userI);
		int index = get_key_pressed();
		key = lookup[index];
	}
	pos = 0;
	while(key != 'c') {
		// maybe generate values later...
		display1('Convert 11 to binary');
		display2('0b' + userI);
		int index = get_key_pressed();
		key = lookup[index];
	}
	pos = 0;
	while(key != 'c') {
		// maybe generate values later...
		display1('XOR this into 0x47b9 to make 0xffff');
		display2('0x' + userI);
		int index = get_key_pressed();
		key = lookup[index];
	}
}
