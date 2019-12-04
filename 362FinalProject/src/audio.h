#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define RATE 		100000
#define N 			1000
#define PIRATE 		0
#define ALARM 		1
#define POTCHAL		2
#define BEATS		24
#define REFR		4
#define BPM		426
#define COUNT	(100000/(BPM/30))

#define MULT	4
#define B3		(246.94 * 4)
#define D4		(293.66 * 4)
#define E4		(329.63 * 4)
#define F4_s	(369.99 * 4)
#define G4		(392.01 * 4)
#define A4		(440.01 * 4)
#define B4		(493.88 * 4)
#define C5		(523.25 * 4)

extern void potUpdate(int stage);
extern void playAudio(int mode);
extern void stopAudio();
