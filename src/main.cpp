#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include <Arduino.h>
#include "serialATmega.h"


#define NUM_TASKS 3 //TODO: Change to the number of tasks being used
unsigned char i;
unsigned char j;

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long Left_Period = 1000;
const unsigned long Right_Period = 1000;
const unsigned long Horn_Period = 1000;
const unsigned long GCD_PERIOD = findGCD(Right_Period, Left_Period);//TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks


void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}


int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};//Stepper motor phases

//TODO: Create your tick functions for each task
enum left_state{idle_left, Left_One, Left_Two, Left_Three};
int TickFtn_left(int state);

enum right_state{idle_right, Right_One, Right_Two, Right_Three};
int TickFtn_right(int state);

enum horn_period{horn_off, horn_on};
int TickFtn_horn(int state);

int main(void) {
    //TODO: initialize all your inputs and ouputs

    ADC_init();   // initializes ADC
    //  Output: DDR = 1, PORT = 0
    //  Input: DDR = 0, PORT = 1
    DDRC = 0b000000; PORTC = 0b111111;
    DDRB = 0b111111; PORTB = 0b000000;
    DDRD = 0b11111111; PORTD = 0b00000000;
    serial_init(9600);

    //TODO: Initialize the buzzer timer/pwm(timer0)
    OCR0A = 128; //sets duty cycle to 50% since TOP is always 256

    //TODO: Initialize the servo timer/pwm(timer1)


    //TODO: Initialize tasks here
    // e.g. 
    // tasks[0].period = ;
    // tasks[0].state = ;
    // tasks[0].elapsedTime = ;
    // tasks[0].TickFct = ;
    // task 1
    unsigned char i  = 0;
    tasks[i].state = idle_left;
    tasks[i].period = Left_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_left;
    i++;

    // task 2
    tasks[i].state = idle_right;
    tasks[i].period = Right_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_right;
    i++;

    //task 3
    tasks[i].state = horn_off;
    tasks[i].period = Horn_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_horn;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}

// enum left_state{idel_left, Left_One, Left_Two, Left_Three};
int TickFtn_left(int state){
    switch (state)
    {
    case idle_left:
    // does not require the ! mark when pressed check code to see for bugs
        if(((PINC >> 3) & 0x01)){
            i = 0;
            state = Left_One;
        }
        else{
            i = 0;
            state = idle_left;
        }
    break;

    case Left_One:
        if(i < 1){
        state = Left_One;
        }
        else if(i >= 1 && ((PINC >> 3) & 0x01)){
            i = 0;
            state = Left_Two;
        }
        else if(!((PINC >> 3) & 0x01)){
            i = 0;
            state = idle_left;
        }
    break;
    
    case Left_Two:
        if(i < 1){
            state = Left_Two;
        }
        if(i >= 1 && ((PINC >> 3) & 0x01)){
            i = 0;
            state = Left_Three;
        }
        else if(!((PINC >> 3) & 0x01)){
            i = 0;
            state = idle_left;
        }
    break;

    case Left_Three:
        if(i < 2){
            state = Left_Three;
        }
        else if(i >= 2 && ((PINC >> 3) & 0x01)){
            state = Left_One;
        }
        else if(!((PINC >> 3) & 0x01)){
            state = idle_left;
        }
    break;

    default:
        break;
    }

    switch (state)
    {
    case idle_left:
        PORTB = SetBit(PORTB,0,0);
        PORTD = SetBit(PORTD,7,0);
        PORTD = SetBit(PORTD,5,0);
    break;

    case Left_One:
        PORTB = SetBit(PORTB,0,1);
        if(i < 1){
            i++;
        }
    break;
    
    case Left_Two:
        PORTD = SetBit(PORTD,7,1);
        if(i < 1){
            i++;
        }
    break;

    case Left_Three:
        PORTD = SetBit(PORTD,5,1);
        if(i < 1){
            i++;
        }
        else if(i >= 1){
            PORTB = SetBit(PORTB,0,0);
            PORTD = SetBit(PORTD,7,0);
            PORTD = SetBit(PORTD,5,0);
            i++;
        }
    break;

    default:
        break;
    }
    return state;
}

// // enum left_state{idle_right, Right_One, Right_Two, Right_Three};
int TickFtn_right(int state){
        switch (state)
    {
    case idle_right:
    // does not require the ! mark when pressed check code to see for bugs
        if(((PINC >> 4) & 0x01)){
            j = 0;
            state = Right_One;
        }
        else{
            j = 0;
            state = idle_left;
        }
    break;

    case Right_One:
        if(j < 1){
        state = Right_One;
        }
        else if(j >= 1 && ((PINC >> 4) & 0x01)){
            j = 0;
            state = Right_Two;
        }
        else if(!((PINC >> 4) & 0x01)){
            j = 0;
            state = idle_right;
        }
    break;
    
    case Right_Two:
        if(j < 1){
            state = Right_Two;
        }
        if(j >= 1 && ((PINC >> 4) & 0x01)){
            j = 0;
            state = Right_Three;
        }
        else if(!((PINC >> 4) & 0x01)){
            j = 0;
            state = idle_right;
        }
    break;

    case Right_Three:
        if(j < 2){
            state = Right_Three;
        }
        else if(j >= 2 && ((PINC >> 4) & 0x01)){
            state = Right_One;
        }
        else if(!((PINC >> 4) & 0x01)){
            state = idle_right;
        }
    break;

    default:
        break;
    }

    switch (state)
    {
    case idle_right:
        PORTD = SetBit(PORTD,4,0);
        PORTD = SetBit(PORTD,3,0);
        PORTD = SetBit(PORTD,2,0);
    break;

    case Right_One:
        PORTD = SetBit(PORTD,4,1);
        if(j < 1){
            j++;
        }
    break;
    
    case Right_Two:
        PORTD = SetBit(PORTD,3,1);
        if(j < 1){
            j++;
        }
    break;

    case Right_Three:
        PORTD = SetBit(PORTD,2,1);
        if(j < 1){
            j++;
        }
        else if(j >= 1){
            PORTD = SetBit(PORTD,4,0);
            PORTD = SetBit(PORTD,3,0);
            PORTD = SetBit(PORTD,2,0);
            j++;
        }
    break;

    default:
        break;
    }
    return state;
}

// enum horn_period{horn_off, horn_on};
int TickFtn_horn(int state){
    switch (state)
    {
    case horn_off:
        if(!((PINC >> 2)&0x01)){
            state = horn_on;
        }
        else{
            state = horn_off;
        }
        break;
    
    case horn_on:
        if(!((PINC >> 2)&0x01)){
            state = horn_on;
        }
        else if((PINC >> 2)&0x01){
            state = horn_off;
        }
    default:
        break;
    }

    switch (state)
    {
    case horn_off:
        TCCR0A |= (1 << COM0A1);// use Channel A
        TCCR0A |= (1 << WGM01) | (1 << WGM00);// set fast PWM Mode
        TCCR0B = (TCCR0B & 0xF8) | 0x00; //set prescaler to 8
        break;
    
    case horn_on:
        TCCR0A |= (1 << COM0A1);// use Channel A
        TCCR0A |= (1 << WGM01) | (1 << WGM00);// set fast PWM Mode
        TCCR0B = (TCCR0B & 0xF8) | 0x04; //set prescaler to 8
        break;
    default:
        break;
    }
    return state;
}