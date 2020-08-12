/**
 * @file lv_demo_widgets.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "grbl.h"
#include "lv_myApp.h"
#include "stdio.h"
#include "fattester.h"	 
#include "sdio_sdcard.h"
//#include "usmart.h"
#include "usart.h"
#include "exfuns.h"
//#include "malloc.h"
#include <stdlib.h>
#include <stdio.h>
#include "ff.h"
/*********************
 *      DEFINES
 *********************/
extern lv_img_dsc_t lxg_logo;
/**********************
 *     global varibles
 **********************/
u8 spindle_pwm_on = 70; 
u8 spindle_pwm_off = 50; 
u8 force_feedrate_flag = 0;
u8 off_line_print_flag = 0;
const char* file_name = 0;
float feedrate = 0;
u8 choice_file_flag = 0;
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void slider_event_cb(lv_obj_t* obj, lv_event_t event);
static void btn_up_event_cb(lv_obj_t* obj, lv_event_t event);
static void btn_down_event_cb(lv_obj_t* obj, lv_event_t event);
static void btn_left_event_cb(lv_obj_t* obj, lv_event_t event);
static void btn_right_event_cb(lv_obj_t* obj, lv_event_t event);
static void btn_home_event_cb(lv_obj_t* obj, lv_event_t event);
static void btn_start_event_cb(lv_obj_t* obj, lv_event_t event);
static void btn_stop_event_cb(lv_obj_t* obj, lv_event_t event);
static void sw_feedrate_event_cb(lv_obj_t* obj, lv_event_t event);
static void sw_spindle_event_cb(lv_obj_t* obj, lv_event_t event);
static void roller_event_cb(lv_obj_t* obj, lv_event_t event);
static void filelist_event_cb(lv_obj_t* obj, lv_event_t event);
static void style_init(void);
static void tab_control_init(void);
static void tab_file_init(void);
static void write_rx_buf_byte(uint8_t byte);
/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t* tv;
/****tab_controls****/
static lv_obj_t* tab_controls;
static lv_obj_t* cont_info;
static lv_obj_t* cont_setting;
static lv_obj_t* label_x;
static lv_obj_t* label_y;
static lv_obj_t* btn_up;
static lv_obj_t* btn_down;
static lv_obj_t* btn_left;
static lv_obj_t* btn_right;
static lv_obj_t* roller_step;
static lv_obj_t* btn_stop;
static lv_obj_t* btn_home;
static lv_obj_t* slider_feedrate;
static lv_obj_t* slider_spindle;
//static lv_obj_t* label_feedrate;
//static lv_obj_t* label_spindle;
static lv_obj_t* sw_feedrate;
static lv_obj_t* sw_spindle;
/****tab_files****/
static lv_obj_t* tab_files;
static lv_obj_t* file_list;
static lv_obj_t* label_filename;
lv_obj_t* btn_startprint;
static lv_obj_t* btn_refreash;
/****tab_settings****/
//static lv_obj_t* tab_settings;




static lv_style_t style_btn;
static lv_style_t style_btn_stop;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// void lv_updata_axis_label(float *position);
 
 
 
 
 
lv_obj_t* label;

