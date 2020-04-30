/*
 * scaledtime.h
 *
 *  Created on: 28 apr 2020
 *      Author: AngeloGargantini
 *
 *      for testing we use this scaled time to run the tests faster
 *
 */
#ifndef SCALEDTIME_H_
#define SCALEDTIME_H_
// wait for the scaled time
long scaledDuration(long ms);
// start the stop watch, wher eto begin the scaled time
void startScaledTime();
// return the millisecs from the beginning
long getScaledMillisecfromInit();
// after the start the scaled time passes faster
long getScaledMillisec();
// scaled sleep
void scaledSleep(long ms);

#endif /* SCALEDTIME_H_ */
