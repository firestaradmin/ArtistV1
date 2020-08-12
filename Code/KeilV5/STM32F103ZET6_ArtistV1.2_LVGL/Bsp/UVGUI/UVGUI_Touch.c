/************************************************************************************
*  Copyright (c), 2019, LXG.
*
* FileName		:UVGUI_Touch.c
* Author		:firestaradmin
* Version		:1.0
* Date			:2020.8.1
* Description	:a universal GUI Lib, usually used in Screen display of Embedded System.
* History		:
*
*
*************************************************************************************/
#include <stdlib.h>
#include <math.h>

#include "UVGUI.h"
#include "delay.h"
#include "main.h"
extern UVGUI_Screen_Device_t UVGUI_Devices[];
extern uint8_t UVGUI_Current_DevIndex;
/* 触摸芯片读取指令 */
static u8 CMD_RDX=0XD0;
static u8 CMD_RDY=0X90;

UVGUI_Touch_Dev_Struct_t touch_Dev;


/* UVGUI读取片外储存器以获取屏幕校准参数，用户需要自己实现此函数 （电容屏不需要校准）*/
static uint32_t UVGUI_Touch_ReadStorageData(u32 address, unsigned char* pBuffer, u32 len)
{
	AT24CXX_Read((u16)address,pBuffer, (u16)len);
	return len;
}
/* UVGUI写片外储存器以储存屏幕校准参数，用户需要自己实现此函数（电容屏不需要校准） */
static uint32_t UVGUI_Touch_WriteStorageData(u32 address, unsigned char* pBuffer, u32 len)
{
	AT24CXX_Write((u16)address, pBuffer, (u16)len);
	return len;
}
//触摸屏初始化  		    
//返回值:0,没有进行校准
//       1,进行过校准
u8 UVGUI_Touch_Init(void)
{	

	UVGUI_Touch_GPIO_Init();
	if(UVGUI_Touch_Get_Adjdata())
		return 0;//已经校准
	else		//未校准							    
		UVFUI_Touch_Adjust();//屏幕校准  
		
	UVGUI_Touch_Get_Adjdata();	
	
	return 1; 									 
}

//////////////////////////////////////////////////////////////////////////////////		  
//触摸按键扫描
//tp:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 UVGUI_Touch_Scan(u8 tp)
{			   
	if(PEN==0)//有按键按下
	{
		if(tp)UVGUI_Touch_Read_XY_Twice(&touch_Dev.xPos[0],&touch_Dev.yPos[0]);//读取物理坐标
		else if(UVGUI_Touch_Read_XY_Twice(&touch_Dev.xPos[0],&touch_Dev.yPos[0]))//读取屏幕坐标
		{
	 		touch_Dev.xPos[0]=touch_Dev.xfac*touch_Dev.xPos[0]+touch_Dev.xoff;//将结果转换为屏幕坐标
			touch_Dev.yPos[0]=touch_Dev.yfac*touch_Dev.yPos[0]+touch_Dev.yoff;  
	 	} 
		if((touch_Dev.status&TP_PRES_DOWN)==0)//之前没有被按下
		{		 
			touch_Dev.status=TP_PRES_DOWN|TP_CATH_PRES;//按键按下  
			touch_Dev.xPos[4]=touch_Dev.xPos[0];//记录第一次按下时的坐标
			touch_Dev.yPos[4]=touch_Dev.yPos[0];  	   			 
		}			   
	}else
	{
		if(touch_Dev.status&TP_PRES_DOWN)//之前是被按下的
		{
			touch_Dev.status&=~(1<<7);//标记按键松开	
		}else//之前就没有被按下
		{
			touch_Dev.xPos[4]=0;
			touch_Dev.yPos[4]=0;
			touch_Dev.xPos[0]=0xffff;
			touch_Dev.yPos[0]=0xffff;
		}	    
	}
	return touch_Dev.status&TP_PRES_DOWN;//返回当前的触屏状态
}	  


