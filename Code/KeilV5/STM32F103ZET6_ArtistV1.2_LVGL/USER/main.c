/************************************************************************************
*  Copyright (c), 2020, LXG.
*
* FileName		:main.c
* Author		:firestaradmin
* Version		:1.0
* Date			:2020.8.9
* Description	:GRBL Machine.
* History		:
*
*
*************************************************************************************
*************************************************************************************
priorityGroup4: 0-15

stepper: 	TIM3 	priority:4
			TIM4 	priority:3
serial port:			
			USART1 	priority:5
limits:
			EXTI_Line3,4,5	priority:0 1 2
spindle：
			TIM1
SystemHeartbeat:
			TIM2	priority:6
LVGL_TASK	TIM5	priority:10

*************************************************************************************************************/
#include "main.h"

#include "grbl.h"
#include <stdio.h>

#include "ff.h"
#include "diskio.h"
#include "exfuns.h"
#include "fattester.h"

#include "lvgl_grbl_app.h"
#include "lv_myApp.h"

static void GRBL_Task(void);
void fats_sd_Init(void);
void BSP_Init(void);
void UVGUI_Init(void);
void GRBL_Init(void);

// Declare system global variable structure
system_t sys; 




int main()
{

	BSP_Init();

	lv_myApp_init();
	
	//g_Off_Line_Flag = 1;	//test 
	
	GRBL_Task();	//grbl loop entry 
	
	while(1);   /* 正常不会执行到这里 */   
 
}



static void GRBL_Task(void)
{
	// Check for power-up and set system alarm if homing is enabled to force homing cycle
	// by setting Grbl's alarm state. Alarm locks out all g-code commands, including the
	// startup scripts, but allows access to settings and internal commands. Only a homing
	// cycle '$H' or kill alarm locks '$X' will disable the alarm.
	// NOTE: The startup script will run after successful completion of the homing cycle, but
	// not after disabling the alarm locks. Prevents motion startup blocks from crashing into
	// things uncontrollably. Very bad.
#ifdef HOMING_INIT_LOCK
	if (bit_istrue(settings.flags,BITFLAG_HOMING_ENABLE)) { sys.state = STATE_ALARM; }
#endif
	
	// Force Grbl into an ALARM state upon a power-cycle or hard reset.
#ifdef FORCE_INITIALIZATION_ALARM
	sys.state = STATE_ALARM;
#endif
	
	// Grbl initialization loop upon power-up or a system abort. For the latter, all processes
	// will return to this loop to be cleanly re-initialized.
	for(;;) 
	{
		
	    // TODO: Separate configure task that require interrupts to be disabled, especially upon
	    // a system abort and ensuring any active interrupts are cleanly reset.
	  
	    // Reset Grbl primary systems.
	    serial_reset_read_buffer(); // Clear serial read buffer
		 
	    gc_init(); // Set g-code parser to default state
		
	    spindle_init();	//TIM1
	    coolant_init();
		
	    limits_init(); 
	    probe_init();
	    plan_reset(); // Clear block buffer and planner variables
	    st_reset(); // Clear stepper subsystem variables.
		
	    // Sync cleared gcode and planner positions to current system position.
	    plan_sync_position();
	    gc_sync_position();
	
	    // Reset system variables.
	    sys.abort = false;
	    sys_rt_exec_state = 0;
	    sys_rt_exec_alarm = 0;
	    sys.suspend = false;
	    sys.soft_limit = false;
		
	            
	    // Start Grbl main loop. Processes program inputs and executes them.
	    protocol_main_loop();
	}  
}



void BSP_Init(void)
{
	SystemInit();	//晶振时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//4位抢占优先级
	DelayInit();	//延时函数初始化
	LED_init();
	AT24CXX_Init();
	W25QXX_Init();
	TIM2_Int_Init(50 - 1, 7200 - 1); //5MS
	TIM5_Int_Init(50 - 1, 7200 - 1); //5MS
	serial_init();   // Setup serial baud rate and interrupts
	//screen
	lv_init();
	lv_port_disp_init();
	
	// Initialize system upon power-up.
	
	settings_init(); // Load Grbl settings from EEPROM
	stepper_init();  // Configure stepper pins and interrupt timers
	system_init();   // Configure pinout pins and pin-change interrupt

	memset(&sys, 0, sizeof(system_t));  // Clear all system variables
	sys.abort = true;   // Set abort to complete initialization
	sei(); // Enable interrupts
	
	
	//TF_Card
	disk_initialize(0);
	fats_sd_Init();

	
}



void fats_sd_Init(void)
{
	u8 res = 0;
	res = disk_initialize(0);	//initialize SD-Card
	if(res == 0)
		printf("initialize disk success!\r\n");
	else
		printf("initialize disk failed!\r\n");
	
	res = exfuns_init();
	if(res == 0)
		printf("malloc success!\r\n");
	else
		printf("malloc failed!\r\n");
	
	res = f_mount(fs[0], "0:", 1);
	if(res == 0)
		printf("mount disk0 success!\r\n");
	else
		printf("mount disk0 failed!\r\n");
	
//	u32 totalSize, freeSize;
//	res = exf_getfree((u8*)"0:", &totalSize, &freeSize);
//	if(res == 0)
//		printf("totalSize:%d KB, freeSize:%d KB\r\n", totalSize, freeSize);
//	else
//		printf("get failed!\r\n");


}



//此处代码将实现GPIO的功能控制，包括初始化，设置引脚电平和读取引脚电平。
void HW_GPIO_Init_Out(u32 gpio_clk,GPIO_TypeDef * gpio,u16 gpio_pin)			
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(gpio_clk,ENABLE);	 			//使能端口时钟
	GPIO_InitStructure.GPIO_Pin = gpio_pin;						//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 	//IO口速度为50MHz
	GPIO_Init(gpio, &GPIO_InitStructure);					//根据设定参数初始化
}

void HW_GPIO_Init_In(u32 gpio_clk,GPIO_TypeDef * gpio,u16 gpio_pin)			
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(gpio_clk,ENABLE);	 			//使能端口时钟
	GPIO_InitStructure.GPIO_Pin = gpio_pin;						//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 		//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 	//IO口速度为50MHz
	GPIO_Init(gpio, &GPIO_InitStructure);					//根据设定参数初始化
}

void HW_GPIO_Write(GPIO_TypeDef * gpio,u16 gpio_pin,u8 val)
{
	if(val==0) GPIO_ResetBits(gpio,gpio_pin);				//引脚输出低电平
	else GPIO_SetBits(gpio,gpio_pin);						//引脚输出高电平
}

u8 HW_GPIO_Read(GPIO_TypeDef * gpio,u16 gpio_pin)
{
	return GPIO_ReadInputDataBit(gpio,gpio_pin);			//返回引脚电平
}




