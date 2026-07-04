/********************金逸晨**************************
*****************GMT130 模块 1.3寸   7PIN SPI TFT FOR STM32F103*************
***************************240*240   16/18BIT真彩***********************
*****STM32F103驱动**************************
***** PB5----3  SCK   ,  PB6----4  SDA   ,    PB7---5  RST   ,
      PB8    ---6   DC   ,  PB6----7  BLK
***** BY:GU 
*********ST7789 控制器,本模块第七针BLK为0时，背光关闭，可以不接
由于本模块CS直接接地，默认片选，在所有操作中不应该随意操作SCK 以避免引起错误命令
为顺利完成初始化，在RESET 之前应先将SCK操作为1，以便顺利识别时序中的START，随后每次SCK=1,将完成一位数据发送！！

默认数据格式位RGB565
********************************************************/
#include "spi_lcd.h"
#include "systick.h"
#include "SPI_LCD_code_table.h"

#define     RED          0XF800	  //红色
#define     GREEN        0X07E0	  //绿色
#define     BLUE         0X001F	  //蓝色
#define     WHITE        0XFFFF	  //白色

#define TFT_COLUMN_NUMBER 240
#define TFT_LINE_NUMBER 240
#define TFT_COLUMN_OFFSET 0

#define PIC_NUM 28800			//图片数据大小


/**********SPI引脚分配，连接TFT屏，更具实际情况修改*********/

#define SPI_SCK_0  GPIOB->BSRRL = (1U << 7);       // 设置sck接口到PB5   清零
#define SPI_SCK_1  GPIOB->BSRRH = (1U << 7);       //置位
#define SPI_SDA_0  GPIO_ResetBits(GPIOA, GPIO_Pin_4)       // 设置SDA接口到PB6
#define SPI_SDA_1  GPIO_SetBits(GPIOA, GPIO_Pin_4) 
#define SPI_RST_0  GPIO_ResetBits(GPIOG, GPIO_Pin_15)       // 设置RST接口到PB7
#define SPI_RST_1  GPIO_SetBits(GPIOG, GPIO_Pin_15)
#define SPI_DC_0   GPIO_ResetBits(GPIOC, GPIO_Pin_7)      // 设置DC接口到PB8
#define SPI_DC_1   GPIO_SetBits(GPIOC, GPIO_Pin_7)
#define SPI_BLK_0   GPIO_ResetBits(GPIOC, GPIO_Pin_9)    // 设置/WR 或CS接口到PB9
#define SPI_BLK_1   GPIO_SetBits(GPIOC, GPIO_Pin_9)



const unsigned char  *point;

void IO_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructer;
	
	//1.开启GPIOE F时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOG,ENABLE);
	
	//2.初始化GPIOF9 F10推挽输出 高速 无上下拉
	GPIO_InitStructer.GPIO_Mode = GPIO_Mode_OUT;//输出模式
	GPIO_InitStructer.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructer.GPIO_Speed = GPIO_Speed_100MHz;//输出速度 高
	GPIO_InitStructer.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStructer.GPIO_Pin = GPIO_Pin_4;//PA4
	GPIO_Init(GPIOA,&GPIO_InitStructer);
	
	GPIO_InitStructer.GPIO_Mode = GPIO_Mode_OUT;//输出模式
	GPIO_InitStructer.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructer.GPIO_Speed = GPIO_Speed_100MHz;//输出速度 高
	GPIO_InitStructer.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStructer.GPIO_Pin = GPIO_Pin_7;//PB7
	GPIO_Init(GPIOB,&GPIO_InitStructer);
	
	GPIO_InitStructer.GPIO_Mode = GPIO_Mode_OUT;//输出模式
	GPIO_InitStructer.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructer.GPIO_Speed = GPIO_Speed_100MHz;//输出速度 高
	GPIO_InitStructer.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStructer.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_9;//PC7 PC9
	GPIO_Init(GPIOC,&GPIO_InitStructer);
	
	GPIO_InitStructer.GPIO_Mode = GPIO_Mode_OUT;//输出模式
	GPIO_InitStructer.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructer.GPIO_Speed = GPIO_Speed_100MHz;//输出速度 高
	GPIO_InitStructer.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStructer.GPIO_Pin = GPIO_Pin_15;//PG15
	GPIO_Init(GPIOG,&GPIO_InitStructer);
}

/*************SPI配置函数*******************
SCL空闲时低电平，第一个上升沿采样
模拟SPI
******************************************/

/**************************SPI模块发送函数************************************************

 *************************************************************************/
void SPI_SendByte(unsigned  char byte)				//向液晶屏写一个8位数据
{
  
  unsigned char counter;
 
  for(counter=0;counter<8;counter++)
  { 
    SPI_SCK_0;		  
    if((byte&0x80)==0)
    {
      SPI_SDA_0;
    }
    else SPI_SDA_1;
    byte=byte<<1;	
    SPI_SCK_1;		
  }		
	SPI_SCK_0;
}

