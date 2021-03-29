#include "stm32f4xx.h"
#include "Init.h"
#include "Retransmitter.h"

UART_API_Descriptor* pReceiveUART;
UART_API_Descriptor* pTransmitUART;

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		GPIO_ToggleBits(GPIOA, GPIO_Pin_2);
		GPIO_ToggleBits(GPIOA, GPIO_Pin_3);
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void USART1_IRQHandler()
{
	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_TC);
		if (pTransmitUART->TransmitCallBack != 0 && pTransmitUART->transArrayCntr > 0)
			pTransmitUART->TransmitCallBack(pTransmitUART->transArray[pTransmitUART->transArrayCntr - 1]);
		
		if (pTransmitUART->transArrayCntr < pTransmitUART->transArraySize)
			((USART_TypeDef*)pTransmitUART->UARTx)->DR = (pTransmitUART->transArray[pTransmitUART->transArrayCntr++] & (uint16_t)0x01FF);
		else
		{
			pTransmitUART->transArrayCntr = 0;
			pTransmitUART->transArraySize = 0;
		}
	}
}

void UART4_IRQHandler()
{
	if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		if (pReceiveUART->ReceiveCallBack != 0)
			pReceiveUART->ReceiveCallBack(USART_ReceiveData(UART4));
	}
}

void main()
{
	__disable_irq();
	
	InitAll();
	GPIO_ToggleBits(GPIOA, GPIO_Pin_2);
	
	pReceiveUART = InitReceiveUART(UART4);
	pTransmitUART = InitTransmitUART(USART1);
	
	__enable_irq();
		
#ifdef TEST_FUNCTIONALITY
	if (TestFunctionality() != TestDone)
		while(1);
#endif
	
	while(1)
	{
		
	}
}
