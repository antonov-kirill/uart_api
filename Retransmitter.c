#include "Retransmitter.h"

UART_API_Descriptor ReceiveUART;
UART_API_Descriptor TransmitUART;

uint16_t dataBuffer[TRASMISSION_BLOCK_SIZE * 2];

uint16_t recCntr = 0;
uint8_t blockReady = 0;

/**
  * @brief  Initializes the receiving UART (USART) peripheral, corresponding GPIO pins and interrupts 
  * 		with baudrate specified in the Retransmitter.h.
  * @param  UARTx: where x can be 1, 2, ... to select the UART (USART) peripheral.
  * @retval Pointer to a UART_API_Descriptor structure that contains
  *         the configuration information for the specified UART (USART) peripheral.
  */
UART_API_Descriptor* InitReceiveUART(const void* UART)
{
	ReceiveUART.Settings.BaudRate = RECEIVE_BAUDRATE;
	ReceiveUART.Settings.Mode = UART_API_Mode_RX;
	ReceiveUART.ReceiveCallBack = DataReceived;
	UART_API_Init(UART, &ReceiveUART);
	
	if (ReceiveUART.Status != UART_API_Status_NotInitialized)
		UART_API_Start(&ReceiveUART);
	
	return &ReceiveUART;
}

/**
  * @brief  Initializes the transmitting UART (USART) peripheral, corresponding GPIO pins and interrupts 
  * 		with baudrate specified in the Retransmitter.h.
  * @param  UARTx: where x can be 1, 2, ... to select the UART (USART) peripheral.
  * @retval Pointer to a UART_API_Descriptor structure that contains
  *         the configuration information for the specified UART (USART) peripheral.
  */
UART_API_Descriptor* InitTransmitUART(const void* UART)
{
	UART_API_Init(UART, &TransmitUART);
	UART_API_SetBaudRate(&TransmitUART, TRANSMIT_BAUDRATE);
	UART_API_SetMode(&TransmitUART, UART_API_Mode_TX);
	
#ifdef TEST_FUNCTIONALITY
	UART_API_SetTransmitCallBack(&TransmitUART, DataTransmitted);
#else
	UART_API_SetTransmitCallBack(&TransmitUART, 0);
#endif
	
	if (TransmitUART.Status != UART_API_Status_NotInitialized)
		UART_API_Start(&TransmitUART);
	
	return &TransmitUART;
}

/**
  * @brief  Callback function called from interrupt after successful reception of word via UART (USART) peripheral.
  *	@param 	data: the last received word
  * @retval None
  */
void DataReceived(uint16_t data)
{
	dataBuffer[recCntr++] = data;
	
	if (recCntr == TRASMISSION_BLOCK_SIZE)
		blockReady = 1;
	else if (recCntr == TRASMISSION_BLOCK_SIZE * 2)
	{
		blockReady = 2;
		recCntr = 0;
	}
	
	if (blockReady != 0)
	{
		UART_API_SendArray(&TransmitUART, &dataBuffer[(blockReady - 1) * TRASMISSION_BLOCK_SIZE], TRASMISSION_BLOCK_SIZE);
		blockReady = 0;
	}
}

#ifdef TEST_FUNCTIONALITY

	uint16_t transData[TRASMISSION_BLOCK_SIZE];
	uint16_t transCntr = 0;
	volatile uint8_t transDone = 0;
	volatile uint8_t testInProgress = 0;

	
	/**
    * 	@brief  Callback function called from interrupt after successful transmission of word via UART (USART) peripheral.
    *	@param 	data: the last transmitted word
    * 	@retval None
    */
	void DataTransmitted(uint16_t data)
	{
		if (testInProgress)
		{
			transData[transCntr++] = data;
			if (transCntr >= TRASMISSION_BLOCK_SIZE)
			{
				transCntr = 0;
				transDone = 1;
			}
		}
	}

	/**
    * 	@brief  Test to check if data getting from DataReceived isn't lost or damaged when it's transfered to transmitting UART (USART).
    *	@param 	None
    * 	@retval TestDone if test passes successfully; 
	*			RetransmissionFailed if the data was damaged during retransmission;
	*			InitializationFailed if UART (USART) aren't initialized
    */
	TestStatus TestFunctionality()
	{		
		if (ReceiveUART.Status != UART_API_Status_NotInitialized)
			UART_API_Stop(&ReceiveUART);
		else
			return InitializationFailed;
		
		if (TransmitUART.Status != UART_API_Status_NotInitialized)
			UART_API_Start(&TransmitUART);
		else
			return InitializationFailed;
				
		testInProgress = 1;
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < TRASMISSION_BLOCK_SIZE; j++)
				DataReceived((j + 1) * (i + 1));
			uint16_t CRC = CRC16((uint8_t*)&dataBuffer[i % 2 * TRASMISSION_BLOCK_SIZE], TRASMISSION_BLOCK_SIZE);
			
			while(!transDone);
			transDone = 0;
			
			if (CRC != CRC16((uint8_t*)transData, TRASMISSION_BLOCK_SIZE))
			{
				testInProgress = 0;
				return RetransmissionFailed;
			}
		}
		testInProgress = 0;
		return TestDone;
	}

	const uint16_t Crc16Table[256] = {
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
	};

	uint16_t CRC16(uint8_t* pcBlock, uint16_t len)
	{
		len *= 2;
		uint16_t CRC = 0xFFFF;

		while (len--)
			CRC = (CRC >> 8) ^ Crc16Table[(CRC & 0xFF) ^ *pcBlock++];

		return CRC;
	}
	
#endif