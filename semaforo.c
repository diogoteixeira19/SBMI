/********************************************************
 *  semaforo.c
 * 	Traffic lights with an emergence function (C)
 * 	Solution: state machine with an interrupt request and
 * 		  timer TC1
 ********************************************************
 *
 *  F=16MHz. To create a interrupt request every
 *  10ms we need to count 16M*10m=160000 clock periods.
 *  Combination of CP TP and CNT to find a solution to
 *  CPxTPxCNT=160000, CP=1
 *
 *  TP=1 -> CNT=160000, CNT too high
 *  TP=8 -> CNT=20000
 *  TP=32 -> CNT=5000, TC2 only, 16bit timer CNT>255
 *  TP=64 -> CNT=2500
 *  TP=128 -> CNT=1250, TC2 only, 16bit timer CNT>255
 *  TP=256 -> CNT=625
 *  TP=1024 -> CNT=156.1, timing error
 *  (3 possible combinations)
 *
 *  CP=1, TP=64, CNT=2500
 *
 *  NORMAL MODE TC1:
 *  TC1 starts at a given BOTTOM value and counts up to
 *  65535, and overflows, every time TC1 overflows
 *  ISR is executed (every 10ms)
 *
 ********************************************************
 *  Created on: 14/10/2018
 *      Author: Diogo Teixeira e Bernardo Moreira
 *
 */

#include <avr/io.h>
#include <util/delay.h>
#include "timer_tools.h"
#include "serial_printf.h"
#include <avr/interrupt.h>

#define DEBUG

#define RNS PB0
#define YNS PB1
#define GNS PB2
#define REW PB3
#define YEW PB4
#define GEW PB5

#define EMR PD2

#define bigT 5000  /* 50 sec */
#define smallT 500  /* 5 sec */
#define mediumT 1000  /* 10 sec */

/* 10ms = 2500 clock cycles @ 16MHz/(1*64) */
#define T1BOTTOM 65536-2500

volatile unsigned char state=0, pstate=0;
volatile int t;

/**************************************************
 * INTERRUPT is executed at falling edge button (EMERGENCIA)
 **************************************************
 * If state indicates normal function then INTERRUPT
 * switches to EMERGENCIA, if already in EMERGENCIA
 * does nothing
 ************************************************/
ISR(INT0_vect)
{
	if(state<7)
	{
		state=17;
	}
}

/********************************************************
 *  Timer 1 ISR is executed each 10ms
 ********************************************************
 *	Reload BOTTOM value
 *  Decrement t
 ********************************************************/
ISR(TIMER1_OVF_vect){
	TCNT1 = T1BOTTOM; // reload TC1
	t--;
}

/********************************************************
 * Timer 1 -> NORMAL mode
 *****************************************************
 *  Stop TC1 and clear all pending interrupts
 *  Define mode of operation & BOTTOM value
 *  Set the required interrupt mask
 *  Start timer with the proper prescaler
 ******************************************************/
void tc1_init(void){
	TCCR1B = 0; // Stop TC1
	TIFR1 = (7<<TOV1) | (1<<ICF1); // Clear all pending interrupts
	TCCR1A = 0; // NORMAL mode
	TCNT1 = T1BOTTOM; // Load BOTTOM value
	TIMSK1 = (1<<TOIE1); // Enable Ovf intrpt
	TCCR1B = 3; // Start TC1 (TP=64)

}

/********************************************************
 * INICIALIZAÇÃO
 *****************************************************
 *  Define outputs
 *  Define inputs
 *  Set interrupts request at falling edge
 *  Enables INT0
 *  Enable global interrupt flag
 *  Clear interrupt requests
 ******************************************************/
void init(void){
  DDRB = DDRB | 0b00111111; //DEFINIR OUTPUTS
  DDRD = DDRD | 0b00000000; //DEFINIR INPUT
  EICRA = EICRA | (2<<ISC00); //Interrupts request at falling edge
  EIMSK = EIMSK | (1<<INT0);  // Enables INT0
  EIFR= EIFR | (1<<INTF0); // Clear
  sei(); // Enable global interrupt flag
}

/********************************************************
 * State machine in main
 *******************************************************/
