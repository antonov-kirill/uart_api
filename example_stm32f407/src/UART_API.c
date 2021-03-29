#include "stm32f4xx.h"
#include "UART_API.h"

void UART_API_Init(const void* UARTx, UART_API_Descriptor* Dscr)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	Dscr->UARTx = UARTx;
	if (UARTx == USART1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9 | GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
		NVIC_EnableIRQ(USART1_IRQn);
	}
	else if (UARTx == UART4)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);
		NVIC_EnableIRQ(UART4_IRQn);
	}
	
	UART_API_SetBaudRate(Dscr, Dscr->Settings.BaudRate);
	UART_API_SetWordLength(Dscr, Dscr->Settings.WordLength);
	UART_API_SetStopBits(Dscr, Dscr->Settings.StopBits);
	UART_API_SetParity(Dscr, Dscr->Settings.Parity);
	UART_API_SetMode(Dscr, Dscr->Settings.Mode);
	UART_API_SetFlowControl(Dscr, Dscr->Settings.FlowControl);
	
	Dscr->Status = UART_API_Status_Stopped;
}

void UART_API_Start(UART_API_Descriptor* Dscr)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	if (Dscr->Status != UART_API_Status_NotInitialized)
	{
		((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_UE;
		Dscr->Status = UART_API_Status_Started;
	}
}

void UART_API_Stop(UART_API_Descriptor* Dscr)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	if (Dscr->Status != UART_API_Status_NotInitialized)
	{
		((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_UE;
		Dscr->Status = UART_API_Status_Stopped;
	}
}

void UART_API_SetBaudRate(UART_API_Descriptor* Dscr, uint32_t BaudRate)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	assert_param(IS_UART_API_BAUDRATE(BaudRate));
	
	UART_API_Status status = Dscr->Status;
	UART_API_Stop(Dscr);
	
	uint32_t tmpreg = 0x00;
	uint32_t apbclock = 0x00;
  	uint32_t integerdivider = 0x00;
  	uint32_t fractionaldivider = 0x00;
  	RCC_ClocksTypeDef RCC_ClocksStatus;
	
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	if ((Dscr->UARTx == USART1) || (Dscr->UARTx == USART6))
		apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	else
		apbclock = RCC_ClocksStatus.PCLK1_Frequency;

	if ((((USART_TypeDef*)Dscr->UARTx)->CR1 & USART_CR1_OVER8) != 0)
		integerdivider = ((25 * apbclock) / (2 * BaudRate));    
	else
		integerdivider = ((25 * apbclock) / (4 * BaudRate));   
	
	tmpreg = (integerdivider / 100) << 4;

	fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	if ((((USART_TypeDef*)Dscr->UARTx)->CR1 & USART_CR1_OVER8) != 0)
		tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
	else
		tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);

	((USART_TypeDef*)Dscr->UARTx)->BRR = (uint16_t)tmpreg;
	Dscr->Settings.BaudRate = BaudRate;
	
	if (status == UART_API_Status_Started)
		UART_API_Start(Dscr);
}

void UART_API_SetWordLength(UART_API_Descriptor* Dscr, UART_API_WordLength WordLength)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	assert_param(IS_UART_API_WORDLENGTH(WordLength));
	
	UART_API_Status status = Dscr->Status;
	UART_API_Stop(Dscr);
	
	switch(WordLength)
	{
		case UART_API_WordLength_9b:
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_M;
			Dscr->Settings.WordLength = UART_API_WordLength_9b;
			break;
		default:
			((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_M;
			Dscr->Settings.WordLength = UART_API_WordLength_8b;
			break;
	}
	
	if (status == UART_API_Status_Started)
		UART_API_Start(Dscr);
}

void UART_API_SetStopBits(UART_API_Descriptor* Dscr, UART_API_StopBits StopBits)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	assert_param(IS_UART_API_STOPBITS(StopBits));
	
	UART_API_Status status = Dscr->Status;
	UART_API_Stop(Dscr);
		
	switch(StopBits)
	{
		case UART_API_StopBits_2b:
			((USART_TypeDef*)Dscr->UARTx)->CR2 |= USART_CR2_STOP_1;
			((USART_TypeDef*)Dscr->UARTx)->CR2 &= ~USART_CR2_STOP_0;
			break;
		case UART_API_StopBits_1_5b:
			((USART_TypeDef*)Dscr->UARTx)->CR2 |= USART_CR2_STOP;
			break;
		default:
			((USART_TypeDef*)Dscr->UARTx)->CR2 &= ~USART_CR2_STOP;
			break;
	}
	Dscr->Settings.StopBits = StopBits;
	
	if (status == UART_API_Status_Started)
		UART_API_Start(Dscr);
}

