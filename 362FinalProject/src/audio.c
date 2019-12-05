#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "audio.h"

short int wavetable[N];

int pirates_0[BEATS] = {1<<7, 1<<6, 1<<5, 0, 1<<5, 0, 1<<5, 1<<4, 1<<3, 0, 1<<3, 0, 1<<3, 1<<2, 1<<4, 0, 1<<4, 0, 1<<5, 1<<6, 1<<6, 1<<5, 0, 0};
int pirates_1[BEATS] = {1<<7, 1<<6, 1<<5, 0, 1<<5, 0, 1<<5, 1<<4, 1<<3, 0, 1<<3, 0, 1<<3, 1<<2, 1<<4, 0, 1<<4, 0, 1<<5, 1<<6, 1<<5, 1<<5, 0, 0};
int pirates_2[BEATS] = {1<<7, 1<<6, 1<<5, 0, 1<<5, 0, 1<<5, 1<<3, 1<<2, 0, 1<<2, 0, 1<<2, 1<<1, 1<<0, 0, 1<<0, 0, 1<<1, 1<<2, 1<<1, 1<<5, 0, 0};
int pirates_3[BEATS] = {1<<5, 1<<4, 1<<3, 0, 1<<3, 0, 1<<2, 0, 1<<1, 1<<5, 0, 0, 1<<5, 1<<3, 1<<4, 0, 1<<4, 0, 1<<5, 1<<6, 1<<5, 0, 0, 0};
int* pirates[REFR] = {pirates_0, pirates_1, pirates_2, pirates_3};

int audioMode;
int count = 0;
int potEn = 0;
int beat_idx = 0;
int refrain_idx = 0;
int offset1 = 0;
int step = 0;

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
    TIM6->PSC = 480 - 1;
    TIM6->ARR = 10 - 1;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;

    NVIC->ISER[0] = 1<<TIM6_DAC_IRQn;
}

int calc_note(int val){
	int note = 0;
	if(val & (1<<7)){
		note = B3;
	}
	if(val & (1<<6)){
		note = D4;
	}
	if(val & (1<<5)){
		note = E4;
	}
	if(val & (1<<4)){
		note = F4_s;
	}
	if(val & (1<<3)){
		note = G4;
	}
	if(val & (1<<2)){
		note = A4;
	}
	if(val & (1<<1)){
		note = B4;
	}
	if(val & (1<<0)){
		note = C5;
	}
	return note;
}

void pirateAudio(){
    float note = 0;
    int val;
    int sep = 1000;
    if(beat_idx == 20 && refrain_idx == 1) sep = 0;
    if(count < COUNT - sep){
    	val = pirates[refrain_idx][beat_idx];
    	note = calc_note(val);
    	step = (note)* N / (RATE) * (1<<16);
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

void potUpdate(int stage){
	if(stage == 1){
		step = E4 * N / (RATE) * (1<<16);
	}
	else if(stage == 2){
		step = G4 * N / (RATE) * (1<<16);
	}
	else if(stage == 3){
		step = B4 * N / (RATE) * (1<<16);
	}
	else{
		step = 0;
	}
}

void TIM6_DAC_IRQHandler() {
    TIM6->SR &= ~TIM_SR_UIF;
    if(audioMode == PIRATE){
    	pirateAudio();
    }
    else if(audioMode == ALARM){
    	alarmAudio();
    }
    offset1 += step;
    if((offset1>>16) >= N){
	offset1 -= N<<16;
    }
    int sample = wavetable[offset1>>16];
    sample = sample / 16 + 2048;
    DAC->DHR12R1 = sample;
    DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
}

void playAudio(int mode){
    audioMode = mode;
    init_wavetable();
    setup_gpio();
    setup_dac();
    setup_timer6();
    TIM6_DAC_IRQHandler();
}

void stopAudio(){
	TIM6->CR1 &= ~TIM_CR1_CEN;
}


