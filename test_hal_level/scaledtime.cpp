/*
 * scaledtime.c
 *
 *  Created on: 28 apr 2020
 *      Author: AngeloGargantini
 *
 *      for testing we use this scaled time to run the tests faster
 *
 */
#include <chrono>
#include <thread>

// take the time passed when the hal starts
static long stopwatch;
// time will be scaled after the stopwatch
static int scale = 2;
// wait for the scaled time
long scaledDuration(long ms){
	return ms/scale;
}
void startScaledTime(){
	// start the stop watch
	stopwatch = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
// after the start the scaled time passes faster
long getScaledMillisec(){
	long now = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	return stopwatch + (now-stopwatch) * scale;
}
// return the seconds from teh beginning
long getScaledMillisecfromInit(){
	return (getScaledMillisec() - stopwatch);
}
// scaled sleep
void scaledSleep(long ms){
	std::this_thread::sleep_for(std::chrono::milliseconds(ms/scale));
}
