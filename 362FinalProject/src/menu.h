#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

#include "seg.h"

//Globals
int EASY_MIN = 10;
int MED_MIN = 6;
int HARD_MIN = 3;

int MIN = 0;
int SEC = 0;

/*
 * Function:  menuSetupGPIO(void)
 * --------------------
 * Description: Initializes GPIOA
 * Returns:void
 * Example: menuSetupGPIO();
 * Updates:
 *  	- 06/11/19 Mitchell Ciupak - Init
 */
void menuSetupGPIO() {
	//enable the clock to GPIOA
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	//Set PA0 to PA2 to Input
	GPIOA->MODER &= ~0x3f;
}

/*
 * Function:  menuStartupDifficulty(void)
 * --------------------
 * Description: Finds Difficulty Based on Wires Pulled
 * Returns:void
 * Example: int difficultyMode = startupRoutine();
 * Updates:
 *  	- 06/11/19 Mitchell Ciupak - Init
 *  	- 01/12/19 Mitchell Ciupak - Updated Name
 */
int menuStartupDifficulty() {
	int difficulty = 0;

	menuSetupGPIO();

	//Wait for wire to plugged in
	//while(!(GPIOA->IDR & GPIO_IDR_0) & !(GPIOA->IDR & GPIO_IDR_0) & !(GPIOA->IDR & GPIO_IDR_0));

	//Wait for a wire to be pulled from power
	while((GPIOA->IDR & GPIO_IDR_0) && ((GPIOA->IDR & GPIO_IDR_1) && (GPIOA->IDR & GPIO_IDR_2)));

	//Select Difficulty Based on Wire Pulled
	if(!(GPIOA->IDR & GPIO_IDR_0)){//Hard
		difficulty = 1;
	}
	else if(!(GPIOA->IDR & GPIO_IDR_1)){ //Medium
		difficulty = 2;
	}
	else{ //Easy
		difficulty = 3;
	}


	return difficulty;
}


/*
 * Function:  menuInit(mode)
 * --------------------
 * Description: Starts a count down timer on STM invoked with I2C
 * Parameters:
 * 		- mode: easy(1), medium(2), hard(3)
 * Returns:void
 * Example: start_timer(1);
 * Updates:
 *  	- 06/11/19 Mitchell Ciupak - Init
 */
void menuInit(int mode) {

	//menuSetupGPIO();

	switch (mode)
	{
	    case 1:
	    	MIN = EASY_MIN;
	    	SEC = 0;
	        break;
	    case 2:
	    	MIN = MED_MIN;
	    	SEC = 0;
	        break;
	    case 3:
	    	MIN = HARD_MIN;
	    	SEC = 0;
	    	break;
	    default:
	    	MIN = EASY_MIN;
	    	SEC = 0;
	}

	menuInitTim2();

}

/*
 * Function:  menuInitTim6(void)
 * --------------------
 * Description: Enable's Timer 6(update event occurs every 1ms)
 * Returns:void
 * Example:init_time6();
 * Updates:
 *  	- 06/11/19 Mitchell Ciupak - Init
 *  	- 01/12/19 Mitchell Ciupak - Moved to Timer 2 from Timer 6
 */
void menuInitTim2(void) {
	    //Enable clock to Timer 6.
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

		//Set PSC and ARR values so that the timer update event occurs exactly once every 1ms
		TIM2->PSC = 480 - 1;
		TIM2->ARR = 100 - 1;

		//Enable UIE in the TIMER6's DIER register.
		TIM2->DIER |= TIM_DIER_UIE;
		TIM2->CR1 = TIM_CR1_CEN;

		//Enable TIM6 interrupt in NVIC's ISER register.
		NVIC->ISER[0] = 1<<TIM2_IRQn;
	}

/*
 * Function:  TIM2_IRQHandler()
 * --------------------
 * Description: TIM6 interrupt: Counts down from MIN and SEC
 * Returns:void
 * Example: NVIC->ISER[0] = 1<<TIM2_IRQn;
 * Updates:
 *  	- 06/11/19 Mitchell Ciupak - Init
 *  	- 01/12/19 Mitchell Ciupak - Moved to Timer 2 from Timer 6
 */
int calls = 0;
void TIM2_IRQHandler() {
	//Acknowledge TIM6 interrupt
	TIM2->SR &= ~TIM_SR_UIF;

	//Count down
	calls++;
	if(calls == 1000){
		calls = 0;
		menuCountdown();
	}

}

/*
 * Function: menuCountdown()
 * --------------------
 * Description: Counts down from MIN and SEC and calls seg
 * Returns:void
 * Example: countdown();
 * Updates:
 *  	- 06/11/19 Mitchell Ciupak - Init
 *  	- 01/12/19 Mitchell Ciupak - Added Function Call
 *  	- 02/12/19 Carrie	Kemmet - Blinking logic
 */
void menuCountdown() {

	if(SEC == 0 && MIN == 0){
		segBlink();
	}
	else{
		int total = MIN * 60 + SEC;
		total--;

		SEC = total % 60;
		MIN = total / 60;
		segDisp(MIN,SEC);
	}
}
