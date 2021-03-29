#ifndef __UART_API_H__
#define __UART_API_H__

#ifndef UINT8_MAX
	typedef unsigned char uint8_t;
#endif
#ifndef UINT16_MAX
	typedef unsigned short uint16_t;
#endif
#ifndef UINT32_MAX
	typedef unsigned long uint32_t;
#endif

#define IS_UART_API_BAUDRATE(BaudRate) 			(BaudRate >= 0 && BaudRate <= 12500000)

typedef enum
{ 
	UART_API_WordLength_8b,
	UART_API_WordLength_5b,
	UART_API_WordLength_6b,
	UART_API_WordLength_7b,
	UART_API_WordLength_9b
	
}UART_API_WordLength;
#define IS_UART_API_WORDLENGTH(WordLength) 		(((WordLength) == UART_API_WordLength_8b) || ((WordLength) == UART_API_WordLength_5b) || \
												 ((WordLength) == UART_API_WordLength_6b) || ((WordLength) == UART_API_WordLength_7b) || \
												 ((WordLength) == UART_API_WordLength_9b))

typedef enum
{ 
	UART_API_StopBits_1b,
	UART_API_StopBits_2b,
	UART_API_StopBits_1_5b
	
}UART_API_StopBits;
#define IS_UART_API_STOPBITS(StopBits)			(((StopBits) == UART_API_StopBits_1b) || ((StopBits) == UART_API_StopBits_2b) || \
												 ((StopBits) == UART_API_StopBits_1_5b))

typedef enum
{ 
	UART_API_Parity_None,
	UART_API_Parity_Even,
	UART_API_Parity_Odd
	
}UART_API_Parity;
#define IS_UART_API_PARITY(Parity)				(((Parity) == UART_API_Parity_None) || ((Parity) == UART_API_Parity_Even) || \
												 ((Parity) == UART_API_Parity_Odd))

typedef enum
{ 
	UART_API_Mode_RX_TX,
	UART_API_Mode_RX,
	UART_API_Mode_TX
	
}UART_API_Mode;
#define IS_UART_API_MODE(Mode)					(((Mode) == UART_API_Mode_RX_TX) || ((Mode) == UART_API_Mode_RX) || \
												 ((Mode) == UART_API_Mode_TX))

typedef enum
{ 
	UART_API_FlowControl_None,
	UART_API_FlowControl_RTS,
	UART_API_FlowControl_CTS,
	UART_API_FlowControl_RTS_CTS
	
}UART_API_FlowControl;
#define IS_UART_API_FLOWCONTROL(FlowControl)	(((FlowControl) == UART_API_FlowControl_None) || ((FlowControl) == UART_API_FlowControl_RTS) || \
												 ((FlowControl) == UART_API_FlowControl_CTS) || ((FlowControl) == UART_API_FlowControl_RTS_CTS))

typedef enum
{ 
	UART_API_Status_NotInitialized,
	UART_API_Status_Stopped,
	UART_API_Status_Started
	
}UART_API_Status;

/** 
  * @brief	UART_API additional structure displaying settings of UART (USART) peripheral
  */ 
typedef struct
{
	uint32_t BaudRate;
	UART_API_WordLength WordLength;        
	UART_API_StopBits StopBits;          
	UART_API_Parity Parity;              
	UART_API_Mode Mode;                
	UART_API_FlowControl FlowControl;
	
} UART_API_Settings;

/** 
  * @brief	UART_API main structure providing access to the UART (USART) peripheral and callback functions,
			displaying status, settings of UART (USART) peripheral and transmission process	 
  */ 
typedef struct
{
	const void* UARTx;
	UART_API_Status Status;
	void(*ReceiveCallBack)(uint16_t data);
	void(*TransmitCallBack)(uint16_t data);
	UART_API_Settings Settings; 
	
	uint16_t* transArray;
	uint16_t transArraySize;
	volatile uint16_t transArrayCntr;
	
} UART_API_Descriptor;

/**
  * @brief  Initializes the UART (USART) peripheral, corresponding GPIO pins and interrupts according to the specified
  *         parameters in the Dscr. This function must be called before any other UART_API functions
  * @param  UARTx: where x can be 1, 2, ... to select the UART (USART) peripheral.
  * @param  Dscr: pointer to a UART_API_Descriptor structure that contains
  *         the configuration information for the specified UART (USART) peripheral.
  * @retval None
  */
void UART_API_Init(const void* UARTx, UART_API_Descriptor* Dscr);

/**
  * @brief  Enables the specified UART (USART) peripheral.
  * @param  Dscr: the pointer to a UART_API_Descriptor structure that contains the pointer to the UART (USART) peripheral
			which should be enabled.
  * @retval None
  */
void UART_API_Start(UART_API_Descriptor* Dscr);

/**
  * @brief  Disables the specified UART (USART) peripheral.
  * @param  Dscr: the pointer to a UART_API_Descriptor structure that contains the pointer to the UART (USART) peripheral
			which should be disabled.
  * @retval None
  */
void UART_API_Stop(UART_API_Descriptor* Dscr);

void UART_API_SetBaudRate(UART_API_Descriptor* Dscr, uint32_t BaudRate);
void UART_API_SetWordLength(UART_API_Descriptor* Dscr, UART_API_WordLength WordLength);
void UART_API_SetStopBits(UART_API_Descriptor* Dscr, UART_API_StopBits StopBits);
void UART_API_SetParity(UART_API_Descriptor* Dscr, UART_API_Parity Parity);
void UART_API_SetMode(UART_API_Descriptor* Dscr, UART_API_Mode Mode);
void UART_API_SetFlowControl(UART_API_Descriptor* Dscr, UART_API_FlowControl FlowControl);

/**
  * @brief  Sets the callback function which will be called after successful reception of word via UART (USART) peripheral.
  * @param  Dscr: the pointer to a UART_API_Descriptor structure that contains the pointer to the UART (USART) peripheral
			receiving data.
  * @param  CallBack: the pointer to the function which should be called from the interrupt after successful reception of data.
  *								@param data: the last received word
  * @retval None
  */
void UART_API_SetReceiveCallBack(UART_API_Descriptor* Dscr, void(*CallBack)(uint16_t data));

/**
  * @brief  Sets the callback function which will be called after successful transmission of word via UART (USART) peripheral.
  * @param  Dscr: the pointer to a UART_API_Descriptor structure that contains the pointer to the UART (USART) peripheral
			transmitting data.
  * @param  CallBack: the pointer to the function which should be called from the interrupt after successful transmission of data.
  *								@param data: the last transmitted word
  * @retval None
  */
void UART_API_SetTransmitCallBack(UART_API_Descriptor* Dscr, void(*CallBack)(uint16_t data));

/**
  * @brief  Initiates data transmission via UART (USART) peripheral using interrupts.
  * @param  Dscr: the pointer to a UART_API_Descriptor structure that contains the pointer to the UART (USART) peripheral
			which will transmit the data.
  * @param  data: the data which will be transmitted via UART (USART) peripheral.
  * @param  size: size of the transmitted data.
  * @retval None
  */
void UART_API_SendArray(UART_API_Descriptor* Dscr, uint16_t* data, uint16_t size);

#endif