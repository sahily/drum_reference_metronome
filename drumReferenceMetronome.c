/*
 * drumReferenceMetronome.c
 *
 * Created: 9/21/2018 1:30:12 AM
 * Author : Sahil S. Mahajan
 * Description: Program to calculate the tempo of a real-time drum beat
 *
 */ 

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#define F_CPU 16000000UL
#include "util/delay.h"

void display(void);
void getPlaceValues(unsigned int n);
void calculateTimeInterval(void);
void updateTempoDisplay(void);
void init(void);


unsigned int numberList[10] = {
                                0xFC,   //0         
                                0x60,   //1
                                0xDA,   //2 
                                0xF2,   //3
                                0x66,   //4
                                0xB6,   //5
                                0xBE,   //6
                                0xE0,   //7
                                0xFE,   //8
                                0xF6    //9
                              };
    
                  
unsigned int selectDigit[3] = {
                                0x7F,           //select 1st digit on 7seg display
                                0xBF,           //select 2nd digit on 7seg display
                                0xDF            //select 3rd digit on 7seg display
                              };
                              
                              
unsigned int digitNumber;
unsigned int places[3] = {0,0,0};
unsigned int temp;
unsigned int beatNumber;
unsigned int timeIntervalHex;
unsigned int timeIntervalDecimal;
long tempoBPM;
unsigned long tempoDisplay;
int avgArray[2];
int avgArrayIndex;
int avg;
char string[50];
                              
                              
                              
int main(void)
{   
    init();
    while (1) 
    {
        display();
    }
}


/* -----------------
 * Function: display
 * -----------------
 *
 * selects one digit on the 7-segment display at a time and displays a number on the selected digit
 *
 */

void display(void)
{
    for(digitNumber=0;digitNumber<3;digitNumber++)
    {
        PORTC = selectDigit[digitNumber];
        PORTD = numberList[places[digitNumber]];            //places is the hundreds, tens, and ones place of a number      
        _delay_ms(7);                                       /*7 ms seems about the right amount of time for */ 
                                                            /*a smooth transition to the next digit/number  */
    }
}


/* ------------------------
 * Function: getPlaceValues
 * ------------------------
 *
 * calculates the hundred, tens, and ones place of a number and stores them to the places variable
 *
 * n: the number to perform the calculations on to get its hundreds, tens, and ones places
 *
 */

void getPlaceValues(unsigned int n)
{
    temp = n;
    places[0] = temp/100;       //get hundreds place of n and store in first element of places array
    temp = n % 100;             //remove first digit
    places[1] = temp/10;        //get tens place of n and store in second element of places array
    temp = n % 10;              //remove second digit
    places[2] = temp/1;         //get ones place of n and store in third element of places array
        
}


/* -------------------------------
 * Function: calculateTimeInterval
 * -------------------------------
 *
 * calculates the time interval between two successive beats
 *
 */

void calculateTimeInterval(void)
{
    timeIntervalHex = ICR1;                         //read uint16_t hexadecimal timer value in ICR1 into timeIntervalHex
    sprintf(string, "%i", timeIntervalHex);         /*convert hex number into decimal. sprintf makes a string */
                                                    /*   containing the decimal value.                        */
    timeIntervalDecimal = atoi(string);             //convert string into an integer
    tempoBPM = 15625L*60L/timeIntervalDecimal;      /*convert decimal number into beats per minute. Timer increments */
                                                    /* once every 1024 clock cycles, and clock cycle is 16 MHz       */
}

/* ----------------------------
 * Function: updateTempoDisplay
 * ----------------------------
 *
 * takes average of two tempos, and updates the tempo value to be displayed
 *
 */

void updateTempoDisplay(void) 
{
    avgArrayIndex = beatNumber%2;
    avgArray[avgArrayIndex] = tempoBPM;
    
    if(tempoBPM >= 40 && tempoBPM <= 999)           //tempoBPM should never exceed 999 or be less than 40
    {
        if(beatNumber<2)                            /*   For the first two beats, the immediate tempo is displayed.  */
        {                                           /*   Then the average of the last two tempos are displayed       */
            tempoDisplay = tempoBPM;                /*   This is so that the value displayed will not swing so much  */
            beatNumber++;                           /*   for every small error made by the user. We use two          */
        }                                           /*   tempos for the average because this can quickly adapt to    */
        else                                        /*   any major tempo change made by the user.                    */
        {
            avg = (avgArray[0] + avgArray[1])/2;
            tempoDisplay = avg;
            beatNumber++;
        }
    }
}


/* --------------
 * Function: init
 * --------------
 *
 * sets timer mode, prescaler, and noise canceller, enables interrupts, turns on 7 segment display,
 *   and initializes variables
 *
 */

void init(void)
{
    TCCR1B = 0x85;              //Normal mode with a prescaler of 1024 for the 16MHz external clock source, set noise canceller
    TIMSK = TIMSK | 0x08;       //Enable Input Capture interrupt in TIMSK register (set TICIE1) 
    sei();                      //enable global interrupt
    DDRC = 0xFF;                //make port C an output port for 7seg display
    DDRD = 0xFF;                //make port D an output port for 7seg display
    
    tempoDisplay = 0;                   
    beatNumber = 0; 
}


/* --------------------------------------
 * Interrupt Service Routine: TIMER1 CAPT
 * --------------------------------------
 *
 * Runs each time TIMER1 CAPT interrupt is generated
 *
 * Takes timer count at the time of each beat, and calculates time rate of beats in BPM
 * Updates tempo value to be displayed on 7 segment
 * Manually resets counter to 0
 *
 * TIMER1_COMPA_vect: Interrupt vector that generates when user taps the drum
 *
 */

ISR(TIMER1_CAPT_vect)
{
    calculateTimeInterval();
    updateTempoDisplay();
    getPlaceValues(tempoDisplay);
    TCNT1 = 0x0000;                 //reset timer to 0
}

