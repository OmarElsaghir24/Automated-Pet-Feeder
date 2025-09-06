/*
 * AutomatedPetFeeder.c
 *
 *  Created on: Nov 3, 2023
 *      Author: Omar Elsaghir
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "eeprom.h"
#include "tm4c123gh6pm.h"
#include "initModules.h"
#include "timers.h"

// Bitband Aliases
#define C0_MINUS_OUTPUT    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 0*4)))
#define C0_MINUS           (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 7*4)))
#define BLUE_LED           (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))
#define PUMP_MOTOR         (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*4))) // Currently at PB7. Switch to pin PC4
#define AUGER_MOTOR        (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 5*4))) // Currently at PF4. Switch to pin PC5
#define SPEAKER            (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define SENSOR             (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 4*4)))
#define GREEN_LED          (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))

// Defines
#define C0_MINUS_MASK 128
#define BLUE_LED_MASK 4
#define TRANSISTOR_BASE_MASK 16
#define C0_MINUS_OUTPUT_MASK 1
#define SPEAKER_MASK 2
#define SENSOR_MASK 16
#define GREEN_LED_MASK 8

// Global Variables
uint32_t time = 0;
uint32_t HOUR, MINUTE, realTime;
uint16_t FEEDING;
uint32_t CurrentTime, i, TIME, Time1;
uint32_t WATER = 10;
uint32_t ON;
uint32_t second = 2;
uint32_t second1 = 5;
char str[100];

void ActivateSpeaker()
{
    int i = 0;
    for(i = 0; i < 5000; i++) {
      SPEAKER = 1;
      waitMicrosecond(185);
      SPEAKER = 0;
      waitMicrosecond(185);
      i++;
    }
}

// Analog Comparator ISR
void wideTimer1Isr()
{
    time = WTIMER1_TAV_R;
    // Only used for calibrating capacitive sensor of pet bowl
    //int level =(int) ((((float)(time) - 3784.1)/1.4475)/100) * 100;
    int level;


    if(time < 1680)
    {
        level = 0;
    }
    else if((time > 2490) && (time <= 2540))
    {
        level = 100;
    }
    else if((time > 2620) && (time <= 2680))
    {
        level = 200;
    }
    else if((time > 2760) && (time <= 2800))
    {
        level = 300;
    }
    else if((time > 2870) && (time <= 2910))
    {
        level = 400;
    }
    else if((time > 3020) && (time <= 3070))
    {
        level = 500;
    }
    /*else {
        level = 600;
    }*/

    snprintf(str, sizeof(str), "Water Level in (mL) = %d mL,  Time = %7"PRIu32" (us)\n", level, time);
    putsUart0(str);

    uint32_t water_level = readEeprom(16*WATER);
    uint32_t mode = readEeprom(16*WATER+1);

    if((mode == 1) && (level < water_level))
    {
        if(ON == 1)
        {
            putsUart0("Alarm on\n");
            ActivateSpeaker();
        }
        else if(ON == 0)
        {
            putsUart0("Alarm off\n");
        }
        PWM0_3_CMPA_R = 1023;
        TIMER0_CTL_R |= TIMER_CTL_TAEN;
    }

    else if((mode == 1) && (level == water_level))
    {
        putsUart0("Water Full\n");
    }

    if((mode == 0) && SENSOR && (level < water_level))
    {
        if(ON == 1)
        {
            putsUart0("Alarm on\n");
            ActivateSpeaker();
        }
        else if(ON == 0)
        {
            putsUart0("Alarm off\n");
        }
        GREEN_LED = SENSOR;
        PWM0_3_CMPA_R = 1023;
        TIMER3_CTL_R |= TIMER_CTL_TAEN;
        WTIMER2_CTL_R |= TIMER_CTL_TAEN;
    }
    else if((mode == 0) && (level == water_level))
    {
        putsUart0("Water Full\n");
    }

    COMP_ACMIS_R = COMP_ACMIS_IN0;
}

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0 | SYSCTL_RCGCTIMER_R1 | SYSCTL_RCGCTIMER_R2 | SYSCTL_RCGCTIMER_R3;
    SYSCTL_RCGCACMP_R = 1;
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1 | SYSCTL_RCGCWTIMER_R2;
    SYSCTL_RCGCHIB_R |= SYSCTL_RCGCHIB_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0 | SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);
    _delay_cycles(3);
    _delay_cycles(3);

    initEeprom();
    initPwm();

    // Enable LED pin
    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | C0_MINUS_OUTPUT_MASK | TRANSISTOR_BASE_MASK;
    GPIO_PORTF_PCTL_R |= 0x90000000;
    GPIO_PORTF_DEN_R |= GREEN_LED_MASK | C0_MINUS_OUTPUT_MASK | TRANSISTOR_BASE_MASK;

    GPIO_PORTC_DIR_R &= ~C0_MINUS_MASK;
    GPIO_PORTC_AFSEL_R |= C0_MINUS_MASK;
    GPIO_PORTC_AMSEL_R |= C0_MINUS_MASK;
    GPIO_PORTC_DEN_R |= C0_MINUS_MASK;

    GPIO_PORTA_DIR_R &= ~SENSOR_MASK;
    GPIO_PORTF_DIR_R |= BLUE_LED_MASK;
    GPIO_PORTF_DIR_R |= SPEAKER_MASK;
    GPIO_PORTA_DEN_R |= SENSOR_MASK;
    GPIO_PORTF_DEN_R |= BLUE_LED_MASK;
    GPIO_PORTF_DEN_R |= SPEAKER_MASK;

    // Configure Timer 0 as the time base for motion sensor
    TIMER0_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER0_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;          // configure for periodic mode (count down)
    //TIMER0_TAILR_R = 800000000;                      // set load value to 40e6 for 1 Hz interrupt rate, 20-second timer
    TIMER0_TAILR_R = 40000000 * second1;                      // 2-second timer
    TIMER0_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    //TIMER0_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER0A-16);              // turn-on interrupt 37 (TIMER1A)

    // Configure Timer 1 as the time base
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER1_TAILR_R = 200000000;                       // set load value to 40e6 for 1 Hz interrupt rate
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER1A-16);              // turn-on interrupt 37 (TIMER1A)

    // Configure Timer 1 as the time base
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;          // configure for periodic mode (count down)
    TIMER2_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    NVIC_EN0_R = 1 << (INT_TIMER2A-16);              // turn-on interrupt 37 (TIMER1A)

    //Configure Timer 3 as the time base for filling water in pet dish
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER3_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;          // configure for periodic mode (count down)
    TIMER3_TAILR_R = 40000000 * second1;                      // 5-second timer
    TIMER3_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    //TIMER3_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN1_R = 1 << (INT_TIMER3A-16-32);              // turn-on interrupt 37 (TIMER1A)


    // Configure Wide Timer 1
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER1_TAMR_R = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge time mode, count up
    WTIMER1_CTL_R = TIMER_CTL_TAEVENT_POS;           // measure time from positive edge to positive edge
    WTIMER1_IMR_R = 0;                               // turn-on interrupts
    WTIMER1_TAV_R = 0;                               // zero counter for first period
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;
                   // turn-on counter

    // Configure a wide timer (WTIMER2) for sensor
    WTIMER2_CTL_R &= ~TIMER_CTL_TAEN;               // turn-off counter before reconfiguring
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER2_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR; // configure for edge time mode, count up
    WTIMER2_TAILR_R = 40000000 * second;                     // 2-second timer
    WTIMER2_CTL_R = TIMER_CTL_TAEVENT_POS;           // measure time from positive edge to positive edge
    WTIMER2_IMR_R = TIMER_IMR_TATOIM;                               // turn-on interrupts
    //WTIMER2_TAV_R = 0;                               // zero counter for first period
    //WTIMER2_CTL_R |= TIMER_CTL_TAEN;
    NVIC_EN3_R = 1 << (INT_WTIMER2A-16-96);
                           // turn-on counter

    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_CTL_R |= HIB_CTL_CLK32EN;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_CTL_R |= HIB_CTL_RTCEN;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_IM_R |= HIB_IM_RTCALT0;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    NVIC_EN1_R = 1 << (INT_HIBERNATE-16-32);

    // Enable Analog Comparator
    COMP_ACCTL0_R |= 0x400;
    COMP_ACREFCTL_R |= COMP_ACREFCTL_EN;
    COMP_ACREFCTL_R &= ~(0x100);
    COMP_ACREFCTL_R |= 0xF;
    //COMP_ACINTEN_R |= COMP_ACINTEN_IN0;
    //COMP_ACCTL0_R |= COMP_ACCTL0_CINV;
    COMP_ACCTL0_R |= COMP_ACCTL0_CINV | COMP_ACCTL0_ISEN_RISE;
    NVIC_EN0_R = 1 << (INT_COMP0-16);

    waitMicrosecond(10);
}

