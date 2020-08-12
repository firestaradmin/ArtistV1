/************************************************************************************
*  Copyright (c), 2019, LXG.
*
* FileName		:UVGUI_Animation.c
* Author		:firestaradmin
* Version		:1.0
* Date			:2020.7.31
* Description	:a universal GUI Lib, usually used in Screen display of Embedded System.
* History		:
*
*
*************************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "UVGUI.h"

#define OBJECT_NUM_MAX 10
extern UVGUI_Screen_Device_t UVGUI_Devices[];
extern uint8_t UVGUI_Current_DevIndex;
static UVGUI_Animation_Object_struct_t object[OBJECT_NUM_MAX];
  

UVGUI_Animation_Object_OutSide_enum UVGUI_Animation_UpdataCoordinate(u8 Index)
{
	if(object[Index].x<=3)
	{	
		object[Index].x = 4;
		return UVGUI_ANIMATION_OBJECT_OUTSIDE;
	}
	else if(object[Index].x>=UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-2)
	{
		object[Index].x = UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-3;
		return UVGUI_ANIMATION_OBJECT_OUTSIDE;
	}
	else if(object[Index].y<=0)
	{
		object[Index].y = 1;
		return UVGUI_ANIMATION_OBJECT_OUTSIDE;
	}
	else if(object[Index].y>=UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-2)
	{
		object[Index].y = UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-3;
		return UVGUI_ANIMATION_OBJECT_OUTSIDE;
	}
	else
	{
		object[Index].x += object[Index].dirx;
		object[Index].y += object[Index].diry;
	}
  return UVGUI_ANIMATION_OBJECT_INSIDE;
}

static u32 UVGUI_Animation_GetObejctDistanceSquare(u16 x0,u16 y0,u16 x1,u16 y1)
{
	return ((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1));
}

void UVGUI_Animation_Init(void) 
{
	int i;
	for(i=0;i<OBJECT_NUM_MAX;i++)
	{

		object[i].color = rand() % ((u32)pow(2,UVGUI_COLOR_DATA_BITS) - 1);
		__ASM("NOP");
		object[i].x = rand()%(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-4)+4;
		__ASM("NOP");
		object[i].y = rand()%UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height;
		
//		object[i].dirx = (rand()%4)-2;
//		object[i].diry = (rand()%4)-2;
		object[i].dirx = (rand()%30-15)*0.1f;
		object[i].diry = (rand()%30-15)*0.1f;
		if(object[i].dirx<0.2&&object[i].dirx>-0.2)
			object[i].dirx = 0.5;
		if(object[i].diry<0.2&&object[i].diry>-0.2)
			object[i].diry = 0.5;
	}
}


void UVGUI_Animation_Mind(void)
{
	int i,j;
	for(i=0;i<OBJECT_NUM_MAX;i++)
	{
		if(UVGUI_Animation_UpdataCoordinate(i) == UVGUI_ANIMATION_OBJECT_OUTSIDE)	//如果对象运动超出边界，重新计算运动增量
		{
			object[i].color = rand() % ((u32)pow(2,UVGUI_COLOR_DATA_BITS) - 1);
			object[i].dirx = (rand()%30-15)*0.1f;
			object[i].diry = (rand()%30-15)*0.1f;
			if(object[i].dirx<0.2&&object[i].dirx>-0.2)
				object[i].dirx = 0.5;
			if(object[i].diry<0.2&&object[i].diry>-0.2)
				object[i].diry = 0.5;
		}
	}
	
	for(i=0;i<OBJECT_NUM_MAX;i++)	//计算运动对象的彼此位置，添加连线
	{
		for(j=0;j<OBJECT_NUM_MAX;j++)
		{
			if((object[i].x-object[j].x)*(object[i].x-object[j].x)+(object[i].y-object[j].y)*(object[i].y-object[j].y)<900)
			{
				UVGUI_DrawColoredLine(object[j].x,object[j].y,object[i].x,object[i].y,rand() % ((u32)pow(2,UVGUI_COLOR_DATA_BITS) - 1));
			}
		}
	}
	
	for(i=0;i<OBJECT_NUM_MAX;i++)	
		UVGUI_FillColoredCircle(object[i].x,object[i].y,2,object[i].color);
}