//SPI写数据
//向触摸屏IC写入1byte数据    
//num:要写入的数据
void UVGUI_Touch_Write_Byte(u8 num)    
{  
	u8 count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN=1;  
		else TDIN=0;   
		num<<=1;    
		TCLK=0; 
		DelayUs(1);
		TCLK=1;		//上升沿有效	        
	}		 			    
} 		 
//SPI读数据 
//从触摸屏IC读取adc值
//CMD:指令
//返回值:读到的数据	   
u16 UVGUI_Touch_Read_AD(u8 CMD)	  
{ 	 
	u8 count=0; 	  
	u16 Num=0; 
	TCLK=0;		//先拉低时钟 	 
	TDIN=0; 	//拉低数据线
	TCS=0; 		//选中触摸屏IC
	UVGUI_Touch_Write_Byte(CMD);//发送命令字
	DelayUs(6);//ADS7846的转换时间最长为6us
	TCLK=0; 	     	    
	DelayUs(1);    	   
	TCLK=1;		//给1个时钟，清除BUSY
	DelayUs(1);    
	TCLK=0; 	     	    
	for(count=0;count<16;count++)//读出16位数据,只有高12位有效 
	{ 				  
		Num<<=1; 	 
		TCLK=0;	//下降沿有效  	    	   
		DelayUs(1);    
 		TCLK=1;
 		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   	//只有高12位有效.
	TCS=1;		//释放片选	 
	return(Num);   
}
//读取一个坐标值(x或者y)
//连续读取READ_TIMES次数据,对这些数据升序排列,
//然后去掉最低和最高LOST_VAL个数,取平均值 
//xy:指令（CMD_RDX/CMD_RDY）
//返回值:读到的数据
#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值
u16 UVGUI_Touch_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=UVGUI_Touch_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//排序
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//升序排列
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 
//读取x,y坐标
//最小值不能少于100.
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
u8 UVGUI_Touch_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=UVGUI_Touch_Read_XOY(CMD_RDX);
	ytemp=UVGUI_Touch_Read_XOY(CMD_RDY);	  												   
	//if(xtemp<100||ytemp<100)return 0;//读数失败
	*x=xtemp;
	*y=ytemp;
	return 1;//读数成功
}
//连续2次读取触摸屏IC,且这两次的偏差不能超过
//ERR_RANGE,满足条件,则认为读数正确,否则读数错误.	   
//该函数能大大提高准确度
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
#define ERR_RANGE 50 //误差范围 
u8 UVGUI_Touch_Read_XY_Twice(u16 *x,u16 *y) 
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;    
    flag=UVGUI_Touch_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=UVGUI_Touch_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-50内
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
}  

//保存校准参数										    
static void UVGUI_Touch_Save_Adjdata(void)
{
	u8 val = 0x0A;
	UVGUI_Touch_WriteStorageData(SAVE_ADDR_BASE,(u8*)&touch_Dev.xfac,14);	//强制保存&touch_Dev.xfac地址开始的14个字节数据，即保存到touch_Dev.touchtype
 	UVGUI_Touch_WriteStorageData(SAVE_ADDR_BASE+14, &val , 1);		//在最后，写0X0A标记校准过了
}

//得到保存在储存器里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
static u8 UVGUI_Touch_Get_Adjdata(void)
{					  
	u8 temp;
	UVGUI_Touch_ReadStorageData(SAVE_ADDR_BASE+14, &temp, 1);//读取标记字,看是否校准过！ 		 
	if(temp==0X0A)//触摸屏已经校准过了			   
 	{ 
		UVGUI_Touch_ReadStorageData(SAVE_ADDR_BASE, (u8*)&touch_Dev.xfac, 14);//读取之前保存的校准数据 
		if(	UVGUI_Devices[UVGUI_Current_DevIndex].screen_Direction == SCREEN_HORIZONTAL ||
			UVGUI_Devices[UVGUI_Current_DevIndex].screen_Direction == SCREEN_HORIZONTAL_180)
		{
			CMD_RDX=0X90;
			CMD_RDY=0XD0;	 
		}else if(	UVGUI_Devices[UVGUI_Current_DevIndex].screen_Direction == SCREEN_VERTICAL ||
					UVGUI_Devices[UVGUI_Current_DevIndex].screen_Direction == SCREEN_VERTICAL_180)	
		{
			CMD_RDX=0XD0;
			CMD_RDY=0X90;	 
		}		 
		return 1;	 
	}
	return 0;
}	 