// Function for setting alarm (shortest time difference between current and next alarm time)
void setAlarm()
{
    char string[100];
    uint32_t nextDay = 86400;
    uint32_t currentTime;
    uint32_t alarmTime;
    uint32_t time1 = 0;
    int i;
    uint32_t Time;
    uint32_t hour, minute;
    int new_time = -1;

    while(!(HIB_CTL_R & HIB_CTL_WRC));
    currentTime = HIB_RTCC_R % 86400;
    uint32_t hh1, mm1;
    hh1 = currentTime / 3600;
    mm1 = (currentTime % 3600) / 60;
    snprintf(string, sizeof(string), "Time: %02d:%02d\n", hh1, mm1);
    putsUart0(string);
    for(i = 0; i < 10; i++)
    {
        hour = readEeprom(16*i+3);
        minute = readEeprom(16*i+4);
        Time = (minute * 60) + (hour * 60 * 60);
         if(Time > currentTime && Time <= nextDay)
         {
             if(time1 > Time || time1 == 0)
             {
                  time1 = Time;
                  new_time = i;
             }
         }
    }

    if(new_time != -1)
    {
      while(!(HIB_CTL_R & HIB_CTL_WRC));
      HIB_RTCM0_R = time1;
      while(!(HIB_CTL_R & HIB_CTL_WRC));
      HIB_RIS_R |= HIB_RIS_RTCALT0;
      alarmTime = HIB_RTCM0_R;
      uint32_t hh, mm;
      hh = alarmTime / 3600;
      mm = (alarmTime % 3600) / 60;
      snprintf(string, sizeof(string), "Alarm Time: %02d:%02d\n", hh, mm);
      putsUart0(string);
      putcUart0('\n');
      FEEDING = new_time;
    }
}

