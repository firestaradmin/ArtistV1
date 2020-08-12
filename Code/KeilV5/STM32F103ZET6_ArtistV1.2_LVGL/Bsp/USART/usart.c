/***********************************************
* 	FileName:		
*	Description:	UART 
*	Author: 		LXG@firestaradmin
*	Date:			2020/6/19
*	Email:			firestardog@qq.com
************************************************/

#include "main.h"
#include "usart.h"	  
#include <stdio.h>
//#include "../bsp/transfer_BIN_to_EX_FLASH/transfer_BIN_to_EX_FLASH.h"
 
#ifdef USE_PRINTF
	#pragma import(__use_no_semihosting)             
	// 请不要勾选Use MicroLib
	#ifdef __MICROLIB
		#error "Please do not use MicroLib!"
	#endif
	
	#ifdef __cplusplus
	extern "C"
	{
		void _sys_exit(int returncode)
		{
			printf("Exited! returncode=%d\n", returncode);
			while (1);
		}
		
		void _ttywrch(int ch)
		{
			if (ch == '\n'){
				while((PRINTF_UART->SR&0X40)==0);
				USART1->DR = (u8) '\r'; 
				while((PRINTF_UART->SR&0X40)==0);
				USART1->DR = (u8) '\n'; 
			} 
			else{
				while((PRINTF_UART->SR&0X40)==0);
				PRINTF_UART->DR = (u8) ch;
			}
			
		}
	}

	namespace std{
		struct __FILE
		{
		int handle;
		/* Whatever you require here. If the only file you are using is */
		/* standard output using printf() for debugging, no file handling */
		/* is required. */
		};
	
		FILE __stdin = {0};
		FILE __stdout = {1};
		FILE __stderr = {2};
			  
		int fgetc(FILE *stream)
		{
			int c = 0;
			if (stream->handle == 0)
			{
				while (USART_GetFlagStatus(PRINTF_UART, USART_FLAG_RXNE) == RESET);
				c = USART_ReceiveData(PRINTF_UART);
				_ttywrch((c == '\r') ? '\n' : c); // 回显
				return c;
			}
			return EOF;
		}

		int fputc(int c, FILE *stream)
		{
			if (stream->handle == 1 || stream->handle == 2)
			{
				while((PRINTF_UART->SR&0X40)==0);
				PRINTF_UART->DR = (u8) c;      
				return c;
			}
			return EOF;
		}

		int fclose(FILE * stream)
		{
			return 0;
		}

		int fseek(FILE *stream, long int offset, int whence)
		{
			return -1;
		}
		
		int fflush(FILE *stream)
		{
			if (stream->handle == 1 || stream->handle == 2)
			while (USART_GetFlagStatus(PRINTF_UART, USART_FLAG_TC) == RESET);
			return 0;
		}
	}
	#else
		//标准库需要的支持函数                 
		struct __FILE 
		{ 
			int handle; 

		}; 

		FILE __stdout;       
		//定义_sys_exit()以避免使用半主机模式    
		void _sys_exit(int returncode)
		{
			printf("Exited! returncode=%d\n", returncode);
			while (1);
		}
		void _ttywrch(int ch)
		{
			if (ch == '\n'){
				while((PRINTF_UART->SR&0X40)==0);
				USART1->DR = (u8) '\r'; 
				while((PRINTF_UART->SR&0X40)==0);
				USART1->DR = (u8) '\n'; 
			} 
			else{
				while((PRINTF_UART->SR&0X40)==0);
				PRINTF_UART->DR = (u8) ch;
			}
			
		}
		int fputc(int c, FILE *stream)
		{
			while((PRINTF_UART->SR&0X40)==0);
			PRINTF_UART->DR = (u8) c;      
			return c;
		}
	#endif
#endif 



	  
/*************************************************************
//USART1
//PA9 	TX->MODULE_RX
//PA10 	RX->MODULE_TX
 *************************************************************/
void usart1_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9

	//USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=5 ;//抢占优先级
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

#ifdef __cplusplus
extern "C"{
#endif
	
/*
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 res;
	// 添加此句，防止串口一直进入此中断
	if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART1);
	}
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  
	{
		USART_ClearITPendingBit(USART1,  USART_IT_RXNE);	//清除中断标志
		res = USART_ReceiveData(USART1);
		//TODO: 处理接收
		//userCallBack(res);
		TBEF_uart_receive_process(res);
		res = res;
	} 

} 
*/
#ifdef __cplusplus
}
#endif


/*************************************************************
//USART2
//PA2 TX->MODULE_RX
//PA3 RX->MODULE_TX
 *************************************************************/
void usart2_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2时钟
	
	//USART2_TX   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA2

	//USART2_RX	  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA3

	//Usart3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=9 ;//抢占优先级
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_Cmd(USART2, ENABLE);                    //使能串口2

}


void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	u8 res;
	// 添加此句，防止串口一直进入此中断
	if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART2);
	}

	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)	//判断发生接收中断
	{
		
		USART_ClearITPendingBit(USART2,  USART_IT_RXNE);	//清除中断标志
		res = USART_ReceiveData(USART2);
		//TODO: 处理接收
		//userCallBack(res);
		res = res;
	}
} 

/*************************************************************
//USART3
PB10 USART3_TX
PB11 USART3_RX 
 *************************************************************/
void usart3_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能USART3时钟
	
	//USART3_TX   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

	//USART3_RX	  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

	//Usart3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=9 ;//抢占优先级
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_Cmd(USART3, ENABLE);                    //使能串口

}


void USART3_IRQHandler(void)                	//串口中断服务程序
{
	u8 res;
	// 添加此句，防止串口一直进入此中断
	if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART3);
	}

	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)	//判断发生接收中断
	{
		
		USART_ClearITPendingBit(USART3,  USART_IT_RXNE);	//清除中断标志
		res = USART_ReceiveData(USART3);
		//TODO: 处理接收
		//userCallBack(res);
		res = res;
	}
} 


/**************************************************************************************
* FunctionName   : UartSendByte()
* Description    : 发送一字节数据
* EntryParameter : dat- 数据值
* ReturnValue    : None
**************************************************************************************/
void UartSendByte(USART_TypeDef *USART, u8 dat)
{

    USART_SendData(USART, dat);					// 发送数据
    while (USART_GetFlagStatus(USART, USART_FLAG_TC) == RESET);      // 获取串口状态

}

/**************************************************************************************
* FunctionName   : UartSendMultByte()
* Description    : 发送多字节数据
* EntryParameter : str - 数据缓存; len - 数据长度
* ReturnValue    : None
**************************************************************************************/
void UartSendMultByte(USART_TypeDef *UART, u8 *pStr, u16 len)
{
    while (len--)
    {
        USART_SendData(UART, *pStr++);				// 发送数据

        while (USART_GetFlagStatus(UART, USART_FLAG_TC) == RESET)  // 获取串口状态
        {
            ;
        }
    }
}

/**************************************************************************************
* FunctionName   : UartSendStr()
* Description    : 发送多字节数据
* EntryParameter : str - 数据缓存
* ReturnValue    : None
**************************************************************************************/
void UartSendStr(USART_TypeDef *UART, uc8 *pStr)
{
    while (*pStr != '\0')
    {
        USART_SendData(UART, *pStr++);				// 发送数据
        while (USART_GetFlagStatus(UART, USART_FLAG_TC) == RESET)  // 获取串口状态
        {
            ;
        }
    }
}


















