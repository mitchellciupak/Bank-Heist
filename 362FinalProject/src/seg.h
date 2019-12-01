#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

// data sheet: https://cdn-shop.adafruit.com/datasheets/ht16K33v110.pdf

//Function Declarations
void segDisp(int min, int sec) {
	printf("Time[%d:%d]",MIN,SEC); //For Debug Purposes Only
}
