// 
// 
// 

#include "CircularBuffer.h"
#include <string.h>

CircularBuffer::CircularBuffer(int32_t size)
{
	_size = size;
	buffer = NULL;
	buffer = new float [size] ;
	for (int i = 0;i < size;i++)
		buffer[i] = 0;
}

CircularBuffer::~CircularBuffer()
{
	if (buffer != NULL)
		delete buffer;
}
void CircularBuffer::PushData(float data)
{
	if (buffer == NULL) false;
	for (int i = 1; i < _size;i++)
	{
		buffer[_size - i] = buffer[_size - i - 1];
	}
	buffer[0] = data;
}
float CircularBuffer::GetData(int32_t delay)
{
	if (delay >= _size)
		return 0;
	else
		return buffer[delay];
}

//                  #     # ### 
//                  ##    #  #  
//                  # #   #  #  
//                  #  #  #  #  
//                  #   # #  #  
//                  #    ##  #  
//                  #     # ### 
//
// Nuclear Instruments 2020 - All rights reserved
// Any commercial use of this code is forbidden
// Contact info@nuclearinstruments.eu
