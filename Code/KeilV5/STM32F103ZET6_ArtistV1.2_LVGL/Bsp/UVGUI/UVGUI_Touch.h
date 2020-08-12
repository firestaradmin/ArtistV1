/************************************************************************************
*  Copyright (c), 2019, LXG.
*
* FileName		:UVGUI_Touch.h
* Author		:firestaradmin
* Version		:1.0
* Date			:2020.8.1
* Description	:a universal GUI Lib, usually used in Screen display of Embedded System.
* History		:
*
*
*************************************************************************************/

#ifndef _UVGUI_TOUCH_H__
#define _UVGUI_TOUCH_H__
#include "stm32f10x.h"
#include "UVGUI.h"


#define TP_PRES_DOWN 0x80  		//触屏被按下	  
#define TP_CATH_PRES 0x40  		//有按键按下了 
#define MAX_TOUCH_Point  5    		//电容屏支持的点数,固定为5点

#define SAVE_ADDR_BASE 490
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	电阻/电容屏芯片连接引脚  Pin_MAP
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define TOUCH_PEN_GPIO		GPIOF
#define TOUCH_PEN_GPIO_Pin	GPIO_Pin_10

#define TOUCH_MISO_GPIO		GPIOF
#define TOUCH_MISO_GPIO_Pin	GPIO_Pin_7

#define TOUCH_MOSI_GPIO		GPIOF
#define TOUCH_MOSI_GPIO_Pin	GPIO_Pin_9

#define TOUCH_SCLK_GPIO		GPIOB
#define TOUCH_SCLK_GPIO_Pin	GPIO_Pin_1

#define TOUCH_CS_GPIO		GPIOF
#define TOUCH_CS_GPIO_Pin	GPIO_Pin_11


#define PEN  		PFin(10)  	//PF10 INT
#define DOUT 		PFin(7)   	//PF7
#define TDIN 		PFout(9)  	//PF9  MOSI
#define TCLK 		PBout(1)  	//PB1  SCLK
#define TCS  		PFout(11)  	//PF11  CS 

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	UVGUI enum define
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef enum UVGUI_Touch_Type_enum{
	UVGUI_TOUCH_TYPE_RESISTIVE= 0,
	UVGUI_TOUCH_TYPE_CAPACITIV= 1
}UVGUI_Touch_Type_enum_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	UVGUI struct define
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* TOUCH Dev struct 
status：触摸状态
	[bit7]1:pressed,0:unpressed
	[bit6]1:有按键按下,0:无按键按下
	[bit5]保留
	[bit4~0]触摸屏按下的点数
touchtype:触摸类型 
	UVGUI_TOUCH_TYPE_RESISTIVE:电阻屏 
	UVGUI_TOUCH_TYPE_CAPACITIV:电容屏
xPos,yPos：当前坐标，
	电容屏用数组最后一个元素储存第一次按下的坐标
	电阻屏始终使用数组第一个元素xPos[0],yPos[0]表示坐标（电阻屏只支持同时按下一个坐标）
*/
typedef struct UVGUI_Touch_Dev_Struct{
	u8 status;	
	UVGUI_Touch_Type_enum_t touchtype;
	u16 xPos[MAX_TOUCH_Point]; 		
	u16 yPos[MAX_TOUCH_Point];							 
	
/////////////////////触摸屏校准参数(电容屏不需要校准)//////////////////////								
	float xfac;					
	float yfac;
	short xoff;
	short yoff;	   
//新增的参数,当触摸屏的左右上下完全颠倒时需要用到.
//b0:0,竖屏(适合左右为X坐标,上下为Y坐标的TP) ; 1,横屏(适合左右为Y坐标,上下为X坐标的TP) 
//b1~6:保留.
//b7: 
	
}UVGUI_Touch_Dev_Struct_t;
extern UVGUI_Touch_Dev_Struct_t touch_Dev;


u8 UVGUI_Touch_Init(void);
u8 UVGUI_Touch_Scan(u8 tp);

static void UVGUI_Touch_GPIO_Init(void);
static u8 UVGUI_Touch_Get_Adjdata(void);
static void UVGUI_Touch_Save_Adjdata(void);
static void UVFUI_Touch_Adjust(void);
void UVGUI_Touch_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac);
static void UVGUI_Touch_Draw_Touch_Point(u16 x,u16 y,u32 color);
static uint32_t UVGUI_Touch_ReadStorageData(u32 address, unsigned char* pBuffer, u32 len);
static uint32_t UVGUI_Touch_WriteStorageData(u32 address, unsigned char* pBuffer, u32 len);
u16 UVGUI_Touch_Read_XOY(u8 xy);
u8 UVGUI_Touch_Read_XY(u16 *x,u16 *y);
u8 UVGUI_Touch_Read_XY_Twice(u16 *x,u16 *y) ;
u16 UVGUI_Touch_Read_AD(u8 CMD)	 ;
void UVGUI_Touch_Write_Byte(u8 num) ;
#endif