void lv_myApp_init(void)
{
    style_init();
    
    LV_THEME_DEFAULT_INIT(lv_theme_get_color_primary(), lv_theme_get_color_secondary(),
        LV_THEME_MATERIAL_FLAG_LIGHT,
        lv_theme_get_font_small(), lv_theme_get_font_normal(), lv_theme_get_font_subtitle(), lv_theme_get_font_title());


    tv = lv_tabview_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_pad_left(tv, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 100);
    lv_obj_set_style_local_bg_color(tv, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_style_local_text_font(tv, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, &lv_font_montserrat_24);

    tab_controls = lv_tabview_add_tab(tv, LV_SYMBOL_HOME" Controls");
    tab_files = lv_tabview_add_tab(tv, LV_SYMBOL_SD_CARD" Files");
    //tab_settings = lv_tabview_add_tab(tv, LV_SYMBOL_SETTINGS"Settings");

    

    tab_control_init();
    tab_file_init();
	lv_obj_t* logo = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(logo, &lxg_logo);
	lv_obj_align(logo, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void style_init(void)
{
    lv_style_init(&style_btn_stop);
    lv_style_set_text_font(&style_btn_stop, LV_STATE_PRESSED, lv_theme_get_font_normal());
    lv_style_set_text_color(&style_btn_stop, LV_STATE_PRESSED, LV_COLOR_RED);
    lv_style_set_bg_color(&style_btn_stop, LV_STATE_DEFAULT, LV_COLOR_CYAN);


    lv_style_init(&style_btn);
    lv_style_set_text_font(&style_btn, LV_STATE_PRESSED, lv_theme_get_font_normal());

    
}

static void slider_event_cb(lv_obj_t* slider, lv_event_t e)
{
    if (e == LV_EVENT_VALUE_CHANGED) {
        if (lv_slider_get_type(slider) == LV_SLIDER_TYPE_NORMAL) {	//feedrate
			static char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
            lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, buf);
            feedrate = (float)lv_slider_get_value(slider);
			
        }
        else {	//spindle
			static char buf[32];
            lv_snprintf(buf, sizeof(buf), "%d-%d", lv_slider_get_left_value(slider), lv_slider_get_value(slider));
            lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, buf);
			spindle_pwm_on = lv_slider_get_value(slider);
			spindle_pwm_off = lv_slider_get_left_value(slider);
        }

    }

}

static void btn_up_event_cb(lv_obj_t* obj, lv_event_t event)
{
	u8 buf[20] = "G91\r\nG01 Y";
	u8 i = 0;
    if (event == LV_EVENT_RELEASED) {
		u16 id = lv_roller_get_selected(roller_step);
		switch(id){
			case 0:
				buf[10] = '0';
				buf[11] = '.';
				buf[12] = '1';
				buf[13] = '\r';
				buf[14] = '\n';
				buf[15] = '\0';
				break;
			case 1:
				buf[10] = '1';
				buf[11] = '\r';
				buf[12] = '\n';
				buf[13] = '\0';
				break;
			case 2:
				buf[10] = '1';
				buf[11] = '0';
				buf[12] = '\r';
				buf[13] = '\n';
				buf[14] = '\0';
				break;
			case 3:
				buf[10] = '1';
				buf[11] = '0';
				buf[12] = '0';
				buf[13] = '\r';
				buf[14] = '\n';
				buf[15] = '\0';
				break;
		};
		while(buf[i] != '\0'){
			write_rx_buf_byte(buf[i++]); 
		}
        
    }
}

static void btn_down_event_cb(lv_obj_t* obj, lv_event_t event)
{
	u8 buf[20] = "G91\r\nG01 Y-";
	u8 i = 0;
    if (event == LV_EVENT_RELEASED) {
		u16 id = lv_roller_get_selected(roller_step);
		switch(id){
			case 0:
				buf[10+1] = '0';
				buf[11+1] = '.';
				buf[12+1] = '1';
				buf[13+1] = '\r';
				buf[14+1] = '\n';
				buf[15+1] = '\0';
				break;
			case 1:
				buf[10+1] = '1';
				buf[11+1] = '\r';
				buf[12+1] = '\n';
				buf[13+1] = '\0';
				break;
			case 2:
				buf[10+1] = '1';
				buf[11+1] = '0';
				buf[12+1] = '\r';
				buf[13+1] = '\n';
				buf[14+1] = '\0';
				break;
			case 3:
				buf[10+1] = '1';
				buf[11+1] = '0';
				buf[12+1] = '0';
				buf[13+1] = '\r';
				buf[14+1] = '\n';
				buf[15+1] = '\0';
				break;
		};
		while(buf[i] != '\0'){
			write_rx_buf_byte(buf[i++]); 
		}
        
    }
}
static void btn_left_event_cb(lv_obj_t* obj, lv_event_t event)
{
	u8 buf[20] = "G91\r\nG01 X-";
	u8 i = 0;
    if (event == LV_EVENT_RELEASED) {
		u16 id = lv_roller_get_selected(roller_step);
		switch(id){
			case 0:
				buf[10+1] = '0';
				buf[11+1] = '.';
				buf[12+1] = '1';
				buf[13+1] = '\r';
				buf[14+1] = '\n';
				buf[15+1] = '\0';
				break;
			case 1:
				buf[10+1] = '1';
				buf[11+1] = '\r';
				buf[12+1] = '\n';
				buf[13+1] = '\0';
				break;
			case 2:
				buf[10+1] = '1';
				buf[11+1] = '0';
				buf[12+1] = '\r';
				buf[13+1] = '\n';
				buf[14+1] = '\0';
				break;
			case 3:
				buf[10+1] = '1';
				buf[11+1] = '0';
				buf[12+1] = '0';
				buf[13+1] = '\r';
				buf[14+1] = '\n';
				buf[15+1] = '\0';
				break;
		};
		while(buf[i] != '\0'){
			write_rx_buf_byte(buf[i++]); 
		}
        
    }
}
static void btn_right_event_cb(lv_obj_t* obj, lv_event_t event)
{
	u8 buf[20] = "G91\r\nG01 X";
	u8 i = 0;
    if (event == LV_EVENT_RELEASED) {
		u16 id = lv_roller_get_selected(roller_step);
		switch(id){
			case 0:
				buf[10] = '0';
				buf[11] = '.';
				buf[12] = '1';
				buf[13] = '\r';
				buf[14] = '\n';
				buf[15] = '\0';
				break;
			case 1:
				buf[10] = '1';
				buf[11] = '\r';
				buf[12] = '\n';
				buf[13] = '\0';
				break;
			case 2:
				buf[10] = '1';
				buf[11] = '0';
				buf[12] = '\r';
				buf[13] = '\n';
				buf[14] = '\0';
				break;
			case 3:
				buf[10] = '1';
				buf[11] = '0';
				buf[12] = '0';
				buf[13] = '\r';
				buf[14] = '\n';
				buf[15] = '\0';
				break;
		};
		while(buf[i] != '\0'){
			write_rx_buf_byte(buf[i++]); 
		}
        
    }
}

static void btn_home_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_RELEASED) {
		u8 i = 0;
        u8 buf[20] = "G92 x0 y0 z0\r\n";
		while(buf[i] != '\0'){
			write_rx_buf_byte(buf[i++]); 
		}
    }
}

