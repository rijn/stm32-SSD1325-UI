/*
 * Rijn
 * pixelnfinite.com
 * 2015/02/25
 */

#ifndef __OLED_H
#define __OLED_H 	

#include "stm32f10x.h"

#define         RCC_APB2Periph_OLED_PORT        		RCC_APB2Periph_GPIOA
#define         OLED_PORT                       		GPIOA
#define         OLED_RST_PIN                    		GPIO_Pin_12
#define         OLED_RST_L                      		GPIO_ResetBits(GPIOA, GPIO_Pin_12);
#define         OLED_RST_H                      		GPIO_SetBits(GPIOA, GPIO_Pin_12);
#define         OLED_MISO_PIN                   		GPIO_Pin_6
#define	        OLED_MISO_PIN_L                   		GPIO_ResetBits(GPIOA, GPIO_Pin_6);
#define         OLED_MISO_PIN_H                   		GPIO_SetBits(GPIOA, GPIO_Pin_6);
#define         OLED_NSS_PIN                    		GPIO_Pin_4
#define	        OLED_NSS_L                      		GPIO_ResetBits(GPIOA, GPIO_Pin_4);
#define         OLED_NSS_H                     			GPIO_SetBits(GPIOA, GPIO_Pin_4);
#define         OLED_DC_PIN                    			GPIO_Pin_8
#define	        OLED_DC_L                      			GPIO_ResetBits(GPIOA, GPIO_Pin_8);
#define         OLED_DC_H                               GPIO_SetBits(GPIOA, GPIO_Pin_8);
#define  		OLED_SCLK_PIN							GPIO_Pin_5
#define			OLED_MOSI_PIN							GPIO_Pin_7

#define 		XLevelL									0x02
#define 		XLevelH									0x10
#define 		XLevel									((XLevelH&0x0F)*16+XLevelL)

#define 		Max_Column								0x3F			// 128/2-1 (Total Columns Devided by 2)
#define 		Max_Row									0x3F			// 64-1
#define			Brightness								0xFF

#define			COMPONENT_NULL							0x00
#define			COMPONENT_LABEL_ENABLED					0x01
#define			COMPONENT_LABEL_DISABLED				0x02
#define			COMPONENT_LABEL_HIDDEN					0x03
#define			COMPONENT_BUTTON_ENABLED 				0x04
#define			COMPONENT_BUTTON_DISABLED 		 	    0x05
#define			COMPONENT_BUTTON_HIDDEN  				0x06
#define			COMPONENT_RADIO_TRUE  					0x07
#define			COMPONENT_RADIO_FALSE  					0x08
#define			COMPONENT_RADIO_DISABLED  			    0x09
#define			COMPONENT_RADIO_HIDDEN  				0x0A
#define			COMPONENT_TEXT_ENABLED					0x0B
#define			COMPONENT_TEXT_DISABLED					0x0C
#define			COMPONENT_TEXT_HIDDEN					0x0D
#define			COMPONENT_PBAR_ENABLED					0x0E
#define			COMPONENT_PBAR_DISABLED					0x0F
#define			COMPONENT_WINDOW_ENABLED				0x10
#define			COMPONENT_WINDOW_HIDDEN					0x11
#define			COMPONENT_CHART_ENABLED					0x12
#define			COMPONENT_CHART_HIDDEN					0x13
#define			COMPONENT_LISTITEM_ENABLED			    0x14
#define			COMPONENT_LISTITEM_DISABLED			    0x15
#define			COMPONENT_LISTITEM_HIDDEN				0x16
#define			COMPONENT_SRADIO_TRUE  					0x17
#define			COMPONENT_SRADIO_FALSE                  0x18
#define			COMPONENT_SRADIO_DISABLED  		     	0x19
#define			COMPONENT_SRADIO_HIDDEN  				0x1A
#define			COMPONENT_BLANK                         0x1B

#define			COMPONENT_MAX_INDEX 						0x15

void SPI1_Configuration(void);

void OLED_WrByte(u8 data);
void Write_Command(unsigned char Data);
void Write_Data(unsigned char Data);

