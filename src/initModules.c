/*
 * initModules.c
 *
 *  Created on: Nov 15, 2023
 *      Author: Omar Elsaghir
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "initModules.h"

// Defines
#define PUMP_MASK 16
#define AUGER_MASK 32

// Initialize PWM for pump and motor control
void initPwm()
{
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;
    _delay_cycles(3);

    GPIO_PORTC_DEN_R |= PUMP_MASK | AUGER_MASK;
    GPIO_PORTC_AFSEL_R |= PUMP_MASK | AUGER_MASK;
    GPIO_PORTC_PCTL_R &= ~(GPIO_PCTL_PC4_M | GPIO_PCTL_PC5_M);
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC4_M0PWM6 | GPIO_PCTL_PC5_M0PWM7;

    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;                // reset PWM0 module
    SYSCTL_SRPWM_R = 0;
    PWM0_3_CTL_R = 0;
    PWM0_3_GENA_R |= PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTLOAD_ZERO;
    PWM0_3_GENB_R |= PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTLOAD_ZERO;
    PWM0_3_LOAD_R = 1024;
    PWM0_3_CMPA_R = 0;
    PWM0_3_CMPB_R = 0;
    PWM0_3_CTL_R = PWM_0_CTL_ENABLE;
    PWM0_ENABLE_R = PWM_ENABLE_PWM6EN | PWM_ENABLE_PWM7EN;
}
