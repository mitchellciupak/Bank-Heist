#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

#include "menu.h"
#include "pots.h"
#include "audio.h"

int main(void){
	playAudio(PIRATE);
	return 1;
}
