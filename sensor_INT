/*
 * sensor.c
 *
 *  Created on: 18/11/2018
 *      Author: DiogoAndré
 */

#include <avr/io.h>
#include <util/delay.h>
#include "timer_tools.h"
#include "serial_printf.h"
#include <avr/interrupt.h>

#define DEBUG


#define TRIG PB1
#define ECHO PD2


/* 1us = 2 CNT ; 65535 CNT = 32.5 ms   TP = 8 */
#define T1BOTTOM 0

volatile int state;
volatile long time;
volatile long distancia;

ISR(INT0_vect)
{

	TCNT1=T1BOTTOM;
	while((PIND & (1<<ECHO))){

	}

	state=2;
}

/********************************************************
 * Timer 1 -> NORMAL mode
 *******************************************************/
void tc1_init(void){
	TCCR1B = 0; // Stop TC1
	TIFR1 = (7<<TOV1) | (1<<ICF1); // Clear all pending interrupts
	TCCR1A = 0; // NORMAL mode
	TCNT1 = T1BOTTOM; // Load BOTTOM value
	TIMSK1 = (0<<TOIE1); // Enable Ovf intrpt
	TCCR1B = 2; // Start TC1 (TP=8)

}

/********************************************************
 * INICIALIZAÇÃO
 *****************************************************/
void init(void){
  DDRB = DDRB | ((1<<TRIG)) ; //DEFINIR OUTPUTS E INPUTS
  DDRD = DDRD & ~(1<<ECHO);
  PORTB=0;
  PORTD=0;
  EICRA = EICRA | (3<<ISC00); //Interrupts request at rising edge
  EIMSK = EIMSK | (1<<INT0);  // Enables INT0
  EIFR= EIFR | (1<<INTF0); // Clear
  sei(); // Enable global interrupt flag

}


int main(void){

	  /*INICIALIZAÇÃO*/
	  init();
	  printf_init();
	  tc1_init();
	  _delay_ms(2000);
	  printf("ready\n");

	  while(1){

		switch(state){
		case 0:
			 PORTB=PORTB | (1<<TRIG); //Ligar TRIG
			 _delay_us(20);
			 PORTB=PORTB & ~(1<<TRIG); //Desligar TRIG
			 state=1;

			 break;
		case 1:
			break;
		case 2:
				time=TCNT1/2;
				distancia = (time/2)*0.034;
				printf("TEMPO -> %ld\n DISTANCIA -> %ld\n", time, distancia);
				state=3;
			break;
		case 3:
			_delay_ms(1000);
			state=0;
		  }
	  }
	return 1;
}
