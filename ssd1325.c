/*
 * Rijn
 * pixelnfinite.com
 * 2015/02/25
 */

#include "ssd1325.h"
#include "stm32f10x.h"
#include "stdio.h"
#include "delay.h"
#include "db.h"
#include "keyboard.h"

u8 OLED_PIXEL[2][65][129];
u8 OLED_STATUS[10][128];
u8 OLED_LAYER_SHIFT[3][2];

struct OLED_COMPONENT
{
	u8 type;
	u8 x0;
	u8 y0;
	u8 x1;
	u8 y1;
	u8 c0;
	u8 c1;
	u8 z;
	u8 chr[33];
};

extern u8 record_data[80];

struct OLED_COMPONENT OLED_COMPONENT_LIST[20];
u8 COMPONENT_INDEX = 0x00;

typedef struct 
{
	u8 hour;
	u8 min;
	u8 sec;			
	u8 w_year;
	u8  w_month;
	u8  w_date;
	u8  week;		 
}_calendar_obj;					 
extern _calendar_obj calendar;
		


void SPI1_Configuration(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1|RCC_APB2Periph_OLED_PORT|RCC_APB2Periph_AFIO, ENABLE); 

	GPIO_InitStructure.GPIO_Pin = OLED_MOSI_PIN|OLED_SCLK_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_Init(OLED_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = OLED_MISO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(OLED_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  OLED_RST_PIN|OLED_NSS_PIN|OLED_DC_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(OLED_PORT, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_SSOutputCmd(SPI1, ENABLE);

	SPI_Cmd(SPI1, ENABLE);
	SPI_SSOutputCmd(SPI1, ENABLE);
}

void OLED_WrByte(u8 data)
{
		
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		//while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == SET);
    SPI_I2S_SendData(SPI1, data);
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
		SPI_I2S_ClearFlag(SPI1,SPI_I2S_FLAG_RXNE);
		SPI_I2S_ClearFlag(SPI1,SPI_I2S_FLAG_TXE);
		
}

void Write_Data(u8 dat)
{
	OLED_DC_H;
	OLED_WrByte(dat);
}

void Write_Command(u8 cmd)
{
	OLED_DC_L;
	//OLED_NSS_L;
	OLED_WrByte(cmd);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Initialization
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void OLED_Init(void)
{

	OLED_RST_L;
	delay_us(100);
	OLED_RST_H;

	Set_Display_On_Off(0x00);		// Display Off (0x00/0x01)
	Set_Display_Clock(0x91);		// Set Clock as 135 Frames/Sec
	Set_Multiplex_Ratio(0x3F);		// 1/64 Duty (0x0F~0x5F)
	Set_Display_Offset(0x4C);		// Shift Mapping RAM Counter (0x00~0x5F)
	Set_Start_Line(0x00);			// Set Mapping RAM Display Start Line (0x00~0x5F)
	Set_Master_Config(0x00);		// Disable Embedded DC/DC Converter (0x00/0x01)
	Set_Remap_Format(0x50);			// Set Column Address 0 Mapped to SEG0
						//     Disable Nibble Remap
						//     Horizontal Address Increment
						//     Scan from COM[N-1] to COM0
						//     Enable COM Split Odd Even
	Set_Current_Range(0x02);		// Set Full Current Range
	Set_Gray_Scale_Table();			// Set Pulse Width for Gray Scale Table
	Set_Contrast_Current(Brightness);	// Set Scale Factor of Segment Output Current Control
	Set_Frame_Frequency(0x51);		// Set Frame Frequency
	Set_Phase_Length(0x55);			// Set Phase 1 as 5 Clocks & Phase 2 as 5 Clocks
	Set_Precharge_Voltage(0x10);		// Set Pre-Charge Voltage Level
	Set_Precharge_Compensation(0x20,0x02);	// Set Pre-Charge Compensation
	Set_VCOMH(0x1C);			// Set High Voltage Level of COM Pin
	Set_VSL(0x0D);				// Set Low Voltage Level of SEG Pin
	Set_Display_Mode(0x00);			// Normal Display Mode (0x00/0x01/0x02/0x03)

	Fill_RAM(0x00);				// Clear Screen

	Set_Display_On_Off(0x01);		// Display On (0x00/0x01)
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Instruction Setting
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Column_Address(unsigned char a, unsigned char b)
{
	Write_Command(0x15);			// Set Column Address
	Write_Command(a);			//   Default => 0x00
	Write_Command(b);			//   Default => 0x3F (Total Columns Devided by 2)
}


void Set_Row_Address(unsigned char a, unsigned char b)
{
	Write_Command(0x75);			// Set Row Address
	Write_Command(a);			//   Default => 0x00
	Write_Command(b);			//   Default => 0x4F
}


void Set_Contrast_Current(unsigned char d)
{
	Write_Command(0x81);			// Set Contrast Value
	Write_Command(d);			//   Default => 0x40
}


void Set_Current_Range(unsigned char d)
{
	Write_Command(0x84|d);			// Set Current Range
						//   Default => 0x84
						//     0x84 (0x00) => Quarter Current Range
						//     0x85 (0x01) => Half Current Range
						//     0x86 (0x02) => Full Current Range
}


void Set_Remap_Format(unsigned char d)
{
	Write_Command(0xA0);			// Set Re-Map & Data Format
	Write_Command(d);			//   Default => 0x00
						//     Column Address 0 Mapped to SEG0
						//     Disable Nibble Re-Map
						//     Horizontal Address Increment
						//     Scan from COM0 to COM[N-1]
						//     Disable COM Split Odd Even
}


void Set_Start_Line(unsigned char d)
{
	Write_Command(0xA1);			// Set Display Start Line
	Write_Command(d);			//   Default => 0x00
}


void Set_Display_Offset(unsigned char d)
{
	Write_Command(0xA2);			// Set Display Offset
	Write_Command(d);			//   Default => 0x00
}


void Set_Display_Mode(unsigned char d)
{
	Write_Command(0xA4|d);			// Set Display Mode
						//   Default => 0xA4
						//     0xA4 (0x00) => Normal Display
						//     0xA5 (0x01) => Entire Display On, All Pixels Turn On at GS Level 15
						//     0xA6 (0x02) => Entire Display Off, All Pixels Turn Off
						//     0xA7 (0x03) => Inverse Display
}


void Set_Multiplex_Ratio(unsigned char d)
{
	Write_Command(0xA8);			// Set Multiplex Ratio
	Write_Command(d);			//   Default => 0x5F
}


void Set_Master_Config(unsigned char d)
{
	Write_Command(0xAD);			// Set Master Configuration
	Write_Command(0x02|d);			//   Default => 0x03
						//     0x02 (0x00) => Select External VCC Supply
						//     0x03 (0x01) => Select Internal DC/DC Voltage Converter
}


void Set_Display_On_Off(unsigned char d)
{
	Write_Command(0xAE|d);			// Set Display On/Off
						//   Default => 0xAE
						//     0xAE (0x00) => Display Off
						//     0xAF (0x01) => Display On
}


void Set_Phase_Length(unsigned char d)
{
	Write_Command(0xB1);			// Phase 1 & 2 Period Adjustment
	Write_Command(d);			//   Default => 0x53 (5 Display Clocks [Phase 2] / 3 Display Clocks [Phase 1])
						//     D[3:0] => Phase 1 Period in 1~15 Display Clocks
						//     D[7:4] => Phase 2 Period in 1~15 Display Clocks
}


void Set_Frame_Frequency(unsigned char d)
{
	Write_Command(0xB2);			// Set Frame Frequency (Row Period)
	Write_Command(d);			//   Default => 0x25 (37 Display Clocks)
}


void Set_Display_Clock(unsigned char d)
{
	Write_Command(0xB3);			// Display Clock Divider/Osciallator Frequency
	Write_Command(d);			//   Default => 0x41
						//     D[3:0] => Display Clock Divider
						//     D[7:4] => Oscillator Frequency
}


void Set_Precharge_Compensation(unsigned char a, unsigned char b)
{
	Write_Command(0xB4);			// Set Pre-Charge Compensation Level
	Write_Command(b);			//   Default => 0x00 (No Compensation)

	if(a == 0x20)
	{
		Write_Command(0xB0);		// Set Pre-Charge Compensation Enable
		Write_Command(0x08|a);		//   Default => 0x08
						//     0x08 (0x00) => Disable Pre-Charge Compensation
						//     0x28 (0x20) => Enable Pre-Charge Compensation
	}
}


void Set_Precharge_Voltage(unsigned char d)
{
	Write_Command(0xBC);			// Set Pre-Charge Voltage Level
	Write_Command(d);			//   Default => 0x10 (Connect to VCOMH)
}


void Set_VCOMH(unsigned char d)
{
	Write_Command(0xBE);			// Set Output Level High Voltage for COM Signal
	Write_Command(d);			//   Default => 0x1D (0.81*VREF)
}


void Set_VSL(unsigned char d)
{
	Write_Command(0xBF);			// Set Segment Low Voltage Level
	Write_Command(0x02|d);			//   Default => 0x0E
						//     0x02 (0x00) => Keep VSL Pin Floating
						//     0x0E (0x0C) => Connect a Capacitor between VSL Pin & VSS
}


void Set_NOP(void)
{
	Write_Command(0xE3);			// Command for No Operation
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Graphic Acceleration
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void GA_Option(unsigned char d)
{
	Write_Command(0x23);			// Graphic Acceleration Command Options
	Write_Command(d);			//   Default => 0x01
						//     Enable Fill Rectangle
						//     Disable Wrap around in Horizontal Direction During Copying & Scrolling
						//     Disable Reverse Copy
}


void Draw_Rectangle(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e)
{
	Write_Command(0x24);			// Draw Rectangle
	Write_Command(a);			//   Column Address of Start
	Write_Command(c);			//   Row Address of Start
	Write_Command(b);			//   Column Address of End (Total Columns Devided by 2)
	Write_Command(d);			//   Row Address of End
	Write_Command(e);			//   Gray Scale Level
	delay_us(200);
}


void Copy(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f)
{
	Write_Command(0x25);			// Copy
	Write_Command(a);			//   Column Address of Start
	Write_Command(c);			//   Row Address of Start
	Write_Command(b);			//   Column Address of End (Total Columns Devided by 2)
	Write_Command(d);			//   Row Address of End
	Write_Command(e);			//   Column Address of New Start
	Write_Command(f);			//   Row Address of New Start
	delay_us(200);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//
//    a: Two Pixels Data
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_RAM(unsigned char a)
{
	GA_Option(0x01);
	Draw_Rectangle(0x00,0x3F,0x00,0x5F,a);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End (Total Columns Devided by 2)
//    c: Row Address of Start
//    d: Row Address of End
//    e: Two Pixels Data
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_Block(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e)
{
	GA_Option(0x01);
	Draw_Rectangle(a,b,c,d,e);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Checkboard (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Checkerboard()
{
unsigned char i,j;
	
	Set_Column_Address(0x00,0x3F);
	Set_Row_Address(0x00,0x5F);

	for(i=0;i<40;i++)
	{
		for(j=0;j<64;j++)
		{
			Write_Data(0xF0);
		}
		for(j=0;j<64;j++)
		{
			Write_Data(0x0F);
		}
	}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Gray Scale Bar (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Grayscale()
{
	//   Level 16 => Column 1~8
		Fill_Block(0x00,0x03,0x00,0x3F,0xFF);

	//   Level 15 => Column 9~16
		Fill_Block(0x04,0x07,0x00,0x3F,0xEE);

	//   Level 14 => Column 17~24
		Fill_Block(0x08,0x0B,0x00,0x3F,0xDD);

	//   Level 13 => Column 25~32
		Fill_Block(0x0C,0x0F,0x00,0x3F,0xCC);

	//   Level 12 => Column 33~40
		Fill_Block(0x10,0x13,0x00,0x3F,0xBB);

	//   Level 11 => Column 41~48
		Fill_Block(0x14,0x17,0x00,0x3F,0xAA);

	//   Level 10 => Column 49~56
		Fill_Block(0x18,0x1B,0x00,0x3F,0x99);

	//   Level 9 => Column 57~64
		Fill_Block(0x1C,0x1F,0x00,0x3F,0x88);

	//   Level 8 => Column 65~72
		Fill_Block(0x20,0x23,0x00,0x3F,0x77);

	//   Level 7 => Column 73~80
		Fill_Block(0x24,0x27,0x00,0x3F,0x66);

	//   Level 6 => Column 81~88
		Fill_Block(0x28,0x2B,0x00,0x3F,0x55);

	//   Level 5 => Column 89~96
		Fill_Block(0x2C,0x2F,0x00,0x3F,0x44);

	//   Level 4 => Column 97~104
		Fill_Block(0x30,0x33,0x00,0x3F,0x33);

	//   Level 3 => Column 105~112
		Fill_Block(0x34,0x37,0x00,0x3F,0x22);

	//   Level 2 => Column 113~120
		Fill_Block(0x38,0x3B,0x00,0x3F,0x11);

	//   Level 1 => Column 121~128
		Fill_Block(0x3C,0x3F,0x00,0x3F,0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Frame (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Frame()
{
	GA_Option(0x00);
	Draw_Rectangle(0x00,0x3F,0x00,0x3F,0xFF);
	Fill_Block(0x00,0x00,0x01,0x3E,0x0F);
	Fill_Block(0x3F,0x3F,0x01,0x3E,0xF0);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End (Total Columns Devided by 2)
//    c: Row Address of Start
//    d: Row Address of End
//    k:0,?????,1????
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d,unsigned char k)
{
unsigned char *Src_Pointer;
unsigned char i,j;
	
	Src_Pointer=Data_Pointer;
	Set_Column_Address(a,b);
	Set_Row_Address(c,d);

	switch(k)
       {
		case 0x00:
			for(i=0;i<(d-c+1);i++)
			{
				for(j=0;j<(b-a+1);j++)
				{
					Write_Data(*Src_Pointer);
					Src_Pointer++;
				}
			}
			break;
		case 0x01:
			for(i=0;i<(d-c+1);i++)
			{
				for(j=0;j<(b-a+1);j++)
				{
					Write_Data(~(*Src_Pointer));
					Src_Pointer++;
				}
			}
			break;
	}

}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Vertical Scrolling (Full Screen)
//
//    a: Scrolling Direction
//       "0x00" (Upward)
//       "0x01" (Downward)
//    b: Set Numbers of Row Scroll per Step
//    c: Set Time Interval between Each Scroll Step
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Vertical_Scroll(unsigned char a, unsigned char b, unsigned char c)
{
unsigned int i,j;	

	switch(a)
	{
		case 0:
			for(i=0;i<80;i+=b)
			{
				Set_Start_Line(i);
				for(j=0;j<c;j++)
				{
					delay_us(200);
				}
			}
			break;
		case 1:
			for(i=0;i<80;i+=b)
			{
				Set_Start_Line(80-i);
				for(j=0;j<c;j++)
				{
					delay_us(200);
				}
			}
			break;
	}
	Set_Start_Line(0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Continuous Horizontal Scrolling (Partial or Full Screen)
//
//    a: Set Numbers of Column Scroll per Step
//    b: Set Numbers of Row to Be Scrolled
//    c: Set Time Interval between Each Scroll Step in Terms of Frame Frequency
//    d: Delay Time
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Horizontal_Scroll(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
	GA_Option(0x03);
	Write_Command(0x26);			// Horizontal Scroll Setup
	Write_Command(a);
	Write_Command(b);
	Write_Command(c);
	Write_Command(0x2F);			// Activate Scrolling
	delay_us(d);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Continuous Horizontal Fade Scrolling (Partial or Full Screen)
//
//    a: Set Numbers of Column Scroll per Step
//    b: Set Numbers of Row to Be Scrolled
//    c: Set Time Interval between Each Scroll Step in Terms of Frame Frequency
//    d: Delay Time
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_Scroll(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
	GA_Option(0x01);
	Write_Command(0x26);			// Horizontal Scroll Setup
	Write_Command(a);
	Write_Command(b);
	Write_Command(c);
	Write_Command(0x2F);			// Activate Scrolling
	delay_us(d);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Deactivate Scrolling (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Deactivate_Scroll()
{
	Write_Command(0x2E);			// Deactivate Scrolling
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade In (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_In()
{
unsigned int i;	

	Set_Display_On_Off(0x01);
	for(i=0;i<(Brightness+1);i++)
	{
		Set_Contrast_Current(i);
		delay_us(200);
		delay_us(200);
		delay_us(200);
	}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade Out (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_Out()
{
unsigned int i;	

	for(i=(Brightness+1);i>0;i--)
	{
		Set_Contrast_Current(i-1);
		delay_us(200);
		delay_us(200);
		delay_us(200);
	}
	Set_Display_On_Off(0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Sleep Mode
//
//    "0x01" Enter Sleep Mode
//    "0x00" Exit Sleep Mode
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Sleep(unsigned char a)
{
	switch(a)
	{
		case 0:
			Set_Display_On_Off(0x00);
			Set_Display_Mode(0x01);
			break;
		case 1:
			Set_Display_Mode(0x00);
			Set_Display_On_Off(0x01);
			break;
	}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Gray Scale Table Setting (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Gray_Scale_Table()
{
	Write_Command(0xB8);			// Set Gray Scale Table
	Write_Command(0x01);			//   Gray Scale Level 1
	Write_Command(0x11);			//   Gray Scale Level 3 & 2
	Write_Command(0x22);			//   Gray Scale Level 5 & 4
	Write_Command(0x32);			//   Gray Scale Level 7 & 6
	Write_Command(0x43);			//   Gray Scale Level 9 & 8
	Write_Command(0x54);			//   Gray Scale Level 11 & 10
	Write_Command(0x65);			//   Gray Scale Level 13 & 12
	Write_Command(0x76);			//   Gray Scale Level 15 & 14
}



/**************************************
  ??????:?2???1???????
  uchar DATA:????????
****************************************/
void con_4_byte(unsigned char DATA)
{
   unsigned char data_4byte[4];
   unsigned char i;
   unsigned char d,k;
   d=DATA;
 
  for(i=0;i<4;i++)   // ????????  2*4=8?
   {
     k=d&0xc0;     //?i=0? ?D7,D6? ?i=1? ?D5,D4?

     /****?4???,16???,????????????,???????????4?***/

     switch(k)
       {
	 case 0x00:
           data_4byte[i]=0x00;
		   delay_us(20);
         break;
     case 0x40:  // 0100,0000
           data_4byte[i]=0x0f;
		   delay_us(20);
         break;	
	 case 0x80:  //1000,0000
           data_4byte[i]=0xf0;
		   delay_us(20);
         break;
     case 0xc0:   //1100,0000
           data_4byte[i]=0xff;
		   delay_us(20);
         break;	 
     default:
      	 break;
	   }
      d=d<<2;                                //????
      
	  
	  Write_Data(data_4byte[i]);                /* 8 column  a nibble of command is a dot*/
   }

}

void con_8_byte(unsigned char DATA)
{
   unsigned char data_4byte[8];
   unsigned char i;
   unsigned char d,k;
   d=DATA;
 
  for(i=0;i<8;i++)   // ????????  2*4=8?
   {
     k=d&0xc0;     //?i=0? ?D7,D6? ?i=1? ?D5,D4?

     /****?4???,16???,????????????,???????????4?***/

     switch(k)
       {
	 case 0x00:
           data_4byte[i]=0x00;
		   delay_us(20);
         break;
     case 0x40:  // 0100,0000
           data_4byte[i]=0x0f;
		   delay_us(20);
         break;	
	 case 0x80:  //1000,0000
           data_4byte[i]=0xf0;
		   delay_us(20);
         break;
     case 0xc0:   //1100,0000
           data_4byte[i]=0xff;
		   delay_us(20);
         break;	 
     default:
      	 break;
	   }
      d=d<<1;                                //????
      
	  
	  Write_Data(data_4byte[i]);                /* 8 column  a nibble of command is a dot*/
   }

}


void Show_dot(unsigned char x, unsigned char y, unsigned char color)
{
    Draw_Rectangle(x,x,y,y,color);
}

void Next_Frame(void)
{
	u8 i,j,temp;

	Set_Remap_Format(0x50);
	Set_Column_Address(0x00, 0x3F);
	Set_Row_Address(0x00, 0x7F);
	
	for(i=0;i<64;i++)
		for(j=0;j<128;j+=2)
		{
			temp = 0x00;
			temp |= OLED_PIXEL[0][i][j]>>4|(OLED_PIXEL[0][i][j+1]>>4<<4);
			//if(OLED_PIXEL[1][i][j]) 	temp |= OLED_PIXEL[1][i][j]>>4;
			//if(OLED_PIXEL[1][i][j+1]) temp |= OLED_PIXEL[1][i][j+1]>>4<<4;
			if(OLED_PIXEL[1][i][j])		temp = temp&0xF0|OLED_PIXEL[1][i][j]>>4;
			if(OLED_PIXEL[1][i][j+1]) temp = temp&0x0F|OLED_PIXEL[1][i][j+1]>>4<<4;
			Write_Data(temp);
			//Write_Data(OLED_PIXEL[i][j+1]<<4);
			//Write_Data(OLED_PIXEL[i][j]>>4|OLED_PIXEL[i][j+1]>>4<<4);
			//Write_Data(OLED_PIXEL[i][j]>>4|OLED_PIXEL[i][j+1]>>4<<4);
		}
}

void Show_XGS(void)
{
	u8 i,j;
	
	for(i=0;i<64;i++)
		for(j=0;j<128;j++)
			OLED_PIXEL[0][i][j]=(i+j)*0xFF/192;
}

void Draw_Dot(u8 x, u8 y, u8 z, u8 color)
{
	OLED_PIXEL[z][x][y] = color;
}

void Clean_Screen(u8 x0, u8 y0, u8 x1, u8 y1, u8 z)
{
	u8 i,j;
	
	for(i=x0;i<x1;i++)
		for(j=y0;j<y1;j++)
			OLED_PIXEL[z][i][j] = 0x00;
}

void Draw_Char(u8 x,u8 y,u8 z,u8 chr,u8 font_color,u8 back_color)
{
	u8 temp,t,t1;
	u8 y0=y;
	chr=chr-' ';
	for(t=0;t<6;t++)
	{
		temp=F6x8[chr][t];
		//if(size==12)temp=oled_asc2_1206[chr][t];
		//else temp=oled_asc2_1608[chr][t];
		y=y0+8;
		for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_PIXEL[z][y][x] = font_color;
			else OLED_PIXEL[z][y][x] = back_color;
			temp<<=1;
			y--;
			if(y==y0)
			{
				y=y0+8;
				x++;
				break;
			}
		}
	}         
}

void Draw_5x7Char(u8 x,u8 y,u8 z,u8 chr,u8 font_color,u8 back_color)
{
	u8 temp,t,t1;
	chr=chr-' ';
	for(t=0;t<5;t++)
	{
		temp=F5x7[chr][t]<<1;
		//if(size==12)temp=oled_asc2_1206[chr][t];
		//else temp=oled_asc2_1608[chr][t];
		for(t1=0;t1<7;t1++)
		{
			if(temp&0x80)OLED_PIXEL[z][x+(7-t1)][y+t] = font_color;
			else OLED_PIXEL[z][x+(7-t1)][y+t] = back_color;
			temp<<=1;
		}
	}         
}

void Draw_4x6Char(u8 x,u8 y,u8 z,u8 chr,u8 font_color,u8 back_color)
{
	u8 temp,t,t1;
	chr=chr-' ';
	if(y+4>128)return;
	if(x+6>64)return;
	for(t=0;t<6;t++)
	{
		temp=F4x6[chr][t/2]<<((t%2)*4);
		//if(size==12)temp=oled_asc2_1206[chr][t];
		//else temp=oled_asc2_1608[chr][t];
		for(t1=0;t1<4;t1++)
		{
			if(temp&0x80)OLED_PIXEL[z][x+t][y+t1] = font_color;
			else OLED_PIXEL[z][x+t][y+t1] = back_color;
			temp<<=1;
		}
	}         
}

void Draw_Icon(u8 x,u8 y,u8 z,u8 chr,u8 font_color,u8 back_color)
{
	u8 temp,t,t1;
	for(t=0;t<5;t++)
	{
		temp=Icon[chr][t];
		for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_PIXEL[z][x+t][y+t1] = font_color;
			else OLED_PIXEL[z][x+t][y+t1] = back_color;
			temp<<=1;
		}
	}         
}

void Draw_4x6String(u8 x,u8 y,u8 z,u8 chr[],u8 font_color,u8 back_color)
{
	u8 j;

    for (j = 0; chr[j] != '\0' && y+j*4<128; j++)
    {    
        Draw_4x6Char(x,y+j*4,z,chr[j],font_color,back_color);
    }     
}

void Draw_5x7String(u8 x,u8 y,u8 z,u8 chr[],u8 font_color,u8 back_color)
{
	u8 j;

    for (j = 0; chr[j] != '\0'; j++)
    {    
        Draw_5x7Char(x,y+j*6,z,chr[j],font_color,back_color);
    }     
}

void Draw_String(u8 x,u8 y,u8 z,u8 chr[],u8 font_color,u8 back_color)
{
	u8 j;

    for (j = 0; chr[j] != '\0' && x+j*6<128; j++)
    {
			Draw_Char(x+j*6,y,z,chr[j],font_color,back_color);
    }     
}

void Draw_Notification(u8 title[], u8 chr[])
{
	u8 i,j, x, y;
	
	Clean_Screen(0, 0, 64, 128, 1);
	
	for (j=8; j<=119; j++)
		for(i=8;i<=55;i++)
		OLED_PIXEL[1][i][j] = 0x01;
	
	for (j=8; j<=119; j++)
	{    
		OLED_PIXEL[1][7][j] = 0xFF;
		OLED_PIXEL[1][56][j] = 0xFF;
		OLED_PIXEL[1][19][j] = 0xFF;
		OLED_PIXEL[1][57][j] = 0x80;
	} 
	
	for (j=8; j<=55; j++)
	{    
		OLED_PIXEL[1][j][7] = 0xFF;
		OLED_PIXEL[1][j][120] = 0xFF;
	} 
	
	OLED_PIXEL[1][56][7] = 0x80;
	OLED_PIXEL[1][56][120] = 0x80;

	Draw_String(11,9,1,title,0xFF,0x01);
	
	x = 0; y = 0;
	for (j = 0; chr[j] != '\0'; j++)
	{    
		switch(chr[j])
		{
			case '|':
				y++;
				x = 0;
				break;
			default:
				Draw_Char(11+x*6,22+y*8,1,chr[j],0xFF,0x01);
				x++;
		}
	}   
	
	Next_Frame();
	
	while(!Keyboard_Now) Update_Keyboard();
	while(Keyboard_Now) Update_Keyboard();
	
	Clean_Screen(0, 0, 64, 128, 1);
	Next_Frame();
}

void Draw_Reverse(u8 x0, u8 y0, u8 x1, u8 y1, u8 z)
{
	u8 i, j;
	for(i=x0;i<=x1;i++)
		for(j=y0;j<=y1;j++)
			OLED_PIXEL[z][i][j] = ~OLED_PIXEL[z][i][j];
}


u8 Draw_Menu(u8 menu[][12], u8 x0, u8 y0, u8 x1, u8 y1, u8 default_select)
{
	u8 n = menu[0][0], dx;
	u8 temp,t,t1;
	u8 i, j, x = 0, y = 0, select = 1, startline = 0;

	dx = x1-x0;

	if(default_select>0)select=default_select;

	Clean_Screen(0, 0, 64, 128, 1);

	while(Keyboard_Now != 'e' && Keyboard_Now != 's')
	{
		u8 temp_x0;
		
		if((select-1)*8<startline)startline=(select-1)*8;
		if(select*8>startline+dx)startline=select*8-dx;
		
		for (j=x0; j<=x1; j++)
			for(i=y0;i<=y1;i++)
				OLED_PIXEL[1][j][i] = 0x01;
		
		x=y0+1;
		for (j = 0; menu[startline/8+1][j] != '\0'; j++)
			for(t=0;t<6;t++)
			{
				temp=F6x8[menu[startline/8+1][j]-' '][t];
				temp>>=startline%8;
				temp<<=startline%8;
				y=x0+8-startline%8;
				for(t1=0;t1<8-startline%8;t1++)
				{
					if(temp&0x80 && t1<8-startline%8){
						OLED_PIXEL[1][y][x] = 0xFE;
					}else 
						OLED_PIXEL[1][y][x] = 0x01;
					temp<<=1;
					y--;
					if(y==x0)
					{
						//y=i*8;
						x++;
						break;
					}
				}
			}   
	
		x=0;
		for(i=startline/8+1;i<(dx-8+startline%8)/8+startline/8+2;i++)
		{
			x++;
			Draw_String(y0+1,x0+x*8-startline%8,1,menu[i+1],0xFE,0x01);
		}

		temp_x0 = (select-(startline/8)-1)*8+(8-startline%8)+x0-7;

		Draw_Reverse(temp_x0, y0+1, temp_x0+7, y1-1, 1);
		
		for (j=x1+2; j<64; j++)
			for(i=y0;i<=y1;i++)
				OLED_PIXEL[1][j][i] = 0x00;

		for (j=y0+1; j<=y1-1; j++)
		{    
			OLED_PIXEL[1][x0][j] = 0xFF;
			OLED_PIXEL[1][x1][j] = 0xFF;
			OLED_PIXEL[1][x1+1][j] = 0x80;
		} 
		
		for (j=x0+1; j<=x1-1; j++)
		{    
			OLED_PIXEL[1][j][y0] = 0xFF;
			OLED_PIXEL[1][j][y1] = 0xFF;
		} 
		
		OLED_PIXEL[1][x1][y0] = 0x80;
		OLED_PIXEL[1][x1][y1] = 0x80;

		Update_Keyboard();
		Next_Frame();
		
		if(Keyboard_Now == 'u' && select > 1){
			select --;
			while(Keyboard_Now == 'u')Update_Keyboard();
		}
		if(Keyboard_Now == 'd' && select < n){
			select ++;
			while(Keyboard_Now == 'd')Update_Keyboard();
		}
		
	}
	if(Keyboard_Now == 's')select = default_select;
	while(Keyboard_Now == 'e' || Keyboard_Now == 's') Update_Keyboard();
	//Clean_Screen(0, 0, 64, 128, 1);
	return select;
}

void Draw_Component(u8 type, u8 index, u8 x0, u8 y0, u8 x1, u8 y1, u8 z, u8 c0, u8 c1, u8 chr[])
{
	u8 j;
	
	OLED_COMPONENT_LIST[index].type = type;
	OLED_COMPONENT_LIST[index].x0 = x0;
	OLED_COMPONENT_LIST[index].x1 = x1;
	OLED_COMPONENT_LIST[index].y0 = y0;
	OLED_COMPONENT_LIST[index].y1 = y1;
	OLED_COMPONENT_LIST[index].c0 = c0;
	OLED_COMPONENT_LIST[index].c1 = c1;
	OLED_COMPONENT_LIST[index].z = z;
	for(j=0;j<32;j++)
		OLED_COMPONENT_LIST[index].chr[j] = 0x00;
	for(j=0;chr[j]!='\0';j++)
		OLED_COMPONENT_LIST[index].chr[j] = chr[j];

}

void Draw_Logo(u8 z, u8 chr[])
{
	int i0 = 0, j0 = 0, x = 0, y = 0;
	u8 temp;

	for(i0=0;i0<1024;i0++)
	{
		temp = gImage_logo_0[i0];
		for(j0=0;j0<8;j0++)
		{
			if(temp&0x80)OLED_PIXEL[z][x][y] = 0xFF;
			else OLED_PIXEL[z][x][y] = 0x00;
			temp<<=1;
			y++;
			if(y==128){
				x++;
				y=0;
			}
		}
	}
}

void Update_Component(u8 select_index)
{
	u8 i,j,t,main_i;
	u8 str_temp[10];
	struct OLED_COMPONENT temp;
	
	Clean_Screen(0, 0, 64, 128, 0);
	Clean_Screen(0, 0, 64, 128, 1);
	
	for(main_i=0;main_i<COMPONENT_MAX_INDEX;main_i++)
	{
		temp = OLED_COMPONENT_LIST[main_i];
		switch(temp.type)
		{
			case 0x01:
				if(temp.x1==4)
					Draw_4x6String(temp.x0, temp.y0, temp.z, temp.chr, 0xFF, 0x01);
				else
					Draw_String(temp.y0, temp.x0, temp.z, temp.chr, 0xFF, 0x01);
				break;
			case 0x02:
				if(temp.x1==4)
					Draw_4x6String(temp.x0, temp.y0, temp.z, temp.chr, 0x80, 0x01);
				else
					Draw_String(temp.y0, temp.x0, temp.z, temp.chr, 0x80, 0x01);
				break;
			case 0x04:
				for (j=temp.x0+1; j<=temp.x1-1; j++)
					for (i=temp.y0+1; i<=temp.y1-1; i++)
						OLED_PIXEL[temp.z][j][i] = 0x01;
			
				for (j=temp.y0+1; j<=temp.y1-1; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0][j] = 0xFE;
					OLED_PIXEL[temp.z][temp.x1][j] = 0xFE;
					OLED_PIXEL[temp.z][temp.x1+1][j] = 0x80;
				} 
				for (j=temp.x0+1; j<=temp.x1-1; j++)
				{    
					OLED_PIXEL[temp.z][j][temp.y0] = 0xFE;
					OLED_PIXEL[temp.z][j][temp.y1] = 0xFE;
				} 
				OLED_PIXEL[temp.z][temp.x1][temp.y0] = 0x80;
				OLED_PIXEL[temp.z][temp.x1][temp.y1] = 0x80;
				for(j=0;temp.chr[j]!='\0';j++);
				Draw_String(temp.y0+(temp.y1-temp.y0-j*6)/2, temp.x0+(temp.x1-temp.x0-8)/2, temp.z, temp.chr, 0xFE, 0x01);
				
				if(select_index == main_i) Draw_Reverse(temp.x0+1, temp.y0+1, temp.x1-1, temp.y1-1, temp.z);
				break;
			case 0x05:
				for (j=temp.x0+1; j<=temp.x1-1; j++)
					for (i=temp.y0+1; i<=temp.y1-1; i++)
						OLED_PIXEL[temp.z][j][i] = 0x01;
			
				for (j=temp.y0+1; j<=temp.y1-1; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0][j] = 0xA0;
					OLED_PIXEL[temp.z][temp.x1][j] = 0xA0;
				} 
				for (j=temp.x0+1; j<=temp.x1-1; j++)
				{    
					OLED_PIXEL[temp.z][j][temp.y0] = 0xA0;
					OLED_PIXEL[temp.z][j][temp.y1] = 0xA0;
				}

				for(j=0;temp.chr[j]!='\0';j++);
				Draw_String(temp.y0+(temp.y1-temp.y0-j*6)/2, temp.x0+(temp.x1-temp.x0-8)/2, temp.z, temp.chr, 0xA0, 0x01);
				break;
				
			case COMPONENT_RADIO_TRUE:
				
				for(t=0;temp.chr[t]!='\0';t++);
				for (j=temp.x0; j<=temp.x0+7; j++)
					for (i=temp.y0; i<=temp.y0+9+t*6; i++)
						OLED_PIXEL[temp.z][j][i] = 0x00;
				
				for (j=0; j<4; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0+1][temp.y0+2+j] = 0xF0;
					OLED_PIXEL[temp.z][temp.x0+6][temp.y0+2+j] = 0xF0;
					OLED_PIXEL[temp.z][temp.x0+2+j][temp.y0+1] = 0xF0;
					OLED_PIXEL[temp.z][temp.x0+2+j][temp.y0+6] = 0xF0;
				} 
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+3] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+3] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+4] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+4] = 0xF0;
				
				Draw_String(temp.y0+8, temp.x0, temp.z, temp.chr, 0xFF, 0x00);
				
				if(select_index == main_i){
					Draw_Reverse(temp.x0, temp.y0, temp.x0+7, temp.y0+7+t*6+2, temp.z);
				}
				break;
				
			case COMPONENT_RADIO_FALSE:
				
				for(t=0;temp.chr[t]!='\0';t++);
				for (j=temp.x0; j<=temp.x0+7; j++)
					for (i=temp.y0; i<=temp.y0+9+t*6; i++)
						OLED_PIXEL[temp.z][j][i] = 0x00;
			
				for (j=0; j<4; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0+1][temp.y0+2+j] = 0xF0;
					OLED_PIXEL[temp.z][temp.x0+6][temp.y0+2+j] = 0xF0;
					OLED_PIXEL[temp.z][temp.x0+2+j][temp.y0+1] = 0xF0;
					OLED_PIXEL[temp.z][temp.x0+2+j][temp.y0+6] = 0xF0;
				} 
				Draw_String(temp.y0+8, temp.x0, temp.z, temp.chr, 0xFF, 0x00);
				
				if(select_index == main_i){
					Draw_Reverse(temp.x0, temp.y0, temp.x0+7, temp.y0+7+t*6+2, temp.z);
				}
				break;
				
			case COMPONENT_RADIO_DISABLED:
				
				for (j=0; j<4; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0+1][temp.y0+2+j] = 0xA0;
					OLED_PIXEL[temp.z][temp.x0+6][temp.y0+2+j] = 0xA0;
					OLED_PIXEL[temp.z][temp.x0+2+j][temp.y0+1] = 0xA0;
					OLED_PIXEL[temp.z][temp.x0+2+j][temp.y0+6] = 0xA0;
					OLED_PIXEL[temp.z][temp.x0+2][temp.y0+2+j] = 0x70;
					OLED_PIXEL[temp.z][temp.x0+3][temp.y0+2+j] = 0x70;
					OLED_PIXEL[temp.z][temp.x0+4][temp.y0+2+j] = 0x70;
					OLED_PIXEL[temp.z][temp.x0+5][temp.y0+2+j] = 0x70;
				}
				Draw_String(temp.y0+8, temp.x0, temp.z, temp.chr, 0xA0, 0x00);
				break;
				
			case COMPONENT_TEXT_ENABLED:
				
				for (j=temp.x0; j<=temp.x1; j++)
					for (i=temp.y0; i<=temp.y1; i++)
						OLED_PIXEL[temp.z][j][i] = 0x01;
			
				for (j=temp.y0; j<=temp.y1; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0][j] = 0xFF;
					OLED_PIXEL[temp.z][temp.x1][j] = 0xFF;
				} 
				for (j=temp.x0; j<=temp.x1; j++)
				{    
					OLED_PIXEL[temp.z][j][temp.y0] = 0xFF;
					OLED_PIXEL[temp.z][j][temp.y1] = 0xFF;
				} 

				for(j=0;temp.chr[j]!='\0';j++);
				if(temp.c0==4)
					Draw_4x6String(temp.x0+(temp.x1-temp.x0-6)/2+1, temp.y0+(temp.y1-temp.y0-j*4)/2, temp.z, temp.chr, 0xFF, 0x00);
				else
					Draw_String(temp.y0+(temp.y1-temp.y0-j*6)/2, temp.x0+(temp.x1-temp.x0-8)/2, temp.z, temp.chr, 0xFF, 0x00);
				if(select_index == main_i) Draw_Reverse(temp.x0+1, temp.y0+1, temp.x1-1, temp.y1-1, temp.z);
				break;
			case 0x0C:
				for (j=temp.x0; j<=temp.x1; j++)
					for (i=temp.y0; i<=temp.y1; i++)
						OLED_PIXEL[temp.z][j][i] = 0x00;
			
				for (j=temp.y0; j<=temp.y1; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0][j] = 0xB0;
					OLED_PIXEL[temp.z][temp.x1][j] = 0xB0;
				} 
				for (j=temp.x0; j<=temp.x1; j++)
				{    
					OLED_PIXEL[temp.z][j][temp.y0] = 0xB0;
					OLED_PIXEL[temp.z][j][temp.y1] = 0xB0;
				} 

				for(j=0;temp.chr[j]!='\0';j++);
				Draw_String(temp.y0+(temp.y1-temp.y0-j*6)/2, temp.x0+(temp.x1-temp.x0-8)/2, temp.z, temp.chr, 0xB0, 0x00);
				
				if(select_index == main_i) Draw_Reverse(temp.x0+1, temp.y0+1, temp.x1-1, temp.y1-1, temp.z);
				break;
			case 0x0E:
				for (j=temp.x0+1; j<=temp.x1-1; j++)
					for (i=temp.y0+1; i<=temp.y1-1; i++)
						OLED_PIXEL[temp.z][j][i] = 0x00;
			
				for (j=temp.y0+1; j<=temp.y1-1; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0][j] = 0xFF;
					OLED_PIXEL[temp.z][temp.x1][j] = 0xFF;
				} 
				for (j=temp.x0+1; j<=temp.x1-1; j++)
				{    
					OLED_PIXEL[temp.z][j][temp.y0] = 0xFF;
					OLED_PIXEL[temp.z][j][temp.y1] = 0xFF;
				}

				Draw_String(temp.y0+3, temp.x0+(temp.x1-temp.x0-8)/2, temp.z, temp.chr, 0xFF, 0x00);
				Draw_Reverse(temp.x0+1, temp.c0+1, temp.x1-1, temp.c1-1, temp.z);
				break;
			case 0x10:
				for (j=temp.x0+1; j<=temp.x1-1; j++)
					for (i=temp.y0+1; i<=temp.y1-1; i++)
						OLED_PIXEL[temp.z][j][i] = 0x01;
			
				for (j=temp.y0+1; j<=temp.y1-1; j++)
				{    
					OLED_PIXEL[temp.z][temp.x0][j] = 0xFF;
					OLED_PIXEL[temp.z][temp.x0+10][j] = 0xFF;
					OLED_PIXEL[temp.z][temp.x1][j] = 0xFF;
				} 
				for (j=temp.x0+1; j<=temp.x1-1; j++)
				{    
					OLED_PIXEL[temp.z][j][temp.y0] = 0xFF;
					OLED_PIXEL[temp.z][j][temp.y1] = 0xFF;
				}
				
				for(j=0;temp.chr[j]!='\0';j++);
				Draw_4x6String(temp.x0+3, temp.y0+3, temp.z, temp.chr, 0xFF, 0x01);
				break;
			case 0x12:
				
			
				for (j=temp.x0; j<=temp.x1; j++)
					for (i=temp.y0; i<=temp.y1; i++)
						OLED_PIXEL[temp.z][j][i] = 0x01;
				
				for (j=temp.y0+10; j<=temp.y1; j++)
				{    
					OLED_PIXEL[temp.z][temp.x1-8][j] = 0xFF;
				} 
				t=0;
				for (j=temp.x1-8; j>=temp.x0; j--,t++)
				{    
					OLED_PIXEL[temp.z][j][temp.y0+10] = 0xFF;
					if(t%12==0){
						OLED_PIXEL[temp.z][j][temp.y0+11] = 0xFF;
						OLED_PIXEL[temp.z][j][temp.y0+12] = 0xFF;
						for (i=temp.y0+14; i<=temp.y1; i++)
						{    
							if(i%2==0)OLED_PIXEL[temp.z][j][i] = 0x50;
						} 
						sprintf((char*)str_temp, "%d", t/12*20);
						Draw_4x6String(j-2, temp.y0, temp.z, str_temp, 0xFF, 0x01);
					}
					if(t%6==0){
						OLED_PIXEL[temp.z][j][temp.y0+11] = 0xFF;
					}
				}
				/*
				for(j=0;temp.chr[j]!=0x00;j++)
				{
					u8 y;
					y = temp.x1-8-temp.chr[j];
					OLED_PIXEL[temp.z][y][j+temp.y0+11] = 0xFF;
				}
				*/
				for(j=0;j<temp.y1-temp.y0-10;j++)
				{
					u8 y;
					y = temp.x1-8-record_data[j]*0.6;
					if(y<temp.x0)y=temp.x0;
					OLED_PIXEL[temp.z][y][j+temp.y0+11] = 0xFF;
				}
				
				//for(j=0;temp.chr[j]!='\0';j++);

				//sprintf(str_temp, "%d", temp.c0);
				for(j=0;temp.chr[j]!='\0';j++);
				Draw_4x6String(temp.x1-5, temp.y1-4*j+4, temp.z, temp.chr, 0xFF, 0x01);
				
				break;
				
			case COMPONENT_LISTITEM_ENABLED:
				
				Draw_4x6String(temp.x0, 0, temp.z, temp.chr, 0xFF, 0x01);
				if(select_index == main_i) Draw_Reverse(temp.x0-1, 0, temp.x0+5, 127, temp.z);
				break;
			
			case COMPONENT_LISTITEM_DISABLED:
				
				Draw_4x6String(temp.x0, temp.y0, temp.z, temp.chr, 0x80, 0x01);
				break;
			
			case COMPONENT_SRADIO_TRUE:
				
				for(t=0;temp.chr[t]!='\0';t++);
				for (j=temp.x0; j<=temp.x0+6; j++)
					for (i=temp.y0; i<=temp.y0+6+t*4; i++)
						OLED_PIXEL[temp.z][j][i] = 0x00;
				
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+2] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+3] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+4] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+2] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+3] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+4] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+2] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+3] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+4] = 0xF0;
				
				Draw_4x6String(temp.x0+1, temp.y0+6, temp.z, temp.chr, 0xFE, 0x01);
				
				if(select_index == main_i){
					Draw_Reverse(temp.x0, temp.y0, temp.x0+6, temp.y0+6+t*4+2, temp.z);
				}
				break;
				
			case COMPONENT_SRADIO_FALSE:
				
				for(t=0;temp.chr[t]!='\0';t++);
				for (j=temp.x0; j<=temp.x0+6; j++)
					for (i=temp.y0; i<=temp.y0+6+t*4; i++)
						OLED_PIXEL[temp.z][j][i] = 0x00;
				
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+2] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+3] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+4] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+2] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+4] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+2] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+3] = 0xF0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+4] = 0xF0;
				
				Draw_4x6String(temp.x0+1, temp.y0+6, temp.z, temp.chr, 0xFE, 0x01);
				
				if(select_index == main_i){
					Draw_Reverse(temp.x0, temp.y0, temp.x0+6, temp.y0+6+t*4+2, temp.z);
				}
				break;
				
			case COMPONENT_SRADIO_DISABLED:
				
				for(t=0;temp.chr[t]!='\0';t++);
				for (j=temp.x0; j<=temp.x0+6; j++)
					for (i=temp.y0; i<=temp.y0+6+t*4; i++)
						OLED_PIXEL[temp.z][j][i] = 0x00;
				
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+2] = 0xA0;
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+3] = 0xA0;
				OLED_PIXEL[temp.z][temp.x0+2][temp.y0+4] = 0xA0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+2] = 0xA0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+3] = 0xA0;
				OLED_PIXEL[temp.z][temp.x0+3][temp.y0+4] = 0xA0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+2] = 0xA0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+3] = 0xA0;
				OLED_PIXEL[temp.z][temp.x0+4][temp.y0+4] = 0xA0;
				
				Draw_4x6String(temp.x0+1, temp.y0+6, temp.z, temp.chr, 0xA0, 0x01);

				break;
			
			case COMPONENT_BLANK:
				
				for (j=0; j<64; j++)
					for (i=0; i<128; i++)
						OLED_PIXEL[temp.z][j][i] = 0x01;
			
				break;
		}
	}
}