void TFT_SEND_CMD(unsigned char o_command)
  {
	SPI_DC_0;
	SPI_SCK_0;	
    SPI_SendByte(o_command);
  }
  //向液晶屏写一个8位数据
void TFT_SEND_DATA(unsigned  char o_data)
  { 
    SPI_DC_1;
	SPI_SCK_0;	
    SPI_SendByte(o_data);   
   }
void TFT_clear(void)
  {
    unsigned int ROW,column;
  TFT_SEND_CMD(0x2a);     //Column address set
  TFT_SEND_DATA(0x00);    //start column
  TFT_SEND_DATA(0x00); 
  TFT_SEND_DATA(0x00);    //end column
  TFT_SEND_DATA(0xF0);

  TFT_SEND_CMD(0x2b);     //Row address set
  TFT_SEND_DATA(0x00);    //start row
  TFT_SEND_DATA(0x00); 
  TFT_SEND_DATA(0x00);    //end row
  TFT_SEND_DATA(0xF0);
    TFT_SEND_CMD(0x2C);     //Memory write
    for(ROW=0;ROW<TFT_LINE_NUMBER;ROW++)             //ROW loop
      { 
    
          for(column=0;column<TFT_COLUMN_NUMBER;column++)  //column loop
            {
              
				TFT_SEND_DATA(0xFF);
				TFT_SEND_DATA(0xFF);
            }
      }
  }
void TFT_full(unsigned int color)
  {
    unsigned int ROW,column;
    TFT_SEND_CMD(0x2a);     //Column address set
  TFT_SEND_DATA(0x00);    //start column
  TFT_SEND_DATA(0x00); 
  TFT_SEND_DATA(0x00);    //end column
  TFT_SEND_DATA(0xF0);

  TFT_SEND_CMD(0x2b);     //Row address set
  TFT_SEND_DATA(0x00);    //start row
  TFT_SEND_DATA(0x00); 
  TFT_SEND_DATA(0x00);    //end row
  TFT_SEND_DATA(0xF0);
    TFT_SEND_CMD(0x2C);     //Memory write
    for(ROW=0;ROW<TFT_LINE_NUMBER;ROW++)             //ROW loop
      { 
    
  for(column=0;column<TFT_COLUMN_NUMBER ;column++) //column loop
          {

			TFT_SEND_DATA(color>>8);
			  TFT_SEND_DATA(color);
          }
      }
  }


void TFT_init(void)				////ST7789V2
  {
	SPI_SCK_1;			//特别注意！！
	SPI_RST_0;
	delay_ms(1000);
	SPI_RST_1;
	delay_ms(1000);
    TFT_SEND_CMD(0x11); 			//Sleep Out
	delay_ms(120);               //DELAY120ms 
	 	  //-----------------------ST7789V Frame rate setting-----------------//
//************************************************
                TFT_SEND_CMD(0x3A);        //65k mode
                TFT_SEND_DATA(0x05);
                TFT_SEND_CMD(0xC5); 		//VCOM
                TFT_SEND_DATA(0x1A);
                TFT_SEND_CMD(0x36);                 // 屏幕显示方向设置
                TFT_SEND_DATA(0x00);
                //-------------ST7789V Frame rate setting-----------//
                TFT_SEND_CMD(0xb2);		//Porch Setting
                TFT_SEND_DATA(0x05);
                TFT_SEND_DATA(0x05);
                TFT_SEND_DATA(0x00);
                TFT_SEND_DATA(0x33);
                TFT_SEND_DATA(0x33);

                TFT_SEND_CMD(0xb7);			//Gate Control
                TFT_SEND_DATA(0x05);			//12.2v   -10.43v
                //--------------ST7789V Power setting---------------//
                TFT_SEND_CMD(0xBB);//VCOM
                TFT_SEND_DATA(0x3F);

                TFT_SEND_CMD(0xC0); //Power control
                TFT_SEND_DATA(0x2c);

                TFT_SEND_CMD(0xC2);		//VDV and VRH Command Enable
                TFT_SEND_DATA(0x01);

                TFT_SEND_CMD(0xC3);			//VRH Set
                TFT_SEND_DATA(0x0F);		//4.3+( vcom+vcom offset+vdv)

                TFT_SEND_CMD(0xC4);			//VDV Set
                TFT_SEND_DATA(0x20);				//0v

                TFT_SEND_CMD(0xC6);				//Frame Rate Control in Normal Mode
                TFT_SEND_DATA(0X01);			//111Hz

                TFT_SEND_CMD(0xd0);				//Power Control 1
                TFT_SEND_DATA(0xa4);
                TFT_SEND_DATA(0xa1);

                TFT_SEND_CMD(0xE8);				//Power Control 1
                TFT_SEND_DATA(0x03);

                TFT_SEND_CMD(0xE9);				//Equalize time control
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x08);
                //---------------ST7789V gamma setting-------------//
                TFT_SEND_CMD(0xE0); //Set Gamma
                TFT_SEND_DATA(0xD0);
                TFT_SEND_DATA(0x05);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x08);
                TFT_SEND_DATA(0x14);
                TFT_SEND_DATA(0x28);
                TFT_SEND_DATA(0x33);
                TFT_SEND_DATA(0x3F);
                TFT_SEND_DATA(0x07);
                TFT_SEND_DATA(0x13);
                TFT_SEND_DATA(0x14);
                TFT_SEND_DATA(0x28);
                TFT_SEND_DATA(0x30);
                 
                TFT_SEND_CMD(0XE1); //Set Gamma
                TFT_SEND_DATA(0xD0);
                TFT_SEND_DATA(0x05);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x08);
                TFT_SEND_DATA(0x03);
                TFT_SEND_DATA(0x24);
                TFT_SEND_DATA(0x32);
                TFT_SEND_DATA(0x32);
                TFT_SEND_DATA(0x3B);
                TFT_SEND_DATA(0x14);
                TFT_SEND_DATA(0x13);
                TFT_SEND_DATA(0x28);
                TFT_SEND_DATA(0x2F);

                TFT_SEND_CMD(0x21); 		//反显
               
                TFT_SEND_CMD(0x29);         //开启显示 

  }
  