static void btn_start_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_RELEASED) {
		if(choice_file_flag == 0)
			lv_msg_inform("please choose a file.");
		else
		{
			off_line_print_flag = 1;
			
		}

    }
}
static void btn_refreash_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_RELEASED) {

		lv_list_clean(file_list);
		lv_obj_t* list_btn;
		
		FRESULT res;	  
		char *fn;   /* This function is assuming non-Unicode cfg. */
		#if _USE_LFN
		fileinfo.lfsize = _MAX_LFN * 2 + 1;
		fileinfo.lfname = malloc(fileinfo.lfsize);
		#endif		  

		res = f_opendir(&dir,(const TCHAR*)"0:/"); //打开一个目录
		if (res == FR_OK) 
		{	
			while(1)
			{
				res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
				if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
				//if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
				#if _USE_LFN
					fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
				#else							   
					fn = fileinfo.fname;
				#endif	                                              /* It is a file. */
				//printf("%s/", path);//打印路径	
				//printf("%s\r\n",  fn);//打印文件名	
				if(f_typetell((u8*)fn) == 0x33){
					list_btn = lv_list_add_btn(file_list, LV_SYMBOL_FILE, fn);
					lv_obj_set_event_cb(list_btn, filelist_event_cb);	
				}
			
			} 
		}	  
		free(fileinfo.lfname);
			  
    }
}

static void btn_stop_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_PRESSED) {
        bit_true(sys_rt_exec_state, EXEC_SAFETY_DOOR);
    }
}

