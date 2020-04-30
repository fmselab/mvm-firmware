/*
 * Serial.cpp
 *
 *  Created on: 26 apr 2020
 *      Author: AngeloGargantini
 */

#include <Serial.h>
#include <iostream>
// used somewhere
//extern SerialImpl Serial;
SerialImpl Serial;

size_t SerialImpl::println(const String &str) {
	std::cout << str;
}

size_t SerialImpl::println(char const *str) {
	std::cout << str;
}
