/************************************************************************************
*  Copyright (c), 2019, LXG.
*
* FileName		:UVGUI.h
* Author		:firestaradmin
* Version		:1.0
* Date			:2020.7.25
* Description	:a universal GUI Lib, usually used in Screen display of Embedded System.
* History		:
*
*
*************************************************************************************/

#ifndef _UVGUI_H__
#define _UVGUI_H__

#include "stm32f10x.h"
#include "UVGUI_Animation.h"
#include "UVGUI_Touch.h"
#include "UVGUI_UserConfig.h"
#include "UVGUI_Ascii_Code_Tab.h"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	UVGUI enum define
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* UVGUI枚举:颜色位数（单位BIT） */
typedef enum UVGUI_Color_Data_Bits_enum{
	UVGUI_COLOR_DATA_BITS_1 = 1,
	UVGUI_COLOR_DATA_BITS_16 = 16,
	UVGUI_COLOR_DATA_BITS_18 = 18,
	UVGUI_COLOR_DATA_BITS_24 = 24,
	UVGUI_COLOR_DATA_BITS_32 = 32,
}UVGUI_Color_Data_Bits_enum_t;
/* UVGUI枚举:字体大小*/
typedef enum UVGUI_Screen_Font_Size_enum{
	UVGUI_FONT_Size_8 = 8,
	UVGUI_FONT_Size_12 = 12,
	UVGUI_FONT_Size_16 = 16,
	UVGUI_FONT_Size_24 = 24,
	UVGUI_FONT_Size_32 = 32,
}UVGUI_Screen_Font_Size_enum_t;
/* UVGUI枚举:屏幕显示方向*/
enum UVGUI_Screen_Direction_enum{
	SCREEN_HORIZONTAL = 0,
	SCREEN_VERTICAL = 1,
	SCREEN_HORIZONTAL_180 = 2,
	SCREEN_VERTICAL_180 = 3
};
/* UVGUI枚举:屏幕设备状态	暂时未实现相关功能 */
enum UVGUI_Screen_Device_Status_enum{
	UVGUI_DEVICE_UNINITIALIZED = 0,
	UVGUI_DEVICE_IDLE = 1,
	UVGUI_DEVICE_BUSY = 2,
	UVGUI_DEVICE_ERROR = 3
};
/* UVGUI枚举:字体对齐模式 */
typedef enum UVGUI_String_Alignment_Mode_enum{
	UVGUI_STRING_ALIGN_LEFT= 0,
	UVGUI_STRING_ALIGN_RIGHT= 1
}UVGUI_String_Alignment_Mode_enum_t;

/* UVGUI枚举:字体显示叠加模式 */
typedef enum UVGUI_String_Show_Mode_enum{
	UVGUI_STRING_SHOW_DISOVERLAY= 0,
	UVGUI_STRING_SHOW_OVERLAY= 1
}UVGUI_String_Show_Mode_enum_t;

/* UVGUI枚举:画圆弧参数 */
typedef enum UVGUI_DrawQuarterCircle_Part_enum{
	UCGUI_CIRCLE_PART_UPRIGHT= 0,
	UCGUI_CIRCLE_PART_LOWRIGHT= 1,
	UCGUI_CIRCLE_PART_UPLEFT= 2,
	UCGUI_CIRCLE_PART_LOWLEFT= 3
}UVGUI_DrawQuarterCircle_Part_enum_t;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	UVGUI struct define
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* UVGUI结构体:屏幕设备 */
typedef struct UVGUI_Screen_Device{

	/* Pixels on the X-axis  */
	uint16_t screen_Width;
	/* Pixels on the Y-axis  */
	uint16_t screen_Height;
	
	enum UVGUI_Screen_Device_Status_enum screen_Device_Status;
	
	enum UVGUI_Screen_Direction_enum screen_Direction ;
	
	void (*drawPoint)(uint16_t x, uint16_t y, uint32_t color);
	
	uint32_t (*readPoint)(uint16_t x, uint16_t y);
	
	void (*setWindow)(uint16_t sX, uint16_t sY, uint16_t eX, uint16_t eY);
	
	void (*writeRam)(u32 color);
}UVGUI_Screen_Device_t;





//extern UVGUI_Screen_Device_t UVGUI_Devices[];