int main(void)
{

  /*INICIALIZAÇÃO*/
  init();
  printf_init();


  while (1)
  {
			 if (state==0){
				PORTB = (PORTB & ~PORTB) | (1<<GNS) | (1<<REW);  // VERDE  -   VERMELHO
				t=bigT;
				tc1_init();
				state=1;
				pstate=1;
				#ifdef DEBUG
				printf("\n estado %d -> VERDE - VERMELHO\n", state);
				#endif
			}
			if ((state==6 && (t==0) )  || ( state==13 && (t==0) ) || ( state==15 && (t==0) ) || ( state==16 && (t==0) )  ){
				PORTB = (PORTB & ~PORTB) | (1<<GNS) | (1<<REW);  // VERDE  -   VERMELHO
				t=bigT;
				tc1_init();
				state=1;
				pstate=1;
				#ifdef DEBUG
				printf("\n estado %d -> VERDE - VERMELHO\n", state);
				#endif
			}
			if (state==1 && (t==0)){
				PORTB = ( PORTB ^ (1<<GNS) ) | (1<<YNS);        // AMARELO -   VERMELHO
				t=smallT;
				tc1_init();
				state=2;
				pstate=2;
				#ifdef DEBUG
				printf("\n estado %d -> AMARELO - VERMELHO\n", state);
				#endif
			}
			if( state==2 && (t==0) ){
				PORTB = ( PORTB ^ (1<<YNS) ) | (1<<RNS);        // VERMELHO -  VERMELHO
				t=smallT;
				tc1_init();
				state=3;
				pstate=3;
				#ifdef DEBUG
				printf("\n estado %d -> VERMELHO - VERMELHO\n", state);
				#endif
			}
			if( (state==3 && (t==0) ) || (state==8 && (t==0)) || ( state==10 && (t==0)) || ( state==11 && (t==0))){
				PORTB = ( PORTB ^ (1<<REW) ) | (1<<GEW);        // VERMELHO -  VERDE
				t=bigT;
				tc1_init();
				state=4;
				pstate=4;
				#ifdef DEBUG
				printf("\n estado %d -> VERMELHO - VERDE\n", state);
				#endif
			}
			if(state==4 && (t==0)){
				PORTB = ( PORTB ^ (1<<GEW) ) | (1<<YEW);        // VERMELHO - AMARELO
				t=smallT;
				tc1_init();
				state=5;
				pstate=5;
				#ifdef DEBUG
				printf("\n estado %d -> VERMELHO - AMARELO\n", state);
				#endif
			}
			if(state==5 && (t==0) ){
				PORTB = ( PORTB ^ (1<<YEW) ) | (1<<REW);        // VERMELHO - VERMELHO
				t=smallT;
				tc1_init();
				state=6;
				pstate=6;
				#ifdef DEBUG
				printf("\n estado %d -> VERMELHO - VERMELHO\n", state);
				#endif
			}
	        	/******************************* - EMERGENCIA - ********************************/
			// estado 17
			// estado 1
			if( (state==17) && (pstate==1)){
				PORTB = ( PORTB ^ (1<<GNS) ) | (1<<YNS);        // AMARELO - VERMELHO
				t=smallT;
				tc1_init();
				state=7;
				pstate=0;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> AMARELO - VERMELHO\n", state);
				#endif
			}
			if(state==7 && (t==0)){
				PORTB = ( PORTB ^ (1<<YNS) ) | (1<<RNS);        //VERMELHO - VERMELHO
				t=mediumT;
				tc1_init();
				state=8;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> VERMELHO - VERMELHO\n", state);
				#endif
			}
		   //estado 2
			if( (state==17) && (pstate==2) ){			//AMARELO - VERMELHO
				state=9;
				pstate=0;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> AMARELO - VERMELHO\n", state);
				#endif
			}
			if( (state==9) && (t==0)){
				PORTB = ( PORTB ^ (1<<YNS) ) | (1<<RNS);	//VERMELHO - VERMELHO
				t=mediumT;
				tc1_init();
				state=10;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> VERMELHO - VERMELHO\n", state);
				#endif
			}
			//estado 3
			if( (state==17) && (pstate==3)){
				t=mediumT;					//VERMELHO - VERMELHO
				tc1_init();
				state=11;
				pstate=0;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> VERMELHO - VERMELHO\n", state);
				#endif
			}
			// estado 4
			if( (state==17) && (pstate==4)){
				PORTB = ( PORTB ^ (1<<GEW) ) | (1<<YEW);        // VERMELHO - AMARELO
				t=smallT;
				tc1_init();
				state=12;
				pstate=0;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> VERMELHO - AMARELO\n", state);
				#endif
			}
			if(state==12 && (t==0)){
				PORTB = ( PORTB ^ (1<<YEW) ) | (1<<REW);        //VERMELHO - VERMELHO
				t=mediumT;
				tc1_init();
				state=13;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> VERMELHO - VERMELHO\n", state);
				#endif
			}
		   //estado 5
			if( (state==17) && (pstate==5)){			//VERMELHO - AMARELO
				state=14;
				pstate=0;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> VERMELHO - AMARELO\n", state);
				#endif
			}
			if( (state==14) && (t==0)){
				PORTB = ( PORTB ^ (1<<YEW) ) | (1<<REW);	//VERMELHO - VERMELHO
				t=mediumT;
				tc1_init();
				state=15;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> VERMELHO - VERMELHO\n", state);
				#endif
			}
			//estado 6
			if( (state==17) && (pstate==6)){
				t=mediumT;					//VERMELHO - VERMELHO
				tc1_init();
				state=16;
				pstate=0;
				#ifdef DEBUG
				printf("\nEMERGENCIA estado %d -> VERMELHO - VERMELHO\n", state);
				#endif
			}
		}

  return 1;
}
