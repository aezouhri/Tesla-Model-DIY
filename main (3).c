/*
* FinalProject.c
*
* Created: 4/24/2022 8:03:06 PM
* Author : Adnane Ezouhri
*/



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define F_CPU 16000000UL
#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


void USART_Init( unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char) ubrr;
	
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	
	//Only 1 stop bit but 8 data No Parity
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);
}
int TimerOverflow=0;

ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;						/* Increment Timer Overflow count */
}
unsigned char garbage= 'g';

double ultra_sonic(void)
{

	char string[10]={NULL};
	double count=0;
	double distance=0;
	
	
	PORTB |= (1 << PORTB3);					/*Setting Trigger Pin high*/
	_delay_us(10);							/*Wait 10 us to trigger the burst of an ultrasound*/
	PORTB &= (~(1 << PORTB3));				/*Setting Trigger Pin low*/
	

	TCNT1 = 0;								/* Clear Timer counter */
	TCCR1B = 0x41;							/* Capture on rising edge, No pre-scaler*/
	TIFR1 = 1<<ICF1;						/* Clear ICP flag (Input Capture flag) */
	TIFR1 = 1<<TOV1;						/* Clear Timer Overflow flag */

	/*Calculate width of Echo by Input Capture (ICP) */
	
	while ((TIFR1 & (1 << ICF1)) == 0);		/* Wait for rising edge */
	TCNT1 = 0;								/* Clear Timer counter */
	TCCR1B = 0x01;							/* Capture on falling edge, No pre-scaler */
	TIFR1 = 1<<ICF1;						/* Clear ICP flag (Input Capture flag) */
	TIFR1 = 1<<TOV1;						/* Clear Timer Overflow flag */
	
	TimerOverflow = 0;						/* Clear Timer overflow count */
	while ((TIFR1 & (1 << ICF1)) == 0);		/* Wait for falling edge */
	
	count = ICR1 + (255 * TimerOverflow);	/* Take count */
	
	/* 16MHz Timer freq, sound speed =343 m/s */
	distance = ((double)count * 343) / 320000;

	dtostrf(count,2,2,string);
	strcat(string," cm");

	if(count>30000)
	{
		distance=50;
	}
	return distance;
}
unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	{
		double distance = ultra_sonic();
		
		//USART_Transmit('t');
		if(distance < 10.00 && distance > 0.93)
		{
			turnRight();
			distance=ultra_sonic();
			//_delay_ms(200);
			if(distance>=10.00)
			{
				stop();
				
			}
		}

	}
	/* Get and return received data from buffer */
	return UDR0;
}


char* ReceivedTerm(void)
{
	char data[20];
	int i=0;
	while(i<5)
	{
		data[i]=USART_Receive();
		i+=1;
	}
	return data;
}



void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)));
	/* Put data into buffer, sends the data */
	UDR0 = data;
}
void WriteTerm(char data[] )
{
	int i=0;
	while(data[i]!=NULL)
	{
		USART_Transmit(data[i]);
		i=i+1;
	}
}





void moveForward()
{
	stop();
	//_delay_ms(200);
	frontLeftForward();
	frontRightForward();
	backLeftForward();
	backRightForward();
}

void moveBackwards()
{
	stop();
	//_delay_ms(8000);
	frontLeftBackwards();
	frontRightBackwards();
	backRightBackwards();
	backtLeftBackwards();
	
}
	
void frontLeftForward()
{
	PORTC &= ~ (1<<PORTC3);
	PORTC |=   (1<<PORTC2);
}
void frontLeftBackwards()
{
	PORTC |=   (1<<PORTC3);
	PORTC &= ~ (1<<PORTC2);
}
void backtLeftBackwards()
{
	PORTD |=   (1<<PORTD4);
	PORTD &= ~ (1<<PORTD5);
}
void backLeftForward()
{
	PORTD |=   (1<<PORTD5);
	PORTD &= ~ (1<<PORTD4);
}
void frontRightForward()
{
	PORTC |=   (1<<PORTC4);
	PORTC &= ~ (1<<PORTC5);
}
void frontRightBackwards()
{
	PORTC |=   (1<<PORTC5);
	PORTC &= ~ (1<<PORTC4);
}

void backRightBackwards()
{
	PORTD &= ~ (1<<PORTD7);
	PORTD |=   (1<<PORTD6);
}

void backRightForward()
{
	PORTD &= ~ (1<<PORTD6);
	PORTD |=   (1<<PORTD7);
}

void turnRight()
{
	stop();
	//_delay_ms(200);
	frontLeftForward();
	backLeftForward();
	backRightBackwards();
}
void turnLeft()
{
	stop();
	//_delay_ms(200);
	frontRightForward();
	backRightForward();
	backtLeftBackwards();
}

void stop()
{

	PORTC &= ~  0b111100;
	PORTD &= ~  0b11110000;
}
//Pb1 input 1
//PD6 - input 2
//Pd5 - input +
//Pb2,Pb4
//Pb0
// yellow high
// green low
int main()
{
	USART_Init(MYUBRR);
	char text[4]={'0'};
	DDRB |= 0B01000;
	DDRD |= 0B111100000;
	DDRC |= 0B111100;
	
	//front right Wheel Green- PC4 , Yellow- PC5
	//front left Wheel  Green- PC2 , Yellow- PC3
	
	//back right Wheel, Green- PD6 , Yellow- PD5
	//back left wheel, Green-  PD4 , Yellow- PD5
	
	PORTC &= ~  0b00000000;
	PORTD &= ~  0b00000000;
	
	sei();									/* Enable global interrupt */
	TIMSK1 = (1 << TOIE1);					/* Enable Timer1 overflow interrupts */
	TCCR1A = 0;								/* Set all bit to zero Normal operation */
	
	char command=NULL;
	char temp=NULL;
	
	while (1) {
			
		command= USART_Receive();
			
			
			if(command=='f')
			{
				moveForward();
			}
			
			if(command=='b' )
			{
				moveBackwards();
			}
			
			if(command=='l' )
			{
				turnLeft();
			}
			
			if(command=='r' )
			{
				turnRight();
			}
			
			if(command=='s' )
			{
				stop();
			}

				
	}

}

