#ifndef __LCD_ILI9486_H
#define __LCD_ILI9486_H		
#include "stm32f10x.h"
#include <stdlib.h>
#include "main.h"
  
//LCD重要参数集
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				//LCD ID
	u8  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	u16	wramcmd;		//开始写gram指令
	u16  rramcmd;   //开始读gram指令
	u16  setxcmd;		//设置x坐标指令
	u16  setycmd;		//设置y坐标指令 
}_lcd_dev; 	  

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数
//LCD的画笔颜色和背景色	   
extern u16  POINT_COLOR;//默认红色    
extern u16  BACK_COLOR; //背景颜色.默认为白色


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------LCD端口定义---------------- 
#define LCD_RES_Clr()  GPIO_ResetBits(GPIOG,GPIO_Pin_15)//RES
#define LCD_RES_Set()  GPIO_SetBits(GPIOG,GPIO_Pin_15)

#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOG,GPIO_Pin_14)//BLK
#define LCD_BLK_Set()  GPIO_SetBits(GPIOG,GPIO_Pin_14) 	 


//LCD地址结构体
typedef struct
{
	vu16 LCD_REG;
	vu16 LCD_RAM;
} LCD_TypeDef;

/*
NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 所以地址是 0x6c000000
使用A10作为数据命令区分线（连接至屏幕的RS脚，用于数据命令选择） 
注意地址设置时STM32内部会右移一位对其! 因为使用的是16位模式	
*/
#define LCD_BASE        ((u32)(0x6C000000 | 0x000007FE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
	 
//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向
#define USE_HORIZONTAL  	 3 //定义液晶屏顺时针旋转方向 	0-0度旋转，1-90度旋转，2-180度旋转，3-270度旋转
//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)		


/* API */
void LCD_Init(void);												//初始化
void LCD_Black_On(void);											//开显示
void LCD_Black_Off(void);											//关显示
void LCD_Clear(u16 color);	 										//清屏
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);		//设置窗口
void LCD_SetCursor(u16 Xpos, u16 Ypos);								//设置光标

void LCD_DrawPoint(u16 x,u16 y);											//画点
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color);								//快速画点
u16  LCD_ReadPoint(u16 x,u16 y); 											//读点 

void LCD_Draw_Circle(u16 x0,u16 y0,u8 r);						 			//画圆
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);							//画线
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   				//画矩形
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);		   				//填充单色
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color);				//填充指定颜色
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode);						//显示一个字符
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size);  						//显示一个数字
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode);				//显示 数字
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p);		//显示一个字符串,12/16字体


void LCD_DrawPoint_forUVGUI(u16 x,u16 y,u32 color);								//快速画点
void LCD_Set_Window_ForUVGUI(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd);
void LCD_WriteRAM_ForUVGUI(u32 RGB_Code);
void LCD_Display_Dir(u8 dir);								//设置屏幕显示方向	
void LCD_WriteRAM_Prepare(void);
void LCD_ReadRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
void LCD_WR_REG(u16 regval);
void LCD_WR_DATA(u16 data);
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue);
u16 LCD_ReadReg(u16 LCD_Reg);

static void LCD_GPIO_Init(void);

#endif  
	 
	 



