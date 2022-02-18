/*
 * main.h
 *
 */ 


#ifndef MAIN_H_
#define MAIN_H_

// the current value is 1mV per milliamp, 1V = 1A.  The full scale is 5V there are 256 levels so each level is measuring about 19.5mA or approx 20mA
// we dont need to be too accurate here

#define OVER_CURRENT_MA			1700
#define MA_PER_STEP				20
#define OVER_CURRENT_VALUE		OVER_CURRENT_MA/MA_PER_STEP
#define PERCENT_HYSTERYSIS		20
#define HYSTERYSIS_MA			OVER_CURRENT_MA/PERCENT_HYSTERYSIS
#define HYSTERYSIS				HYSTERYSIS_MA/MA_PER_STEP

#define OVER_CURRENT_TIME_BEFORE_TRIP	1000 //ms

#define N_CURRENT_SAMPLES	12

#define false 0
#define true !false
typedef uint8_t bool;


#endif /* MAIN_H_ */