void OLED_Init(void);
void Set_Start_Column(unsigned char d);
void Set_Addressing_Mode(unsigned char d);
void Set_Column_Address(unsigned char a, unsigned char b);
void Set_Page_Address(unsigned char a, unsigned char b);
void Set_Start_Line(unsigned char d);
void Set_Contrast_Control(unsigned char d);
void Set_Area_Brightness(unsigned char d);
void Set_Segment_Remap(unsigned char d);
void Set_Entire_Display(unsigned char d);
void Set_Inverse_Display(unsigned char d);
void Set_Multiplex_Ratio(unsigned char d);
void Set_Dim_Mode(unsigned char a, unsigned char b);
void Set_Master_Config(unsigned char d);
void Set_Display_On_Off(unsigned char d);
void Set_Start_Page(unsigned char d);
void Set_Common_Remap(unsigned char d);
void Set_Display_Offset(unsigned char d);
void Set_Display_Clock(unsigned char d);
void Set_Area_Color(unsigned char d);
void Set_Precharge_Period(unsigned char d);
void Set_Common_Config(unsigned char d);
void Set_VCOMH(unsigned char d);
void Set_Read_Modify_Write(unsigned char d);
void Set_NOP(void);


void Fill_RAM(unsigned char Data);
void Fill_Block(unsigned char Data, unsigned char a, unsigned char b, unsigned char c, unsigned char d);
void Checkerboard(void);
void Frame(void);
void Show_String(unsigned char a, unsigned char *Data_Pointer, unsigned char b, unsigned char c,unsigned char k);
void Show_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d,unsigned char k);
void Deactivate_Scroll(void);
void Fade_In(void);
void Fade_Out(void);
void Sleep(unsigned char a);
void GA_Option(unsigned char d);
void Set_Remap_Format(unsigned char d);
void Set_Current_Range(unsigned char d);
void Set_Gray_Scale_Table(void);
void Set_Contrast_Current(unsigned char d);
void Set_Frame_Frequency(unsigned char d);
void Set_Phase_Length(unsigned char d);
void Set_Precharge_Voltage(unsigned char d);
void Set_Precharge_Compensation(unsigned char a, unsigned char b);
void Set_VSL(unsigned char d);
void Set_Display_Mode(unsigned char d);
void Set_Row_Address(unsigned char a, unsigned char b);
void Vertical_Scroll(unsigned char a, unsigned char b, unsigned char c);
void Horizontal_Scroll(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
void Fade_Scroll(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
void Grayscale(void);
void Draw_Rectangle(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e);
void Copy(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f);
void Show_dot(unsigned char x, unsigned char y, unsigned char color);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 mode);
void Show_Font57(unsigned char b, unsigned char c, unsigned char d,unsigned char k);
void Show_Font68(unsigned char x, unsigned char y, unsigned char c,unsigned char k);

void Next_Frame(void);
void Show_XGS(void);
void Draw_Dot(u8 x, u8 y, u8 z, u8 color);
void Clean_Screen(u8 x0, u8 y0, u8 x1, u8 y1, u8 z);
void Draw_Char(u8 x,u8 y,u8 z,u8 chr,u8 font_color,u8 back_color);
void Draw_String(u8 x,u8 y,u8 z,u8 chr[],u8 font_color,u8 back_color);
void Draw_5x7Char(u8 x,u8 y,u8 z,u8 chr,u8 font_color,u8 back_color);
void Draw_5x7String(u8 x,u8 y,u8 z,u8 chr[],u8 font_color,u8 back_color);
void Draw_4x6Char(u8 x,u8 y,u8 z,u8 chr,u8 font_color,u8 back_color);
void Draw_4x6String(u8 x,u8 y,u8 z,u8 chr[],u8 font_color,u8 back_color);
void Draw_Icon(u8 x,u8 y,u8 z,u8 chr,u8 font_color,u8 back_color);
void Draw_Notification(u8 title[], u8 chr[]);
void Draw_Reverse(u8 x0, u8 y0, u8 x1, u8 y1, u8 z);
u8   Draw_Menu(u8 menu[][12], u8 x0, u8 y0, u8 x1, u8 y1, u8 default_select);
void Draw_Component(u8 type, u8 index, u8 x0, u8 y0, u8 x1, u8 y1, u8 z, u8 c0, u8 c1, u8 chr[]);
void Draw_Logo(u8 z, u8 chr[]);
void Update_Component(u8 select_index);
void Change_Index(s8 dx);
void Click_Component(void);
void Recycle_Component(u8 indexA, u8 indexB);
u8 Return_Index_Available(u8 type);

#endif
