/* Includes ------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
#include <assert.h>
#include "ring_buffer.h"

bool RingBuffer_Init(RingBuffer *ringBuffer, uint8_t *dataBuffer,
		size_t dataBufferSize) {
	assert(ringBuffer);
	assert(dataBuffer);
	assert(dataBufferSize > 0);

	if ((ringBuffer) && (dataBuffer) && (dataBufferSize > 0)) {
		ringBuffer->buffer = dataBuffer;
		ringBuffer->size = dataBufferSize;
		return RingBuffer_Clear(ringBuffer);
	}

	return false;
}

bool RingBuffer_Clear(RingBuffer *ringBuffer) {
	assert(ringBuffer);

	if (ringBuffer) {
	    ringBuffer->head = 0;
		ringBuffer->tail = 0;
		ringBuffer->len=0;
		for (int i = 0; i < ringBuffer->size; ++i) {
			ringBuffer->buffer[i] = 0;
		}
		return true;
	}
	return false;
}

bool RingBuffer_IsEmpty(const RingBuffer *ringBuffer) {
	assert(ringBuffer);
//	printf("head:%d\ttail:%d\tsize:%d\n",ringBuffer->head,ringBuffer->tail,ringBuffer->size);
	if (ringBuffer->len == 0)
		return true;
	return false;
}

size_t RingBuffer_GetLen(const RingBuffer *ringBuffer) {
	assert(ringBuffer);

	if (ringBuffer) {
    	return ringBuffer->len;
	}
	return 0;

}

size_t RingBuffer_GetCapacity(const RingBuffer *ringBuffer) {
	assert(ringBuffer);

	if (ringBuffer) {
		return ringBuffer->size;
	}
	return 0;
}

bool RingBuffer_PutChar(RingBuffer *ringBuffer, uint8_t c) {
	assert(ringBuffer);

	if (ringBuffer) {
	   // printf("head:%d\ttail:%d\tsize:%d\n",ringBuffer->head,ringBuffer->tail,ringBuffer->size);
	   if(ringBuffer->len==ringBuffer->size)
	    return false;
        ringBuffer->buffer[ringBuffer->head++]=c;
        ringBuffer->len++;
        if(ringBuffer->head>=(ringBuffer->size))
            ringBuffer->head=0;
		return true;
	}
	return false;
}

bool RingBuffer_GetChar(RingBuffer *ringBuffer, uint8_t *c) {
	assert(ringBuffer);
	assert(c);

	if ((ringBuffer) && (c)) {
	    //printf("head:%d\ttail:%d\tsize:%d\n",ringBuffer->head,ringBuffer->tail,ringBuffer->size);
		if(ringBuffer->len==0)
		    return false;
		*c=ringBuffer->buffer[ringBuffer->tail++];
		ringBuffer->len--;
		if(ringBuffer->tail>=ringBuffer->size)
		ringBuffer->tail=0;
		return true;
	}
	return false;
}
#ifdef __cplusplus
}
#endif
