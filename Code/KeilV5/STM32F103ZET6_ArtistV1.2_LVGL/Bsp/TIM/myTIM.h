#ifndef __MYTIM_H_
#define __MYTIM_H_

#include <stm32f10x.h>



void TIM2_Int_Init(u16 arr,u16 psc);
void TIM5_Int_Init(u16 arr,u16 psc);


//##############################【API】##############################
void HW_TIM_Init(void);				//定时器初始化，要移至到stepper.c中的st_init()
//Driver Interrupt
void HW_TIM_DriverInterrupt_Enable(void); 		//开启定时器
void HW_TIM_DriverInterrupt_Disable(void);		//关闭定时器
void HW_TIM_DriverInterrupt_ValueConfig(u32 Prescaler,u32 Autoreload);		//设置定时器重装值
//Port Reset Interrupt
void HW_TIM_PortResetInterrupt_Enable(void);  	//开启定时器
void HW_TIM_PortResetInterrupt_Disable(void);	//关闭定时器
void HW_TIM_PortResetInterrupt_ValueConfig(u32 Prescaler,u32 Autoreload); 	//设置定时器重装值
//Debounce
void HW_Debounce_Init(void);		//去抖定时器
void HW_Debounce_Enable(void);		//使能去抖定时器
void HW_Debounce_Disable(void);		//除能去抖定时器



#endif


