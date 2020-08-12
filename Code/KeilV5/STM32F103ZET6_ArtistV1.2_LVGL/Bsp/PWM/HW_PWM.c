#include "main.h"


void HW_PWM_Init(u32 gpio_clk,GPIO_TypeDef * gpio,u16 gpio_pin,u32 pwm_clk,TIM_TypeDef * pwm_timer,float pwm_max_value)		  
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(pwm_clk, ENABLE);					//使能定时器时钟
 	RCC_APB2PeriphClockCmd(gpio_clk|RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
 
	GPIO_InitStructure.GPIO_Pin = gpio_pin; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio, &GPIO_InitStructure);//初始化GPIO
 
   //初始化TIM
	TIM_TimeBaseStructure.TIM_Period = (u32)pwm_max_value; 	
	TIM_TimeBaseStructure.TIM_Prescaler =32;			//阅读源文件spindle_control.c可知分频数为8 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(pwm_timer, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化Channel 1 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(pwm_timer, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIMx OC1

	TIM_OC1PreloadConfig(pwm_timer, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
}

void HW_PWM_Write(TIM_TypeDef * pwm_timer,u32 val)
{
	TIM_SetCompare1(pwm_timer,val);
}

void HW_PWM_Disable(TIM_TypeDef * pwm_timer)
{
	TIM_Cmd(pwm_timer, DISABLE); 
}

void HW_PWM_Enable(TIM_TypeDef * pwm_timer)
{
	TIM_Cmd(pwm_timer, ENABLE); 
}



//for servo
void HW_TIM1_PWM_Spindle_Init()		  
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);					//使能定时器时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

   //初始化TIM
	TIM_TimeBaseStructure.TIM_Period = (u32)999; 	//50Hz
	TIM_TimeBaseStructure.TIM_Prescaler =1439;			//阅读源文件spindle_control.c可知分频数为8 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;//周期计数器值
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	 


	//初始化Channel 1 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputNState_Enable; //比较输出失能 正向通道无效
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;//反向通道有效
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高、
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_Low; //互补端的极性
	TIM_OCInitStructure.TIM_Pulse = 35; 
	
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//空闲状态下的非工作状态 
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIMx OC1

	TIM_OC3PreloadConfig(TIM1,TIM_OCPreload_Enable);        //使能预装载寄存器
	
	/* TIM1 counter enable开定时器 */
	//TIM_Cmd(TIM1,ENABLE);        //使能TIM1
	//TIM_CtrlPWMOutputs(TIM1,ENABLE);//主动输出使能
	
}

void HW_TIM1_PWM_Set_Pulse(u16 val)	
{
	TIM_SetCompare3(TIM1, val);
}

void HW_TIM1_PWM_Disable()
{
	TIM_CtrlPWMOutputs(TIM1,DISABLE);
	TIM_Cmd(TIM1, DISABLE); 
	
}

void HW_TIM1_PWM_Enable()
{
	TIM_Cmd(TIM1, ENABLE); 
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//主动输出使能
}
