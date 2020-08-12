
#include "main.h"
#include "grbl.h"
#include <stdio.h>
#include "ff.h"
#include "diskio.h"
#include "exfuns.h"
#include "fattester.h"

#include "lvgl_grbl_app.h"


/******************************* 全局变量声明 ************************************/
u8 g_Off_Line_Flag = 0;	//脱机执行nc文件标志

//按钮回调函数
static void event_handler(lv_obj_t * obj, lv_event_t event)
{
    
    if(event == LV_EVENT_CLICKED) {//如果按钮按下
        
        //建立一个消息框
        lv_obj_t * mbox = lv_msgbox_create(lv_scr_act(), NULL);//创建消息框
        lv_obj_set_width(mbox, 200);//设置宽度
        static const char * mbox_btns[] = {"ok","cancel", ""};//设置按钮名字
        lv_msgbox_add_btns(mbox, mbox_btns);    /*The default action is close*///给消息框添加一个按钮,默然按钮事件会关闭消息框
        lv_obj_align(mbox, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 2);//按钮居中
    }

}

void lvgl_create_page(void)
{
	//建立一按钮
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn1, event_handler);//按钮按下回调函数，可以不用设置，可以多按钮同一个回调函数
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Button");

	lv_obj_t * slider1 = lv_slider_create(lv_scr_act(), NULL);
}