void UART_API_SetParity(UART_API_Descriptor* Dscr, UART_API_Parity Parity)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	assert_param(IS_UART_API_PARITY(Parity));
	
	UART_API_Status status = Dscr->Status;
	UART_API_Stop(Dscr);
		
	switch(Parity)
	{
		case UART_API_Parity_Even:
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_PCE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_PS;
			break;
		case UART_API_Parity_Odd:
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_PCE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_PS;
			break;
		default:
			((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_PCE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_PS;
			break;
	}
	Dscr->Settings.Parity = Parity;
	
	if (status == UART_API_Status_Started)
		UART_API_Start(Dscr);
}

void UART_API_SetMode(UART_API_Descriptor* Dscr, UART_API_Mode Mode)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	assert_param(IS_UART_API_MODE(Mode));
	
	UART_API_Status status = Dscr->Status;
	UART_API_Stop(Dscr);
		
	switch(Mode)
	{
		case UART_API_Mode_RX:
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_RE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_RXNEIE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_TE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_TCIE;
			break;
		case UART_API_Mode_TX:
			((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_RE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 &= ~USART_CR1_RXNEIE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_TE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_TCIE;
			break;
		default:
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_RE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_RXNEIE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_TE;
			((USART_TypeDef*)Dscr->UARTx)->CR1 |= USART_CR1_TCIE;
			break;
	}
	Dscr->Settings.Mode = Mode;
	
	if (status == UART_API_Status_Started)
		UART_API_Start(Dscr);
}

void UART_API_SetFlowControl(UART_API_Descriptor* Dscr, UART_API_FlowControl FlowControl)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	assert_param(IS_UART_API_FLOWCONTROL(FlowControl));
	
	UART_API_Status status = Dscr->Status;
	UART_API_Stop(Dscr);
		
	if (((USART_TypeDef*)Dscr->UARTx) == UART4 || ((USART_TypeDef*)Dscr->UARTx) == UART5)
		FlowControl = UART_API_FlowControl_None;
	
	switch(FlowControl)
	{
		case UART_API_FlowControl_RTS:
			((USART_TypeDef*)Dscr->UARTx)->CR3 |= USART_CR3_RTSE;
			((USART_TypeDef*)Dscr->UARTx)->CR3 &= ~USART_CR3_CTSE;
			break;
		case UART_API_FlowControl_CTS:
			((USART_TypeDef*)Dscr->UARTx)->CR3 &= ~USART_CR3_RTSE;
			((USART_TypeDef*)Dscr->UARTx)->CR3 |= USART_CR3_CTSE;
			break;
		case UART_API_FlowControl_RTS_CTS:
			((USART_TypeDef*)Dscr->UARTx)->CR3 |= USART_CR3_RTSE;
			((USART_TypeDef*)Dscr->UARTx)->CR3 |= USART_CR3_CTSE;
			break;
		default:
			((USART_TypeDef*)Dscr->UARTx)->CR3 &= ~USART_CR3_RTSE;
			((USART_TypeDef*)Dscr->UARTx)->CR3 &= ~USART_CR3_CTSE;
			break;
	}
	Dscr->Settings.FlowControl = FlowControl;
	
	if (status == UART_API_Status_Started)
		UART_API_Start(Dscr);
}

void UART_API_SetReceiveCallBack(UART_API_Descriptor* Dscr, void(*CallBack)(uint16_t data))
{
	UART_API_Status status = Dscr->Status;
	UART_API_Stop(Dscr);
	
	Dscr->ReceiveCallBack = CallBack;
	
	if (status == UART_API_Status_Started)
		UART_API_Start(Dscr);
}

void UART_API_SetTransmitCallBack(UART_API_Descriptor* Dscr, void(*CallBack)(uint16_t data))
{	
	UART_API_Status status = Dscr->Status;
	UART_API_Stop(Dscr);
	
	Dscr->TransmitCallBack = CallBack;
	
	if (status == UART_API_Status_Started)
		UART_API_Start(Dscr);
}

void UART_API_SendArray(UART_API_Descriptor* Dscr, uint16_t* data, uint16_t size)
{
	assert_param(IS_USART_ALL_PERIPH((USART_TypeDef*)Dscr->UARTx));
	
	Dscr->transArray = data;
	Dscr->transArraySize = size;
	Dscr->transArrayCntr = 0;
	
	if (size > 0)
		((USART_TypeDef*)Dscr->UARTx)->DR = (data[Dscr->transArrayCntr++] & (uint16_t)0x01FF);
}