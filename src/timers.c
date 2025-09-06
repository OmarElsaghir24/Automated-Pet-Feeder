/*
 * timers.c
 *
 *  Created on: Oct 20, 2023
 *      Author: Omar Elsaghir
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "wait.h"

// Defines
#define GREEN_LED          (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))
#define TRANSISTOR_BASE    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4)))

// Timer 1 ISR
void timer1Isr()
{
    TRANSISTOR_BASE = 1;
    waitMicrosecond(100);
    WTIMER1_TAV_R = 0;
    TRANSISTOR_BASE = 0;
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;       // clear interrupt flag
    COMP_ACINTEN_R |= COMP_ACINTEN_IN0;
}

// Wide Timer 2 ISR
void wideTimer2Isr()
{
    putsUart0("WideTimer 2 Interrupt\n");
    WTIMER2_ICR_R |= TIMER_ICR_TATOCINT;
    GREEN_LED = 0;
    WTIMER2_CTL_R &= ~TIMER_CTL_TAEN;
}

// Timer 3 ISR
void timer3Isr()
{
    putsUart0("Timer 3 Interrupt\n");
    TIMER3_ICR_R |= TIMER_ICR_TATOCINT;
    PWM0_3_CMPA_R = 0;
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;
}

// Timer 0 ISR
void timer0Isr()
{
    putsUart0("Timer 0 Interrupt\n");
    TIMER0_ICR_R |= TIMER_ICR_TATOCINT;
    PWM0_3_CMPA_R = 0;
    TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
}
