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

#define B3		246.94
#define D4		293.66
#define E4		329.63
#define F4_s	369.99
#define G4		392.01
#define A4		440.01
#define B4		493.88
#define C5		523.25

short int wavetable[N];

int pirates_0[BEATS] = {1<<7, 1<<6, 1<<5, 0, 1<<5, 0, 1<<5, 1<<4, 1<<3, 0, 1<<3, 0, 1<<3, 1<<2, 1<<4, 0, 1<<4, 0, 1<<5, 1<<6, 1<<6, 1<<5, 0, 0};
int pirates_1[BEATS] = {1<<7, 1<<6, 1<<5, 0, 1<<5, 0, 1<<5, 1<<4, 1<<3, 0, 1<<3, 0, 1<<3, 1<<2, 1<<4, 0, 1<<4, 0, 1<<5, 1<<6, 1<<5, 1<<5, 0, 0};
int pirates_2[BEATS] = {1<<7, 1<<6, 1<<5, 0, 1<<5, 0, 1<<5, 1<<3, 1<<2, 0, 1<<2, 0, 1<<2, 1<<1, 1<<0, 0, 1<<0, 0, 1<<1, 1<<2, 1<<1, 1<<5, 0, 0};
int pirates_3[BEATS] = {1<<5, 1<<4, 1<<3, 0, 1<<3, 0, 1<<2, 0, 1<<1, 1<<5, 0, 0, 1<<5, 1<<3, 1<<4, 0, 1<<4, 0, 1<<5, 1<<6, 1<<5, 0, 0, 0};
int* pirates[REFR] = {pirates_0, pirates_1, pirates_2, pirates_3};

int audioMode;
int count;
int beat_idx;
int refrain_idx;
int offset;
int step;

void init_wavetable(void)
{
  int x;
  for(x=0; x<N; x++)
    wavetable[x] = 32767 * sin(2 * M_PI * x / N);
}

void setup_gpio() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 3<<(2*4);// | 3 | 3<<(2) | 3<<(2*2);
}

void setup_dac() {
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR &= ~DAC_CR_EN1;
    DAC->CR &= ~DAC_CR_BOFF1;
    DAC->CR |= DAC_CR_TEN1;
    DAC->CR |= DAC_CR_TSEL1;
    DAC->CR |= DAC_CR_EN1;
}

void setup_timer6() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 48 - 1;
    TIM6->ARR = 10 - 1;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;

    NVIC->ISER[0] = 1<<TIM6_DAC_IRQn;
}

float calc_note(int val){
	float note = 0;
	if(val == 128){
		note = B3;
	}
	if(val == 64){
		note = D4;
	}
	if(val == 32){
		note = E4;
	}
	if(val == 16){
		note = F4_s;
	}
	if(val == 8){
		note = G4;
	}
	if(val == 4){
		note = A4;
	}
	if(val == 2){
		note = B4;
	}
	if(val == 1){
		note = C5;
	}
	return note;
}

void pirateAudio(){
    float note;
    int val;
    int sep = 1000;
    if(beat_idx == 20 && refrain_idx == 1) sep = 0;
    if(count < COUNT - sep){
    	val = pirates[refrain_idx][beat_idx];
    	note = calc_note(val);
    	step = (note * 4)* N / (RATE) * (1<<16);
    }
    else{
    	step = 0;
    }
    count++;
    if(count >= COUNT){
        count = 0;
        beat_idx++;
        if(beat_idx == 24){
        	beat_idx = 0;
        	refrain_idx++;
        	if(refrain_idx == REFR){
        		refrain_idx = 0;
        	}
        }
    }
}

void alarmAudio(void){
	if(count < COUNT*10 - 1000){
		step += .00001 * step;
	}
	else{
		step = C5 * N / (RATE) * (1<<16);
	}
	count++;
	count = count > COUNT*10 ? 0 : count;
}

void TIM6_DAC_IRQHandler() {
    TIM6->SR &= ~TIM_SR_UIF;
    if(audioMode == PIRATE){
    	pirateAudio();
    }
    else if(audioMode == ALARM){
    	alarmAudio();
    }
    offset += step;
    if((offset>>16) >= N){
	offset -= N<<16;
    }
    int sample = wavetable[offset>>16];
    sample = sample / 16 + 2048;
    DAC->DHR12R1 = sample;
    DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
}

void playAudio(int mode){
    count = 0;
    offset = 0;
    beat_idx = 0;
    refrain_idx = 0;
    step = 0;

    audioMode = mode;
    init_wavetable();
    setup_gpio();
    setup_dac();
    setup_timer6();
    TIM6_DAC_IRQHandler();
}

void stopAudio(){
	TIM6->CR1 |= TIM_CR1_CEN;
}