static void sw_feedrate_event_cb(lv_obj_t* obj, lv_event_t event)
{
	
    if (event == LV_EVENT_VALUE_CHANGED) {
        if (lv_switch_get_state(obj) == true){
			force_feedrate_flag = 1;
			feedrate = (float)lv_slider_get_value(slider_feedrate);
            //gc_block.values.f = feed;
			u8 buf[10] = "F0\r\n";
			u8 i = 0;
			while(buf[i] != '\0'){
				write_rx_buf_byte(buf[i++]); 
			}
        }
        else{
			force_feedrate_flag = 0;
            //gc_block.values.f = (float)1000;
        }
    }
}

static void sw_spindle_event_cb(lv_obj_t* obj, lv_event_t event)
{
	
    if (event == LV_EVENT_VALUE_CHANGED) {
		u8 i = 0;
		u8 buf[10] = "M03\r\n";
        if (lv_switch_get_state(obj) == true) {
			buf[2] = '3';
        }
        else {
            buf[2] = '5';
        }
		while(buf[i] != '\0'){
			write_rx_buf_byte(buf[i++]); 
		}
    }
}

static void roller_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
//        char buf[30];
//        lv_roller_get_selected_str(obj, buf, sizeof(buf));
//        printf("Selected : %s\n", buf);
       
    }
}

static void filelist_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        //printf("Clicked: %s\n", lv_list_get_btn_text(obj));

        char buf2[255];
		file_name = lv_list_get_btn_text(obj);
        lv_snprintf(buf2, sizeof(buf2), "#0000ff current file:# \n%s", file_name);
        lv_label_set_text(label_filename, buf2);
		choice_file_flag = 1;
		
    }
}