void display_char16_16(unsigned int x,unsigned int y,unsigned long color,unsigned char word_serial_number)
{
   unsigned int column;
  unsigned char tm=0,temp=0,xxx=0;

   TFT_SEND_CMD(0x2a);    //Column address set
  TFT_SEND_DATA(x>>8);    //start column
  TFT_SEND_DATA(x);
  x=x+15;
  TFT_SEND_DATA(x>>8);    //end column
  TFT_SEND_DATA(x);

  TFT_SEND_CMD(0x2b);     //Row address set
  TFT_SEND_DATA(y>>8);    //start row
  TFT_SEND_DATA(y); 
  y=y+15;
  TFT_SEND_DATA(y>>8);    //end row
  TFT_SEND_DATA(y);
    TFT_SEND_CMD(0x2C);     //Memory write
    
    
  for(column=0;column<32;column++)  //column loop
          {
        temp=chines_word[  word_serial_number ][xxx];
        for(tm=0;tm<8;tm++)
        {
        if(temp&0x01)
          {
          TFT_SEND_DATA(color>>8);
          TFT_SEND_DATA(color);
          }
        else 
          {
          TFT_SEND_DATA(0XFF);
          TFT_SEND_DATA(0XFF);
          }
          temp>>=1;
        }
        xxx++;
          
      }
}

void Picture_Display(const unsigned char *ptr_pic)
{
    unsigned long  number;
	TFT_SEND_CMD(0x2a); 		//Column address set
	TFT_SEND_DATA(0x00); 		//start column
	TFT_SEND_DATA(0x00); 
	TFT_SEND_DATA(0x00);		//end column
	TFT_SEND_DATA(0x77);

	TFT_SEND_CMD(0x2b); 		//Row address set
	TFT_SEND_DATA(0x00); 		//start row
	TFT_SEND_DATA(0x00); 
	TFT_SEND_DATA(0x00);		//end row
	TFT_SEND_DATA(0x78);
	TFT_SEND_CMD(0x2C);			//Memory write
	  
	for(number=0;number<PIC_NUM;number++)	
          {
//            data=*ptr_pic++;
//            data=~data;
            TFT_SEND_DATA(*ptr_pic++);
	
			
          }
  }

  
void lcd_test(void)
{ 
	point= &picture_tab[0];
	IO_init();
	SPI_BLK_1;
	SPI_SCK_0;
	TFT_init();

	
	while(1)
	{
		
		TFT_full(RED);
		delay_ms(2000);
		TFT_full(GREEN);
		delay_ms(2000);
		TFT_full(BLUE);
		delay_ms(2000);
		TFT_clear();
		Picture_Display(point);
		delay_ms(5000);
//		display_char16_16(20,160,BLUE,0);
//		display_char16_16(36,160,GREEN,1);
//		display_char16_16(60,160,RED,2);
//		display_char16_16(76,160,BLUE,3);
//		display_char16_16(92,160,GREEN,4);
//		display_char16_16(118,160,BLUE,5);
//		display_char16_16(134,160,RED,6);
		
		display_char16_16(20,160,BLUE,7);
		display_char16_16(36,160,GREEN,8);
		display_char16_16(52,160,RED,9);
		display_char16_16(68,160,RED,10);
		delay_ms(5000);

	}
}