// Timer 2 ISR
void timer2Isr()
{
    putsUart0("Timer 2 Interrupt\n");
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
    PWM0_3_CMPB_R = 0;
    setAlarm();
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
}

// RTC ISR
void RTCIsr()
{
    uint32_t speed, duration;
    putsUart0("Trigger Interrupt\n");
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_IC_R |= HIB_IC_RTCALT0;
    duration = readEeprom(16*FEEDING+1);
    TIMER2_TAILR_R = duration * 40000000;
    speed = readEeprom(16*FEEDING+2);
    float pwm = (((float)speed) / 100) * 1023;
    PWM0_3_CMPB_R = pwm;
    TIMER2_CTL_R |= TIMER_CTL_TAEN;
}


int main(void)
{

    USER_DATA data;

    initHw();
    initUart0();
    setUart0BaudRate(115200, 40e6);

    while(true)
    {
        bool valid = false;
        data.fieldCount = 0;
        getsUart0(&data);
        setAlarm();
        parseFields(&data);

        uint8_t i;
        uint32_t hours, seconds;
        uint32_t minutes, time_local;
        uint32_t remaining_seconds;
        uint32_t realTime;
        uint32_t DURATION, PWM, HOURS, MINUTES;
        uint32_t VOLUME;
        uint32_t MODE;
        char string[100];

        for(i = 0; i < data.fieldCount; i++)
        {
            putcUart0(data.fieldType[i]);
            putcUart0('\t');
            putsUart0(&data.buffer[data.fieldPosition[i]]);
            putcUart0('\n');
        }

        if(isCommand(&data, "time HH:MM", 2))
        {
            valid = true;
            HOUR = getFieldInteger(&data, 1);
            MINUTE = getFieldInteger(&data, 2);
            time_local = (MINUTE * 60) + (HOUR * 60 * 60);
            while(!(HIB_CTL_R & HIB_CTL_WRC));
            HIB_RTCLD_R = time_local;
            putcUart0('\n');
        }

        else if(isCommand(&data, "time", 0))
        {
            //while(!(HIB_CTL_R & HIB_CTL_WRC));
            //HIB_RTCLD_R = time_local;
            //while(!(HIB_CTL_R & HIB_CTL_WRC));
            while(!(HIB_CTL_R & HIB_CTL_WRC));
            realTime = HIB_RTCC_R;
            //while(!(HIB_CTL_R & HIB_CTL_WRC));
            hours = realTime / 3600;
            remaining_seconds = realTime % 3600;
            minutes = remaining_seconds / 60;
            seconds = remaining_seconds % 60;
            valid = true;

            snprintf(string, sizeof(string), "Current time:   %02d:%02d:%02d\n", hours, minutes, seconds);
            putsUart0(string);
            putcUart0('\n');
         }

         if(isCommand(&data, "feed FEEDING DURATION PWM HH:MM", 5))
         {
            valid = true;
            FEEDING = getFieldInteger(&data, 1);          // Feeding number, between 0 and 9
            DURATION = getFieldInteger(&data, 2);         // Number of seconds auger will dispense food for
            PWM = getFieldInteger(&data, 3);              // Strength in percentage (between 0 and 100) motor for auger will operate in
            HOURS = getFieldInteger(&data, 4);            // Time in hours
            MINUTES = getFieldInteger(&data, 5);          // Time in minutes
            writeEeprom((16 * FEEDING), FEEDING);
            writeEeprom((16 * FEEDING) + 1, DURATION);
            writeEeprom((16 * FEEDING) + 2, PWM);
            writeEeprom((16 * FEEDING) + 3, HOURS);
            writeEeprom((16 * FEEDING) + 4, MINUTES);
            snprintf(string, sizeof(string), "%d          %d            %d          %d:%02d\n", readEeprom(16*FEEDING), readEeprom(16*FEEDING+1), readEeprom(16*FEEDING+2), readEeprom(16*FEEDING+3), readEeprom(16*FEEDING+4));
            putsUart0(string);
            putcUart0('\n');
         }

         else if(isCommand(&data, "Display", 0))
         {
            valid = true;
            uint16_t i;
            for(i = 0; i < 10; i++)
            {
                snprintf(string, sizeof(string), "Feed: %d,  Duration: %d,  PWM: %d,  Hour: %d,  Minute: %02d\n", readEeprom(16*i), readEeprom(16*i+1), readEeprom(16*i+2), readEeprom(16*i+3), readEeprom(16*i+4));
                putsUart0(string);
            }
            putcUart0('\n');
         }

         else if(isCommand(&data, "feed FEEDING delete", 1))
         {
            valid = true;
            uint16_t i;
            for(i = 0; i < 10; i++)
            {
                writeEeprom((16 * FEEDING) + i, 0xFFFFFFFF);
                snprintf(string, sizeof(string), "Feed: %d,  Duration: %d,  PWM: %d,  Hour: %d,  Minute: %02d\n", readEeprom(16*i), readEeprom(16*i+1), readEeprom(16*i+2), readEeprom(16*i+3), readEeprom(16*i+4));
                putsUart0(string);
            }
            putcUart0('\n');

          }

          if(isCommand(&data, "water VOLUME", 1))
          {
             valid = true;
             VOLUME = getFieldInteger(&data, 1);              // Input a volume number (between 100 and 600, range of 100) for the pump to be provided a limit to fill the bowl with water
             if(VOLUME > 0)
             {
                writeEeprom((16 * WATER), VOLUME);
             }
             else
             {
                writeEeprom((16 * WATER), 0x0);
             }
             snprintf(string, sizeof(string), "Water volume: %d\n", VOLUME);
             putsUart0(string);
          }

          if(isCommand(&data, "fill MODE", 1))
          {
             valid = true;
             MODE = getFieldInteger(&data, 1); // Type in 1 for auto. Type in 0 for motion
             writeEeprom((16 * WATER) + 1, MODE);
             snprintf(string, sizeof(string), "Fill mode: %d\n", MODE);
             putsUart0(string);
          }

          if(isCommand(&data, "alert status", 1))
          {
             valid = true;
             ON = getFieldInteger(&data, 1);  // Type in 1 for ON, 0 for OFF
             if(ON == 1)
             {
                writeEeprom((16*WATER)+2, ON);
             }
             else if(ON == 0)
             {
                writeEeprom((16*WATER)+2, ON);
             }
             snprintf(string, sizeof(string), "Alert status: %d\n", ON);
             putsUart0(string);
          }

          if(isCommand(&data, "Show", 0))
          {
             valid = true;
             snprintf(string, sizeof(string), "Water Volume: %d, Mode: %d, Alert Status: %d\n", readEeprom(16*WATER), readEeprom(16*WATER+1), readEeprom(16*WATER+2));
             putsUart0(string);
             putcUart0('\n');
          }

          if(!valid)
          {
             putsUart0("Invalid Command\n");
             putcUart0('\n');
          }

     }
}