/******************* UVGUI_API ******************************/
/* Init */
void UVGUI_Device_Init(UVGUI_Screen_Device_t* dev, uint8_t devIndex);

/* Display set */
void UVGUI_SetCurrentDev(uint8_t devIndex);
void UVGUI_SetFrontColor(uint32_t color);
void UVGUI_SetBackColor(uint32_t color);
void UVGUI_SetFontSize(UVGUI_Screen_Font_Size_enum_t fontSize);
void UVGUI_SetFont(UVGUI_Screen_Font_Size_enum_t fontSize, uint32_t color);

/* String */
void UVGUI_ShowString(uint16_t x,uint16_t y,const uint8_t *pStr,UVGUI_String_Show_Mode_enum_t mode);
void UVGUI_ShowINT(uint16_t x, uint16_t y, long num, uint8_t width, UVGUI_String_Alignment_Mode_enum_t alignment);
void UVGUI_ShowFLOAT(u16 x, u16 y, float num, u8 pointNum);

/* Line */
void UVGUI_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) ;
void UVGUI_DrawColoredLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint32_t color) ;
void UVGUI_DrawWideColoredLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t with, uint32_t color);

/* Rectangle */
void UVGUI_DrawRectangle(int16_t x, int16_t y, int16_t w, int16_t h);
void UVGUI_DrawColoredRectangle(int16_t x, int16_t y, int16_t w, int16_t h,uint32_t color) ;
void UVGUI_Slow_FillRect(u16 x1, u16 y1, u16 x2, u16 y2,u32 color);	
void UVGUI_Fast_FillRect(u16 x1, u16 y1, u16 x2, u16 y2,u32 color);
void UVGUI_Slow_Clear(u32 color);
void UVGUI_Fast_Clear(u32 color);

/* RoundRectangle */
void UVGUI_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r) ;
void UVGUI_DrawColoredRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,uint32_t color) ;

/* Circle */
void UVGUI_DrawCircle(int16_t x0, int16_t y0, int16_t r) ;
void UVGUI_DrawColoredCircle(int16_t x0, int16_t y0, int16_t r,uint32_t color) ;
void UVGUI_FillColoredCircle(int16_t x0, int16_t y0, int16_t r,uint32_t color);
void UVGUI_DrawQuarterCircle(int16_t x0, int16_t y0, int16_t r,UVGUI_DrawQuarterCircle_Part_enum_t part,uint32_t color) ;

/* Tirangle */
void UVGUI_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) ;
void UVGUI_DrawColoredTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2,uint32_t color)  ;	

/* Point */
void UVGUI_DrawPoint(int16_t x, int16_t y);
void UVGUI_DrawColoredPoint(int16_t x, int16_t y, uint32_t color);


	
/* UVGUI_Private_Fun */
static void UVGUI_ShowChar(uint16_t x,uint16_t y,uint8_t chr,UVGUI_String_Show_Mode_enum_t mode);
static void UVGUI_ShowGB2312Char(uint8_t x,uint8_t y,uint8_t *pGB2312Char,UVGUI_String_Show_Mode_enum_t mode);
static void UVGUI_User_Read_FLASH( uint8_t *pBuffer, uint32_t address, uint32_t len);
static void UVGUI_GetGBKCode_from_EXFlash( uint8_t *pBuffer, uint8_t *pGB2312_Char, UVGUI_Screen_Font_Size_enum_t font_Size);
static void UVGUI_FillCircle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t corner, int16_t delta,uint32_t color)  ;
static void UVGUI_DrawCircle_Helper( int16_t x0, int16_t y0, int16_t r, uint8_t corner);
static void UVGUI_DrawColoredCircle_Helper( int16_t x0, int16_t y0, int16_t r, uint8_t corner,uint32_t color) ;
static void UVGUI_DrawVLine(int16_t x, int16_t y, int16_t length,uint32_t color) ;
static void UVGUI_DrawHLine(int16_t x, int16_t y, int16_t length,uint32_t color) ;
static void UVGUI_WriteLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) ;
static void UVGUI_WriteColoredLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint32_t color)  ;
static unsigned char *reverse(unsigned char *s);
static uint8_t my_itoa(u8* str, long n);
#define swap(a, b) { uint16_t t = a; a = b; b = t; }
//static unsigned char *my_strcat(u8 * str1, u8 * str2);
#endif


