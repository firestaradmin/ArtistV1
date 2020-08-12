/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "LCD_ILI9486.h"
#include "uvgui.h"
#include "uvgui_Touch.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
#if LV_USE_GPU
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
					const lv_area_t * fill_area, lv_color_t color);
#endif
bool my_touchpad_read(struct _lv_indev_drv_t * indev, lv_indev_data_t * data);
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /* LVGL requires a buffer where it draws the objects. The buffer's has to be greater than 1 display row
     *
     * There are three buffering configurations:
     * 1. Create ONE buffer with some rows: 
     *      LVGL will draw the display's content here and writes it to your display
     * 
     * 2. Create TWO buffer with some rows: 
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     * 
     * 3. Create TWO screen-sized buffer: 
     *      Similar to 2) but the buffer have to be screen sized. When LVGL is ready it will give the
     *      whole frame to display. This way you only need to change the frame buffer's address instead of
     *      copying the pixels.
     * */

    /* Example for 1) */
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf1[LV_HOR_RES_MAX * 25];                      /*A buffer for 10 rows*/
    lv_disp_buf_init(&disp_buf, buf1, NULL, LV_HOR_RES_MAX * 25);   /*Initialize the display buffer*/

	/* Example for 2) */
//	static lv_disp_buf_t disp_buf;
//	static lv_color_t buf1[LV_HOR_RES_MAX * 10];                        /*A buffer for 10 rows*/
//	static lv_color_t buf2[LV_HOR_RES_MAX * 10];                        /*An other buffer for 10 rows*/
//	lv_disp_buf_init(&disp_buf, buf1, buf2, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/

//    /* Example for 3) */
//	lv_disp_buf_t disp_buf;
//	lv_color_t buf1[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*A screen sized buffer*/
//	lv_color_t buf2[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*An other screen sized buffer*/
//  lv_disp_buf_init(&disp_buf, buf1, buf2, LV_HOR_RES_MAX * LV_VER_RES_MAX);   /*Initialize the display buffer*/


    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.buffer = &disp_buf;

#if LV_USE_GPU
    /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/

    /*Blend two color array using opacity*/
    disp_drv.gpu_blend_cb = gpu_blend;

    /*Fill a memory array with a color*/
    disp_drv.gpu_fill_cb = gpu_fill;
#endif

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
	
	
	
	/* input device Init */
	lv_indev_drv_t indev_drv;                  /*Descriptor of a input device driver*/
	lv_indev_drv_init(&indev_drv);             /*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
	indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
	lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{
    /*You code here*/
	LCD_Init();
	/* define a struct variable. if your device is not support part of function, you can use NULL */
	UVGUI_Screen_Device_t UVGUI_Screen_Device;
	
	/* necessary parameter  */
	UVGUI_Screen_Device.drawPoint = LCD_DrawPoint_forUVGUI;
	UVGUI_Screen_Device.screen_Width = 480;
	UVGUI_Screen_Device.screen_Height = 320;
	UVGUI_Screen_Device.screen_Direction = SCREEN_HORIZONTAL;
	UVGUI_Screen_Device.readPoint = NULL;
	
	/* unnecessary parameter, but if you want to use faster refreash function. */
	UVGUI_Screen_Device.setWindow = LCD_Set_Window_ForUVGUI;
	UVGUI_Screen_Device.writeRam = LCD_WriteRAM_ForUVGUI;
	
	/* Init UVGUI device  */
	UVGUI_Device_Init(&UVGUI_Screen_Device, 0);
	UVGUI_Touch_Init();
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

    int32_t x;
    int32_t y;
	LCD_SetWindows(area->x1, area->y1, area->x2, area->y2);
	//LCD_WriteRAM_Prepare();
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
			LCD_WriteRAM(color_p->full);
            color_p++;
        }
    }

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}


/*OPTIONAL: GPU INTERFACE*/
#if LV_USE_GPU

/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/
    uint32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/
    int32_t x, y;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

    for(y = fill_area->y1; y <= fill_area->y2; y++) {
        for(x = fill_area->x1; x <= fill_area->x2; x++) {
            dest_buf[x] = color;
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }
}

void touchpad_get_xy(lv_coord_t *pX, lv_coord_t *pY)
{
	UVGUI_Touch_Scan(0);
	*pX = touch_Dev.xPos[0];
	*pY = touch_Dev.yPos[0];
}

bool my_touchpad_read(struct _lv_indev_drv_t * indev, lv_indev_data_t * data)
{
	data->state = (PEN==0) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
	if(data->state == LV_INDEV_STATE_PR) touchpad_get_xy(&data->point.x, &data->point.y);

	return false; /*Return `false` because we are not buffering and no more data to read*/
}


#endif  /*LV_USE_GPU*/

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
