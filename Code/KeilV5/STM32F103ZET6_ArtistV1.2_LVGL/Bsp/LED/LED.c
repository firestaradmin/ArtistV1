#include "LED.h"
#include "stm32f10x.h"

void LED_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef  GPIO_InitStructure;
	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}


void LED_ON(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void LED_OFF(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void LED_TOGGLE(void)
{
	static FlagStatus LED_Status = RESET;
	if(LED_Status){
		LED_OFF();
		LED_Status = RESET;
	}
	else{
		LED_ON();
		LED_Status = SET;
	}
}