static void UVGUI_Touch_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOF, ENABLE);	 //使能PB,PF端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 // PB1端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//B1推挽输出
	GPIO_SetBits(GPIOB,GPIO_Pin_1);//上拉

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 // PB2端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//B2上拉输入
	GPIO_SetBits(GPIOF,GPIO_Pin_7);//上拉		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_9;				 // F9，PF11端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);//PF9,PF11推挽输出
	GPIO_SetBits(GPIOF, GPIO_Pin_11|GPIO_Pin_9);//上拉

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				 // PF10端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //上拉输入
	GPIO_Init(GPIOF, &GPIO_InitStructure);//PF10上拉输入
	GPIO_SetBits(GPIOF,GPIO_Pin_10);//上拉		
}
//提示校准结果(各个参数)
void UVGUI_Touch_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
{	  
	UVGUI_SetFrontColor(UVGUI_COLOR_RED);
	UVGUI_ShowString(40,160,(u8*)"x1:", UVGUI_STRING_SHOW_OVERLAY);
 	UVGUI_ShowString(40+80,160,(u8*)"y1:", UVGUI_STRING_SHOW_OVERLAY);
 	UVGUI_ShowString(40,180,(u8*)"x2:", UVGUI_STRING_SHOW_OVERLAY);
 	UVGUI_ShowString(40+80,180,(u8*)"y2:", UVGUI_STRING_SHOW_OVERLAY);
	UVGUI_ShowString(40,200,(u8*)"x3:", UVGUI_STRING_SHOW_OVERLAY);
 	UVGUI_ShowString(40+80,200,(u8*)"y3:", UVGUI_STRING_SHOW_OVERLAY);
	UVGUI_ShowString(40,220,(u8*)"x4:", UVGUI_STRING_SHOW_OVERLAY);
 	UVGUI_ShowString(40+80,220,(u8*)"y4:", UVGUI_STRING_SHOW_OVERLAY);  
 	UVGUI_ShowString(40,240,(u8*)"fac is:", UVGUI_STRING_SHOW_OVERLAY);   

	UVGUI_ShowINT(40+24,160,x0,4,UVGUI_STRING_ALIGN_RIGHT);		//显示数值
	UVGUI_ShowINT(40+24+80,160,y0,4,UVGUI_STRING_ALIGN_RIGHT);	//显示数值
	UVGUI_ShowINT(40+24,180,x1,4,UVGUI_STRING_ALIGN_RIGHT);		//显示数值
	UVGUI_ShowINT(40+24+80,180,y1,4,UVGUI_STRING_ALIGN_RIGHT);	//显示数值
	UVGUI_ShowINT(40+24,200,x2,4,UVGUI_STRING_ALIGN_RIGHT);		//显示数值
	UVGUI_ShowINT(40+24+80,200,y2,4,UVGUI_STRING_ALIGN_RIGHT);	//显示数值
	UVGUI_ShowINT(40+24,220,x3,4,UVGUI_STRING_ALIGN_RIGHT);		//显示数值
	UVGUI_ShowINT(40+24+80,220,y3,4,UVGUI_STRING_ALIGN_RIGHT);	//显示数值
 	UVGUI_ShowINT(40+56,240,fac,3,UVGUI_STRING_ALIGN_RIGHT); 	//显示数值,该数值必须在95~105范围之内.

}

