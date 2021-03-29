#ifndef __RETRANSMITTER_H__
#define __RETRANSMITTER_H__

#include "UART_API.h"

#define RECEIVE_BAUDRATE 	9600
#define TRANSMIT_BAUDRATE 	115200
#define TRASMISSION_BLOCK_SIZE	128

UART_API_Descriptor* InitReceiveUART(const void* UART);
UART_API_Descriptor* InitTransmitUART(const void* UART);

void DataReceived(uint16_t data);
void DataTransmitted(uint16_t data);

#define TEST_FUNCTIONALITY
#ifdef TEST_FUNCTIONALITY

	typedef enum
	{
		TestDone,
		InitializationFailed,
		RetransmissionFailed
	} TestStatus;

	TestStatus TestFunctionality();
	uint16_t CRC16(uint8_t* pcBlock, uint16_t len);
	
#endif

#endif