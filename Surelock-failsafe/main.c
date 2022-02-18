/*
 * Surelock-failsafe.c
 *
 * Created: 30/06/2021 15:15:10
 * Author : markr
 */ 

#define F_CPU 4000000
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "main.h"


inline void setB0() {PORTB |= 1<<PORTB0;}
inline void clearB0() {PORTB &= ~(1<<PORTB0);}
inline void setOutputB0() {DDRB |= 1<<DDB0;}
inline void setInputB0() {DDRB &= ~(1<<DDB0);}
inline void setPUB0() {PUEB |= 1<<PUEB0;}
inline void setB1() {PORTB |= 1<<PORTB1;}
inline void clearB1() {PORTB &= ~(1<<PORTB1);}
inline void setOutputB1() {DDRB |= 1<<DDB1;}
inline void setInputB1() {DDRB &= ~(1<<DDB1);}
inline void setPUB1() {PUEB |= 1<<PUEB1;}
inline void setB2() {PORTB |= 1<<PORTB2;}
inline void clearB2() {PORTB &= ~(1<<PORTB2);}
inline bool getB2() {return ((PINB>>PINB2) & 1); }
inline void setOutputB2() {DDRB |= 1<<DDB2;}
inline void setInputB2() {DDRB &= ~(1<<DDB2);}
inline void setPUB2() {PUEB |= 1<<PUEB2;}
inline void setADCChannel(uint8_t channel) {ADMUX = channel;}
inline void setADCClockPrescaler(uint8_t prescaleValue) {ADCSRA &= 0xF8; ADCSRA |= prescaleValue<<ADPS0;}
inline void enableADC() {ADCSRA |= 1<<ADEN;}
inline void startADCConversion() {ADCSRA |= 1<<ADSC;}
inline void waitADCCompletion() { while (ADCSRA & 1<<ADSC);}
inline uint8_t	getADCValue() {return ADCL;}

inline void indicateFault(void) { setB1(); }
inline void resetFaultIndicator(void) {clearB1();}
inline void disableCoilController(void) {clearB2();setOutputB2();}
inline void enableCoilController(void) {setInputB2();}

void initClock( void ) {
	//set clock to 8Mhz
	CCP = 0xD8;
	CLKPSR =  1;
}

void initialiseCoilControllerDisable( void ) {
	setInputB2();		//let the external pull up resistor do the work
						// to keep it inactive
}


void initialiseFaultIndicator() {
	setOutputB1();		// Fault Indication Pin
	clearB1();			// set into the inactive state, ie 0
}


void initCurrentMeasurement( void ) {
	setADCChannel(0);
	setADCClockPrescaler(3); // gives 1Mhz / 8 = 125kHz using default system clock settings
	enableADC();
}

uint8_t getCurrentValue(){
	startADCConversion();
	waitADCCompletion();
	return getADCValue();
}

uint8_t arrCurrent[N_CURRENT_SAMPLES];
uint8_t posCurrent = 0;
uint8_t newAvgCurrent = 0;
uint16_t sumCurrent = 0;
uint8_t lenCurrent = sizeof(arrCurrent) / sizeof(uint8_t);


uint16_t movingAvg(uint8_t* ptrArrNumbers, uint16_t* ptrSum, uint8_t pos, uint8_t len, uint8_t nextNum)
{
	//Subtract the oldest number from the prev sum, add the new number
	*ptrSum = *ptrSum - ptrArrNumbers[pos] + nextNum;
	//Assign the nextNum to the position in the array
	ptrArrNumbers[pos] = nextNum;
	//return the average
	return *ptrSum / len;
}

void sampleAverageCurrent(void)
{
	newAvgCurrent = movingAvg(arrCurrent, &sumCurrent, posCurrent, lenCurrent, getCurrentValue());
	posCurrent++;
	if (posCurrent >= lenCurrent) {
		posCurrent = 0;
	}
}

uint16_t getAverageCurrent() {
	return newAvgCurrent;
}

bool currentHigh = false;

bool isOverCurrent( void ) {
	
uint8_t current = getAverageCurrent();
bool ret = false;

    if ( current >= OVER_CURRENT_VALUE )
    {
	    ret = true;
	    currentHigh = true;
    }
    else if ( current <= (OVER_CURRENT_VALUE - HYSTERYSIS) )
    {
	    ret = false;
	    currentHigh = false;
    }
    else if ( (current < OVER_CURRENT_VALUE)
				&& ( current > (OVER_CURRENT_VALUE - HYSTERYSIS) ))
    {
	    if ( currentHigh )
			ret = true;  
	    else
			ret = false;   
	}
	return ret;
}


int main(void)
{
	uint16_t timer = 0;
	bool faultState = false;
	
	initClock();
	initialiseCoilControllerDisable();
	initialiseFaultIndicator();
	initCurrentMeasurement();

    while (1) 
    {
		if (!faultState) {
			if (isOverCurrent()) {
				if (timer++ >= OVER_CURRENT_TIME_BEFORE_TRIP) {
					indicateFault();
					disableCoilController();
					faultState = true;
				}
			}
			else {
				resetFaultIndicator();
				timer = 0;
			}
		}
		sampleAverageCurrent();
		_delay_ms(1);
    }
}