static void tab_control_init(void)
{

    /******************tab_controls*********************/
    cont_info = lv_cont_create(tab_controls, NULL);
    lv_obj_set_drag_parent(cont_info, true);
    lv_obj_set_size(cont_info, 180, 100);
    lv_obj_align(cont_info, tab_controls, LV_ALIGN_IN_BOTTOM_LEFT, 10, -20);


    cont_setting = lv_cont_create(tab_controls, NULL);
    lv_obj_set_drag_parent(cont_setting, true);
    lv_obj_set_size(cont_setting, 300, 100);
    lv_obj_align(cont_setting, tab_controls, LV_ALIGN_IN_TOP_LEFT, 10, 20);


    label = lv_label_create(cont_info, NULL);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, "#0000ff X :#");
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    lv_obj_align(label, cont_info, LV_ALIGN_IN_TOP_LEFT, 10, 20);

    label_x = lv_label_create(cont_info, NULL);
    //lv_obj_set_width(label, 50);
    lv_label_set_text(label_x, "0 mm");
    lv_obj_set_style_local_text_font(label_x, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    //lv_obj_set_style_local_value_align(label_x, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
    lv_obj_align(label_x, cont_info, LV_ALIGN_IN_TOP_RIGHT, -10, 20);

    label = lv_label_create(cont_info, NULL);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, "#0000ff Y :#");
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    lv_obj_align(label, cont_info, LV_ALIGN_IN_BOTTOM_LEFT, 10, -20);

    label_y = lv_label_create(cont_info, NULL);
    lv_obj_set_style_local_text_font(label_y, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    //lv_obj_set_width(label, 50);
    lv_label_set_text(label_y, "0 mm");
	//lv_obj_set_style_local_value_align(label_y, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
    lv_obj_align(label_y, cont_info, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -20);



    btn_up = lv_btn_create(tab_controls, NULL);
    lv_obj_set_event_cb(btn_up, btn_up_event_cb);
    lv_obj_add_style(btn_up, LV_BTN_PART_MAIN, &style_btn);
    lv_obj_set_size(btn_up, 40, 40);
    label = lv_label_create(btn_up, NULL);
    lv_label_set_text(label, LV_SYMBOL_UP);
    lv_obj_align(btn_up, tab_controls, LV_ALIGN_IN_BOTTOM_MID, 25, -75);

    btn_down = lv_btn_create(tab_controls, btn_up);
    lv_obj_set_event_cb(btn_down, btn_down_event_cb);
    lv_obj_add_style(btn_down, LV_BTN_PART_MAIN, &style_btn);
    //lv_obj_set_size(btn_up, 40, 40);
    label = lv_label_create(btn_down, NULL);
    lv_label_set_text(label, LV_SYMBOL_DOWN);
    lv_obj_align(btn_down, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    btn_left = lv_btn_create(tab_controls, btn_up);
    lv_obj_set_event_cb(btn_left, btn_left_event_cb);
    lv_obj_add_style(btn_left, LV_BTN_PART_MAIN, &style_btn);
    //lv_obj_set_size(btn_up, 40, 40);
    label = lv_label_create(btn_left, NULL);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_align(btn_left, btn_down, LV_ALIGN_OUT_LEFT_MID, -5, 0);

    btn_right = lv_btn_create(tab_controls, btn_up);
    lv_obj_set_event_cb(btn_right, btn_right_event_cb);
    lv_obj_add_style(btn_right, LV_BTN_PART_MAIN, &style_btn);
    //lv_obj_set_size(btn_up, 40, 40);
    label = lv_label_create(btn_right, NULL);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_align(btn_right, btn_down, LV_ALIGN_OUT_RIGHT_MID, 5, 0);


    roller_step = lv_roller_create(tab_controls, NULL);
    lv_obj_set_event_cb(roller_step, roller_event_cb);
    lv_roller_set_options(roller_step,
        "0.1 mm\n"
        "1 mm\n"
        "10 mm\n"
        "100 mm",
        LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(roller_step, 1, LV_ANIM_OFF);
    lv_roller_set_visible_row_count(roller_step, 3);
    lv_obj_set_size(roller_step, 100, 100);
    lv_obj_align(roller_step, tab_controls, LV_ALIGN_IN_RIGHT_MID, -15, 60);
    //lv_obj_set_event_cb(roller_step, event_handler);
    label = lv_label_create(tab_controls, NULL);
    lv_label_set_text(label, "step\nsize:");
    lv_obj_align(label, roller_step, LV_ALIGN_OUT_LEFT_TOP, -5, 0);


    btn_home = lv_btn_create(tab_controls, NULL);
    lv_obj_set_event_cb(btn_home, btn_home_event_cb);
    lv_obj_add_style(btn_home, LV_BTN_PART_MAIN, &style_btn);
    lv_obj_set_size(btn_home, 140, 40);
    label = lv_label_create(btn_home, NULL);
    lv_label_set_text(label, "SET ZERO");
    lv_obj_align(btn_home, tab_controls, LV_ALIGN_IN_RIGHT_MID, -15, -85);


    btn_stop = lv_btn_create(tab_controls, NULL);
    lv_obj_set_event_cb(btn_stop, btn_stop_event_cb);
    lv_obj_add_style(btn_stop, LV_BTN_PART_MAIN, &style_btn_stop);
    lv_obj_set_size(btn_stop, 140, 40);
    label = lv_label_create(btn_stop, NULL);
    lv_label_set_text(label, "STOP");
    lv_obj_align(btn_stop, btn_home, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);


    slider_feedrate = lv_slider_create(cont_setting, NULL);
    lv_slider_set_range(slider_feedrate, 50, 6000);
    lv_slider_set_value(slider_feedrate, 4000, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider_feedrate, slider_event_cb);
    lv_obj_set_size(slider_feedrate, 170, 15);
    lv_obj_align(slider_feedrate, NULL, LV_ALIGN_IN_LEFT_MID, 10, -20);
    lv_obj_set_style_local_margin_top(slider_feedrate, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(slider_feedrate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_obj_set_style_local_value_ofs_y(slider_feedrate, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(slider_feedrate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(slider_feedrate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(slider_feedrate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(slider_feedrate, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);


    slider_spindle = lv_slider_create(cont_setting, slider_feedrate);
    lv_slider_set_range(slider_spindle, 50, 100);
    lv_slider_set_type(slider_spindle, LV_SLIDER_TYPE_RANGE);
    lv_slider_set_value(slider_spindle, spindle_pwm_on, LV_ANIM_OFF);
    lv_slider_set_left_value(slider_spindle, spindle_pwm_off, LV_ANIM_OFF);
    lv_obj_set_style_local_value_ofs_y(slider_spindle, LV_SLIDER_PART_INDIC, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_font(slider_spindle, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_obj_set_style_local_value_opa(slider_spindle, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_event_cb(slider_spindle, slider_event_cb);
    lv_obj_set_width(slider_spindle, 170);
    lv_obj_align(slider_spindle, NULL, LV_ALIGN_IN_LEFT_MID, 10, 20);
    lv_obj_set_style_local_transition_time(slider_spindle, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(slider_spindle, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(slider_spindle, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);



    sw_feedrate = lv_switch_create(cont_setting, NULL);
    lv_obj_set_event_cb(sw_feedrate, sw_feedrate_event_cb);
    lv_obj_set_size(sw_feedrate, 40, 20);
    lv_obj_align(sw_feedrate, slider_feedrate, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_set_style_local_value_font(sw_feedrate, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_obj_set_style_local_value_str(sw_feedrate, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, "Force\nfeedRate");
    lv_obj_set_style_local_value_align(sw_feedrate, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_RIGHT_MID);
    lv_obj_set_style_local_value_ofs_x(sw_feedrate, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, 5);

    sw_spindle = lv_switch_create(cont_setting, NULL);
    lv_obj_set_event_cb(sw_spindle, sw_spindle_event_cb);
    lv_obj_set_size(sw_spindle, 40, 20);
    lv_obj_align(sw_spindle, slider_spindle, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_set_style_local_value_font(sw_spindle, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_obj_set_style_local_value_str(sw_spindle, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, "Pen\ndrop");
    lv_obj_set_style_local_value_align(sw_spindle, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_RIGHT_MID);
    lv_obj_set_style_local_value_ofs_x(sw_spindle, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, 5);
	
	lv_event_send(slider_feedrate, LV_EVENT_VALUE_CHANGED, NULL);
    lv_event_send(slider_spindle, LV_EVENT_VALUE_CHANGED, NULL);      /*To refresh the text*/
}


static void tab_file_init(void)
{

    /*Create a list*/
    file_list = lv_list_create(tab_files, NULL);
    lv_obj_set_size(file_list, lv_obj_get_width_fit(tab_files)-40, 180);
    lv_obj_align(file_list, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
	//lv_obj_set_drag(file_list, true);



    label_filename = lv_label_create(tab_files, NULL);
    lv_obj_align(label_filename, tab_files, LV_ALIGN_IN_TOP_LEFT, 10, 10);
    lv_label_set_recolor(label_filename, true);
	lv_obj_set_style_local_text_font(label_filename, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_16);
    lv_label_set_text(label_filename, "#0000ff current file:# \n---------------");
    

    btn_startprint = lv_btn_create(tab_files, NULL);
    lv_btn_set_checkable(btn_startprint, true);
    lv_obj_set_event_cb(btn_startprint, btn_start_event_cb);
    lv_obj_add_style(btn_startprint, LV_BTN_PART_MAIN, &style_btn);
    lv_obj_set_size(btn_startprint, 80, 40);
    label = lv_label_create(btn_startprint, NULL);
    lv_label_set_text(label, "print");
    lv_obj_align(btn_startprint, tab_files, LV_ALIGN_IN_TOP_RIGHT, -10, 10);
	

    btn_refreash = lv_btn_create(tab_files, NULL);
    lv_obj_set_event_cb(btn_refreash, btn_refreash_event_cb);
    lv_obj_add_style(btn_refreash, LV_BTN_PART_MAIN, &style_btn);
    lv_obj_set_size(btn_refreash, 80, 40);
    label = lv_label_create(btn_refreash, NULL);
    lv_label_set_text(label, "refrsh");
    lv_obj_align(btn_refreash, tab_files, LV_ALIGN_IN_TOP_RIGHT, -100, 10);


}
extern uint8_t serial_rx_buffer[];
extern uint8_t serial_rx_buffer_head ;
extern volatile uint8_t serial_rx_buffer_tail;
static void write_rx_buf_byte(uint8_t byte)
{
	uint8_t next_head;
	switch (byte) {
		case CMD_STATUS_REPORT: bit_true_atomic(sys_rt_exec_state, EXEC_STATUS_REPORT); break; // Set as true  设为真
		case CMD_CYCLE_START:   bit_true_atomic(sys_rt_exec_state, EXEC_CYCLE_START); break; // Set as true	   设为真
		case CMD_FEED_HOLD:     bit_true_atomic(sys_rt_exec_state, EXEC_FEED_HOLD); break; // Set as true	   设为真
		case CMD_SAFETY_DOOR:   bit_true_atomic(sys_rt_exec_state, EXEC_SAFETY_DOOR); break; // Set as true	   设为真
		case CMD_RESET:         mc_reset(); break; // Call motion control reset routine. 调用运动控制复位程序。
		default: // Write character to buffer   写字符到缓冲区 
		next_head = serial_rx_buffer_head + 1;
		if (next_head == RX_BUFFER_SIZE) { next_head = 0; }
		
		// Write data to buffer unless it is full.	将数据写入缓冲区，除非它已满。
		if (next_head != serial_rx_buffer_tail) {
			serial_rx_buffer[serial_rx_buffer_head] = byte;
			serial_rx_buffer_head = next_head;    
	
			#ifdef ENABLE_XONXOFF
					if ((serial_get_rx_buffer_count() >= RX_BUFFER_FULL) && flow_ctrl == XON_SENT) {
						flow_ctrl = SEND_XOFF;
						UCSR0B |=  (1 << UDRIE0); // Force TX
					} 
			#endif
	
		}
		//TODO: else alarm on overflow?
	}
}


//static lv_obj_t* label_x;
//static lv_obj_t* label_y;

void lv_updata_axis_label(float *position)
{
	char buf[12];
	sprintf(buf, "%.1f mm", *(position+X_AXIS));
	lv_label_set_text(label_x, buf);
	sprintf(buf, "%.1f mm", *(position+Y_AXIS));
	lv_label_set_text(label_y, buf);
	lv_obj_align(label_y, cont_info, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -20);
	lv_obj_align(label_x, cont_info, LV_ALIGN_IN_TOP_RIGHT, -10, 20);
}

void lv_msg_question(const char* str)
{
	//建立一个消息框
	lv_obj_t* mbox = lv_msgbox_create(lv_scr_act(), NULL);//创建消息框
	lv_obj_set_width(mbox, 300);//设置宽度
	static const char * mbox_btns[] = {"Yes","No", ""};//设置按钮名字
	lv_msgbox_add_btns(mbox, mbox_btns);    /*The default action is close*///给消息框添加一个按钮,默然按钮事件会关闭消息框
	lv_obj_align(mbox, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 2);//按钮居中
	lv_msgbox_set_text(mbox, str);
}

void lv_msg_inform(const char* str)
{
	//建立一个消息框
	lv_obj_t* mbox = lv_msgbox_create(lv_scr_act(), NULL);//创建消息框
	lv_obj_set_width(mbox, 300);//设置宽度
	static const char * mbox_btns[] = {"OK", ""};//设置按钮名字
	lv_msgbox_add_btns(mbox, mbox_btns);    /*The default action is close*///给消息框添加一个按钮,默然按钮事件会关闭消息框
	lv_obj_align(mbox, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 2);//按钮居中
	lv_msgbox_set_text(mbox, str);
}

void lv_msg_waiting(const char* str)
{
	//建立一个消息框
	lv_obj_t* mbox = lv_msgbox_create(lv_scr_act(), NULL);//创建消息框
	lv_obj_set_width(mbox, 300);//设置宽度
//	static const char * mbox_btns[] = {"OK", ""};//设置按钮名字
//	lv_msgbox_add_btns(mbox, mbox_btns);    /*The default action is close*///给消息框添加一个按钮,默然按钮事件会关闭消息框
	lv_obj_align(mbox, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 2);//按钮居中
	lv_msgbox_set_text(mbox, str);
	//lv_obj_del();
}
