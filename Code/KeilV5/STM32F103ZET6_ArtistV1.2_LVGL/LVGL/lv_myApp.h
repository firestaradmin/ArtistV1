/**
 * @file lv_demo_widgets.h
 *
 */

#ifndef LV_myAPP_H
#define LV_myAPP_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f10x.h"
/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
 /**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_msg_question(const char* str);
void lv_msg_inform(const char* str);
void lv_msg_waiting(const char* str);
void lv_updata_axis_label(float *position);
void lv_myApp_init(void);

extern u8 spindle_pwm_on; 
extern u8 spindle_pwm_off; 
extern u8 force_feedrate_flag;
extern u8 off_line_print_flag;
extern const char* file_name;
extern float feedrate;
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_WIDGETS_H*/