u8 Return_Index_Available(u8 type)
{
	if(type == COMPONENT_BUTTON_ENABLED) return 0;
	if(type == COMPONENT_RADIO_TRUE) return 0;
	if(type == COMPONENT_RADIO_FALSE) return 0;
	if(type == COMPONENT_TEXT_ENABLED) return 0;
	if(type == COMPONENT_LISTITEM_ENABLED) return 0;
	if(type == COMPONENT_SRADIO_TRUE) return 0;
	if(type == COMPONENT_SRADIO_FALSE) return 0;
	return 1;
}

void Change_Index(s8 dx)
{
	int i;
	i=COMPONENT_INDEX+dx;
	while((i<=COMPONENT_MAX_INDEX&&i>=0) && Return_Index_Available(OLED_COMPONENT_LIST[i].type)){
		i+=dx;
	}
	if((i<=COMPONENT_MAX_INDEX&&i>=0) && (!Return_Index_Available(OLED_COMPONENT_LIST[i].type))){
		COMPONENT_INDEX = i;
	}
}

void Click_Component(void)
{
	switch(OLED_COMPONENT_LIST[COMPONENT_INDEX].type)
	{
		case COMPONENT_RADIO_TRUE:
			OLED_COMPONENT_LIST[COMPONENT_INDEX].type = COMPONENT_RADIO_FALSE;
			break;
		case COMPONENT_RADIO_FALSE:
			OLED_COMPONENT_LIST[COMPONENT_INDEX].type = COMPONENT_RADIO_TRUE;
			break;
		case COMPONENT_SRADIO_TRUE:
			OLED_COMPONENT_LIST[COMPONENT_INDEX].type = COMPONENT_SRADIO_FALSE;
			break;
		case COMPONENT_SRADIO_FALSE:
			OLED_COMPONENT_LIST[COMPONENT_INDEX].type = COMPONENT_SRADIO_TRUE;
			break;
	}
}

void Recycle_Component(u8 indexA, u8 indexB)
{
	u8 i;
	for(i=indexA;i<=indexB;i++)
		Draw_Component(COMPONENT_NULL, i, 0, 0, 0, 0, 0, 0, 0, "");
}
