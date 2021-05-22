/* Author: Karan Bhogal
 * Lab Section: 21
 * Assignment: Lab #10  Exercise #2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 *
 *  Demo Link:
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;

unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80;
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


unsigned char GetBit (unsigned char port, unsigned char number){
	return ( port & ( 0x01 << number));
}

unsigned char GetKeypadKey() {
	PORTC = 0xEF;
	asm("nop");
	if (GetBit((PINC),0) == 0) { return('1'); }
	if (GetBit((PINC),1) == 0) { return('4'); }
	if (GetBit((PINC),2) == 0) { return('7'); }
	if (GetBit((PINC),3) == 0) { return('*'); }

	PORTC = 0xDF;
	asm("nop");
	if (GetBit((PINC),0) == 0) { return('2'); }
	if (GetBit((PINC),1) == 0) { return('5'); }
	if (GetBit((PINC),2) == 0) { return('8'); }
	if (GetBit((PINC),3) == 0) { return('0'); }

	PORTC = 0xBF;
	asm("nop");
	if (GetBit((PINC),0) == 0) { return('3'); }
	if (GetBit((PINC),1) == 0) { return('6'); }
	if (GetBit((PINC),2) == 0) { return('9'); }
	if (GetBit((PINC),3) == 0) { return('#'); }

	PORTC = 0x7F;
        asm("nop");
        if (GetBit((PINC),0) == 0) { return('A'); }
        if (GetBit((PINC),1) == 0) { return('B'); }
        if (GetBit((PINC),2) == 0) { return('C'); }
        if (GetBit((PINC),3) == 0) { return('D'); }

	return('\0');

}

typedef struct task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

//shared variables:
unsigned char led0_output = 0x00;
unsigned char led1_output = 0x00;
unsigned char l7out = 0x00;
unsigned char pause = 0;
//done variables

enum pauseButtonSM_States { pauseButton_wait, pauseButton_press, pauseButton_release };

int pauseButtonSMTick(int state) {
	unsigned char press = ~PINA & 0x01;

	switch (state) {
		case pauseButton_wait:
			state = press == 0x01? pauseButton_press: pauseButton_wait; break;
		case pauseButton_press:
			state = pauseButton_release; break;
		case pauseButton_release:
			state = press == 0x00? pauseButton_wait: pauseButton_press; break;
		default: state = pauseButton_wait; break;
	}

	switch (state) {
		case pauseButton_wait:break;
		case pauseButton_press:
			pause = (pause == 0) ? 1 : 0; //toggle pause
			break;
		case pauseButton_release: break;
	}
	return state;
}

enum toggleLED0_States { toggleLED0_wait, toggleLED0_blink };

int toggleLED0SMTick(int state) {
	switch (state) {
		case toggleLED0_wait: state = !pause? toggleLED0_blink: toggleLED0_wait; break;
		case toggleLED0_blink: state = pause? toggleLED0_wait: toggleLED0_blink; break;
		default: state = toggleLED0_wait; break;
	}
	switch (state) {
		case toggleLED0_wait: break;
		case toggleLED0_blink:
			led0_output = (led0_output == 0x00) ? 0x01 : 0x00;
			break;
	}
	return state;
}

enum toggleLED1_States { toggleLED1_wait, toggleLED1_blink };

int toggleLED1SMTick(int state) {
	switch (state) {
		case toggleLED1_wait: state = !pause? toggleLED1_blink: toggleLED1_wait; break;
		case toggleLED1_blink: state = pause? toggleLED1_wait: toggleLED1_blink; break;
		default: state = toggleLED1_wait; break;
	}
	switch (state) {
		case toggleLED1_wait: break;
		case toggleLED1_blink:
				     led1_output = (led1_output == 0x00) ? 0x01 : 0x00;
				     break;
	}
	return state;
}


enum display_States { display_display };

int displaySMTick(int state) {
	unsigned char output;

	switch (state) {
		case display_display: state = display_display; break;
		default: state = display_display; break;
	}
	switch (state) {
		case display_display:
			output = (led0_output) | (led1_output << 1);

			break;
	}
	PORTB = output | l7out;
	return state;
}


enum exer_2 { wait, r, p1, r1, p2, r2, p3, r3, p4 , r4, p5, r5, lock };

int runexer_2(int state) {
	unsigned char x = GetKeypadKey();
	unsigned char z = ~PINA;
	switch (state) {
		case wait:
			
			if ( x == '#' ){
				state = r;
			}
			else if (z == 0x01 ){
				state = lock;
			}
			else{

				state = wait;
			}
			break;
		case r: 
			if (x == '#' ) {
				state = r;
			}
			else{
				state = p1;
			}
			break;
		case p1:
			if ( x == '1' ){
				state = r1;
			}
			else if (z == 0x01){
				state = lock;
			}
			else if ( x == '#'){
                                state = wait;
                        }
			else{
				state = p1;
			}
			break;
		case r1:
			if ( x == '1' ){
				state = r1;
			}
			else{
				state = p2;
			}
			break;
		//c2
		case p2:
                        if ( x == '2' ){
                                state = r2;
                        }
                        else if (z == 0x01){
                                state = lock;
                        }
			else if ( x == '#'){
                                state = wait;
                        }

                        else{
                                state = p2;
                        }
                        break;
                case r2:
                        if ( x == '2' ){
                                state = r2;
                        }
                        else{
                                state = p3;
                        }
                        break;
		//c3
		case p3:
                        if ( x == '3' ){
                                state = r3;
                        }
                        else if (z == 0x01){
                                state = lock;
                        }
			else if ( x == '#'){
                                state = wait;
                        }

                        else{
                                state = p3;
                        }
                        break;
                case r3:
                        if ( x == '3' ){
                                state = r3;
                        }
                        else{
                                state = p4;
                        }
                        break;
		//c4
		case p4:
                        if ( x == '4' ){
                                state = r4;
                        }
                        else if (z == 0x01){
                                state = lock;
                        }
			else if ( x == '#'){
                                state = wait;
                        }
                        else{
                                state = p4;
                        }
                        break;
                case r4:
                        if ( x == '4' ){
                                state = r4;
                        }
                        else{
                                state = p5;
                        }
                        break;
		//c5
		case p5:
                        if ( x == '5' ){
                                state = r5;
                        }
                        else if (z == 0x01){
                                state = lock;
                        }
			else if ( x == '#'){
				state = wait;
			}
                        else{
                                state = p5;
                        }
                        break;
                case r5:
			PORTB = 0x01;
                        if ( x == '5' ){
                                state = r5;
                        }
                        else{
                                state = p5;
                        }
                        break;
		case lock:
			PORTB = 0x00;
			if ( z == 0x01){
				state = lock;
			}
			else{
				state = wait;
			}
			break;


	}
	
	return state;
}



unsigned long int findGCD (unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a=b;
		b=c;
	}
	return 0;
}


void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency){
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }

		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }

		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on(){
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off(){
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

unsigned int sc = 1;

double ar[6] = { 293.66, 349.23, 293.66, 261.63, 293.66, 349.23};
unsigned int l = 0;

enum LA_State { Start, Song, Stop };

int Speaker(int state)
{
	unsigned char num = ~PINA;

	switch (state){
	   case Start:
		//PORTB = 0x02;
		if ( num == 0x80 ){
			state = Song;
		}
		else{
			state = Start;
		}
		break;

	  case Song:
		//PORTB = 0xFF;
		if ( l != 6 ){
			l = l + 1;
			state = Song;
		}
		else {
			l = 0;
			state = Stop;
		}
		break;

	case Stop:
		//PORTB = 0x04;
		if ( num == 0x80 ){
			state = Stop;
		}
		else{
			state = Start;
		}
		break;
	}


	switch (state){
	   case Start:
		break;

	case Song:
		set_PWM(ar[l]);
		break;

	case Stop:
		set_PWM(0);
		break;

	}
	return state;
}




int main(void) {

    /* Insert DDR and PORT initializations */
	DDRA = 0x00;	PORTA = 0xFF;
	DDRC = 0xF0;	PORTC = 0x0F;
	DDRB = 0xFF;	PORTB = 0x00;
    /* Insert your solution below */
	static task task1, task2;
	task *tasks[] = { &task1, &task2 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	PWM_on();
	//const char start = -1;
	
	task1.state = wait;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &runexer_2;

	task2.state = Start;
	task2.period = 400;
	task2.elapsedTime = task2.period;
	task2.TickFct = &Speaker;
	
	unsigned short i;
	unsigned long GCD = tasks[0] -> period;
        for (i = 1;i < numTasks; i++){
                GCD = findGCD(GCD, tasks[i]->period);
        }

	TimerSet(GCD);
	TimerOn();

	

    while (1) {

	for ( i = 0; i < numTasks; i++) {
		if ( tasks[i] ->elapsedTime >= tasks[i]->period ) {
			tasks[i] -> state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i] -> elapsedTime += GCD;
	}
	while (!TimerFlag);
	TimerFlag = 0;

	/*
	a = GetKeypadKey();
	   switch(a){
		case '\0': PORTB = 0x1F; break;
		case '1': PORTB = 0x01; break;
		case '2': PORTB = 0x02; break;
		case '3': PORTB = 0x03; break;
		case '4': PORTB = 0x04; break;
		case '5': PORTB = 0x05; break;
		case '6': PORTB = 0x06; break;
		case '7': PORTB = 0x07; break;
		case '8': PORTB = 0x08; break;
		case '9': PORTB = 0x09; break;
		case 'A': PORTB = 0x0A; break;
		case 'B': PORTB = 0x0B; break;
		case 'C': PORTB = 0x0C; break;
		case 'D': PORTB = 0x0D; break;
		case '*': PORTB = 0x0E; break;
		case '0': PORTB = 0x00; break;
		case '#': PORTB = 0x0F; break;
		default: PORTB = 0x1B; break;
	   } */

    }
    return 0;
    
}