//触摸屏校准代码
//得到四个校准参数
static void UVFUI_Touch_Adjust(void)
{								 
	u16 pos_temp[4][2];//坐标缓存值
	u8  cnt=0;	
	u16 d1,d2;
	u32 tem1,tem2;
	double fac; 	
	u16 outtime=0;
 	cnt=0;			
	UVGUI_SetFrontColor(UVGUI_COLOR_BLUE);
	UVGUI_SetBackColor(UVGUI_COLOR_WHITE);
	UVGUI_Slow_Clear(UVGUI_COLOR_WHITE);//清屏 
	UVGUI_SetFrontColor(UVGUI_COLOR_BLACK);
	UVGUI_SetFontSize(UVGUI_FONT_Size_16);
	UVGUI_ShowString(40,40,(u8*)"screen adjust, please touch point appear", UVGUI_STRING_SHOW_OVERLAY);//显示提示信息
	UVGUI_Touch_Draw_Touch_Point(20,20,UVGUI_COLOR_RED);//画点1 
	touch_Dev.status = 0;	//消除触发信号 
	touch_Dev.xfac = 0;		//xfac用来标记是否校准过,所以校准之前必须清掉!以免错误	 
	while(1)//如果连续10秒钟没有按下,则自动退出
	{
		UVGUI_Touch_Scan(1);//扫描物理坐标
		if((touch_Dev.status&0xc0)==TP_CATH_PRES)//按键按下了一次(此时按键松开了.)
		{	
			outtime=0;		
			touch_Dev.status&=~(1<<6);//标记按键已经被处理过了.
						   			   
			pos_temp[cnt][0]=touch_Dev.xPos[0];
			pos_temp[cnt][1]=touch_Dev.yPos[0];
			cnt++;	  
			switch(cnt)
			{			   
				case 1:						 
					UVGUI_Touch_Draw_Touch_Point(20,20,UVGUI_COLOR_WHITE);				//清除点1 
					UVGUI_Touch_Draw_Touch_Point(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-20,20,UVGUI_COLOR_RED);	//画点2
					break;
				case 2:
 					UVGUI_Touch_Draw_Touch_Point(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-20,20,UVGUI_COLOR_WHITE);	//清除点2
					UVGUI_Touch_Draw_Touch_Point(20,UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-20,UVGUI_COLOR_RED);	//画点3
					break;
				case 3:
 					UVGUI_Touch_Draw_Touch_Point(20,UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-20,UVGUI_COLOR_WHITE);			//清除点3
 					UVGUI_Touch_Draw_Touch_Point(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-20,UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-20,UVGUI_COLOR_RED);	//画点4
					break;
				case 4:	 //全部四个点已经得到
	    		    //对边相等
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,2的距离
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到3,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//不合格
					{
						cnt=0;
 				    	UVGUI_Touch_Draw_Touch_Point(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-20,UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-20,UVGUI_COLOR_WHITE);	//清除点4
   	 					UVGUI_Touch_Draw_Touch_Point(20,20,UVGUI_COLOR_RED);								//画点1
 						UVGUI_Touch_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
 						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,3的距离
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    	UVGUI_Touch_Draw_Touch_Point(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-20,UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-20,UVGUI_COLOR_WHITE);	//清除点4
   	 					UVGUI_Touch_Draw_Touch_Point(20,20,UVGUI_COLOR_RED);								//画点1
 						UVGUI_Touch_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
								   
					//对角线相等
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,4的距离
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,3的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    	UVGUI_Touch_Draw_Touch_Point(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-20,UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-20,UVGUI_COLOR_WHITE);	//清除点4
   	 					UVGUI_Touch_Draw_Touch_Point(20,20,UVGUI_COLOR_RED);								//画点1
 						UVGUI_Touch_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
					//计算结果
					touch_Dev.xfac=(float)(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-40)/(pos_temp[1][0]-pos_temp[0][0]);//得到xfac		 
					touch_Dev.xoff=(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-touch_Dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//得到xoff
						  
					touch_Dev.yfac=(float)(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-40)/(pos_temp[2][1]-pos_temp[0][1]);//得到yfac
					touch_Dev.yoff=(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-touch_Dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//得到yoff  
					if(abs((int32_t)touch_Dev.xfac)>2||abs((int32_t)touch_Dev.yfac)>2)//触屏和预设的相反了.
					{
						cnt=0;
 				    	UVGUI_Touch_Draw_Touch_Point(UVGUI_Devices[UVGUI_Current_DevIndex].screen_Width-20,UVGUI_Devices[UVGUI_Current_DevIndex].screen_Height-20,UVGUI_COLOR_WHITE);	//清除点4
   	 					UVGUI_Touch_Draw_Touch_Point(20,20,UVGUI_COLOR_RED);								//画点1
						UVGUI_ShowString(40,26,(u8*)"TP Need readjust!",UVGUI_STRING_SHOW_OVERLAY);
						touch_Dev.touchtype=!touch_Dev.touchtype;//修改触屏类型.
						if(touch_Dev.touchtype)//X,Y方向与屏幕相反
						{
							CMD_RDX=0X90;
							CMD_RDY=0XD0;	 
						}else				   //X,Y方向与屏幕相同
						{
							CMD_RDX=0XD0;
							CMD_RDY=0X90;	 
						}			    
						continue;
					}		
					UVGUI_SetFrontColor(UVGUI_COLOR_BLUE);
					UVGUI_Fast_Clear(UVGUI_COLOR_WHITE);//清屏
					UVGUI_ShowString(35,110, (u8*)"Touch Screen Adjust OK!", UVGUI_STRING_SHOW_OVERLAY);//校正完成
					DelayMs(1000);
					UVGUI_Touch_Save_Adjdata();  
 					UVGUI_Fast_Clear(UVGUI_COLOR_WHITE);//清屏   
					return;//校正完成				 
			}
		}
		DelayMs(10);
		outtime++;
		if(outtime>1000)
		{
			UVGUI_Touch_Get_Adjdata();
			break;
	 	} 
 	}
}	 





//画一个触摸点
//用来校准用的
//x,y:坐标
//color:颜色
static void UVGUI_Touch_Draw_Touch_Point(u16 x,u16 y,u32 color)
{
	UVGUI_SetFrontColor(color);

	UVGUI_DrawLine(x-12,y,x+13,y);//横线
	UVGUI_DrawLine(x,y-12,x,y+13);//竖线
	UVGUI_DrawPoint(x+1,y+1);
	UVGUI_DrawPoint(x-1,y+1);
	UVGUI_DrawPoint(x+1,y-1);
	UVGUI_DrawPoint(x-1,y-1);
	UVGUI_DrawCircle(x,y,6);//画中心圈
}	  






