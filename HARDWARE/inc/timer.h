#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

extern uint8_t post_flag;
extern uint8_t ping_flag;
extern uint8_t ten_min_flag;
    
void TIM2_Init(u16 arr,u16 psc); 
void TIM3_Init(u16 arr,u16 psc);

#endif
