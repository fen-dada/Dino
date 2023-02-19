
#include "oled.h"
#include "oledfont.h"
#include "main.h"
#include "spi.h"

//OLED的显存
//存放格式如下
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127
//向SSD1306写入一个字节
//dat:要写入的数据/命令
//cmd:数据/命令标志 0，表示命令；1，表示数据
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
    if(cmd)
    {
        OLED_DC_Set();//命令/数据标志位置为1，则表示传送的是命令字节
    }

    else
        OLED_DC_Clr();//命令/数据标志位置为0，则表示传送的是数据字节
    OLED_CS_Clr();//片选信号为低，表示选中OLED
    HAL_SPI_Transmit(&hspi1,&dat,1,1000);//oled.c文件唯一修改的地方，这里是利用了hal库提供的SPI传送函数
    OLED_CS_Set();
    OLED_DC_Set();
}

void OLED_Set_Pos(unsigned char x, unsigned char y)
{
    OLED_WR_Byte(0xb0+y,OLED_CMD);
    OLED_WR_Byte((((x+2)&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte(((x+2)&0x0f),OLED_CMD);
}
//开启OLED显示
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //设置电荷泵命令字
    OLED_WR_Byte(0X14,OLED_CMD);  //开启电荷泵
    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //设置电荷泵命令字
    OLED_WR_Byte(0X10,OLED_CMD);  //关闭电荷泵
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}
//清屏函数，清完后整个屏幕都是黑色的，没有一点光亮
void OLED_Clear(void)
{
    uint8_t i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址
        OLED_WR_Byte (0x02,OLED_CMD);      //设置起始列低地址
        OLED_WR_Byte (0x10,OLED_CMD);      //设置起始列高地址
        for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA);
    } //更新显示
}


//在指定位置显示一个字符，包括部分字符
//x:0~127
//y:0~6
//mode:0,反白显示；1，正常显示
//size:选择字体大小 16/12
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr)
{
    unsigned char c=0,i=0;
    c=chr-' ';//得到偏移后的值
    if(x>Max_Column-1){x=0;y=y+2;}
    if(SIZE ==16)
    {
        OLED_Set_Pos(x,y);
        for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
        OLED_Set_Pos(x,y+1);
        for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
    }
    else {
        OLED_Set_Pos(x,y+1);
        for(i=0;i<6;i++)
            OLED_WR_Byte(F6x8[c][i],OLED_DATA);

    }
}
//m^n函数
uint32_t oled_pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;
    while(n--)result*=m;
    return result;
}
//显示两个数字
//x,y :起点坐标
//len :数字的位数
//size:字体大小
//mode:0:填充模式；1:叠加模式
//num:数值(0~4294967295);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{
    uint8_t t,temp;
    uint8_t enshow=0;
    for(t=0;t<len;t++)
    {
        temp=(num/oled_pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                OLED_ShowChar(x+(size/2)*t,y,' ');
                continue;
            }else enshow=1;

        }
        OLED_ShowChar(x+(size/2)*t,y,temp+'0');
    }
}
//显示一个字符串
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr)
{
    unsigned char j=0;
    while (chr[j]!='\0')
    {		OLED_ShowChar(x,y,chr[j]);
        x+=8;
        if(x>120){x=0;y+=2;}
        j++;
    }
}
//显示汉字
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no)
{
    uint8_t t,adder=0;
    OLED_Set_Pos(x,y);
    for(t=0;t<16;t++)
    {
        OLED_WR_Byte(Hzk[2*no][t],OLED_DATA);
        adder+=1;
    }
    OLED_Set_Pos(x,y+1);
    for(t=0;t<16;t++)
    {
        OLED_WR_Byte(Hzk[2*no+1][t],OLED_DATA);
        adder+=1;
    }
}
/*显示BMP图片。x的范围为0~127，y的页得的范围0~7*/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{
    unsigned int j=0;
    unsigned char x,y;

    if(y1%8==0) y=y1/8;
    else y=y1/8+1;
    for(y=y0;y<y1;y++)
    {
        OLED_Set_Pos(x0,y);
        for(x=x0;x<x1;x++)
        {
            OLED_WR_Byte(BMP[j++],OLED_DATA);
        }
    }
}
int gui_run(int* a,int* a_trg)
{
	
	if(*a > *a_trg){
		*a-=1;
	}
	else if(*a < *a_trg){
		*a+=1;
	}
	else return 0;
	return 1;
	
	
}

void OLED_draw_black(int x, int y,int clear_size){
		unsigned int i=0;
		OLED_Set_Pos(x,y);
		for(i=0;i<clear_size;i++){
			if(x+i>127) break;
				OLED_WR_Byte(0x0,OLED_DATA);
		}
}

void OLED_ground()
{
	static unsigned int pos=0,pos_trg=0;
	unsigned char speed=5;
	unsigned int len = sizeof(ground);
    unsigned int j=0;
    unsigned char x,y;
        OLED_Set_Pos(0,7);
        for(x=0;x<127;x++)
        {
            OLED_WR_Byte(ground[(x+pos)%len],OLED_DATA);
        }
				//OLED_Set_Pos(32,7);
				/*
				for(x=32;x<127;x++)
        {
            OLED_WR_Byte(ground[(x+pos)%len],OLED_DATA);
        }
				*/
				pos_trg+=speed;
				if(pos>len) pos=0;
				gui_run(&pos,&pos_trg);
}

void OLED_cloud(){
	//srand((unsigned)time(NULL));

	static int pos=127;
	static int pos_trg=127;
		char speed = 1;
	static char height=0;
	
	int x;
	int start_x=0;
	int len=sizeof(cloud);
	unsigned char byte=0x0;
	
	
	if(pos + len <= -speed){
			pos = 127;
		height=rand()%3;
	} 
		 if(pos<0){
				start_x=-pos;
			 OLED_Set_Pos(0,height);
		 }else{
				OLED_Set_Pos(pos,height);
		 }
		 
		 for(x=start_x;x<len+speed;x++)
     {
			 if(pos+x>=126) break;
			 
			 if(x<len) {byte = cloud[x];}
			 else {byte=0x0;}
			OLED_WR_Byte(byte,OLED_DATA);
     }
		 //byte = 0x0;
		 pos_trg-=speed;
		 gui_run(&pos,&pos_trg);
		 //HAL_Delay(50);
}

void OLED_dragon(){
	int ver=1;
	unsigned int j=0;
	static int pos=160;
	static int pos_trg=127;
		char speed = 3;
	static char height=0;
	unsigned char s=0;
	
	int x,y;
	int start_x=0;
	int len;
	if(ver==1) len=sizeof(dragon);
	else len=sizeof(dragon2);
	unsigned char byte=0x0;
	
	//OLED_ShowNum(20,1,len,3,2);
	
	if(pos + len <= -speed){
			pos = 127;
		height=rand()%2;
	} 
//	height=0;
		 
		 for(y=0;y<2;y++){
			 //OLED_Set_Pos(pos,height+1+y);
			 if(pos<0){
				start_x=-pos;
			 OLED_Set_Pos(0,height+y);
	}
		 else{
				OLED_Set_Pos(pos,height+y);
		 }
			 for(x=start_x;x<len/2;x++)
			 {
				 if(pos+x>=126) break;
				 j=y*len/2+x;
				 if(ver==1) byte = dragon[j];
				 else byte=dragon2[j];
				OLED_WR_Byte(byte,OLED_DATA);
				 ver++;
				 ver%=2;
			 }
			}
		 	OLED_draw_black(pos+len/2,height,20);
			OLED_draw_black(pos+len/2,height+1,20);
		 
		 //byte = 0x0;
		 pos_trg-=speed;
		 gui_run(&pos,&pos_trg);
		 //HAL_Delay(50);
}

void OLED_dino(){
		static unsigned char dino_dir=0;
		unsigned int j=0;
	static int s=0;
	unsigned char x,y;
	unsigned char byte=0x0;
		
		dino_dir++;
		if(dino_dir<5){
				s=0;
		}else if(dino_dir<10){
				s=1;
		}
		dino_dir%=10;
		for(y=0;y<2;y++){
				OLED_Set_Pos(16,5+y);

				for(x=0;x<16;x++){
						j=y*16+x;
						byte=dino[s][j];
					OLED_WR_Byte(byte,OLED_DATA);
				}

		}
		byte=0x0;
}

int OLED_cactus(int ver, char reset){
		char speed=5;
		static int pos =127,pos_trg=127;;
		int start_x=0;
		int len;
		unsigned char x,y;
		unsigned int j=0;
		unsigned char byte;
	
		if(reset==1){
		pos=127,pos_trg=127;
			start_x=0;
			j=0;
			return 127;
		}
		
		if(ver==0) len=8;
		else if(ver==1) len=16;
		else len=24;
		if(pos+len<0){
				OLED_draw_black(0,5,speed);
				pos=127;
		}
		for(y=0;y<2;y++){
				if(pos<0){
						start_x=-pos;
						OLED_Set_Pos(0,5+y);
				}else{
						OLED_Set_Pos(pos,5+y);
				}
				
				for(x=start_x;x<len;x++){
						if(x+pos>=126) break;
						j=y*len+x;
						if(ver ==0) byte=cactus[j];
						else if(ver==1) byte=cactus_2[j];
						else if(ver==3) byte=cactus_3[j];
						else byte = cactus_4[j];
					OLED_WR_Byte(byte,OLED_DATA);
				}
		}
		
		OLED_draw_black(pos+len,5,speed);
		OLED_draw_black(pos+len,6,speed);
		pos_trg-=speed;
		gui_run(&pos,&pos_trg);
		return pos;
}

int OLED_dino_jump(char reset){
		//static int arr[]={1,1,1,2,2,2,3,4,4,4,4,4};
		static int arr[]={1,1,3,3,4,4,5,6,7};
		static char speed_idx=8;
		static int height=0;
		static char dir=0;
		int len=8;
		//char speed =1;
		
		static int i=0,j=0;
		unsigned char x,y;
		char offset =0;
		unsigned char byte;
		i++;
		i%=10;
		if(reset==1){
				height=0;
				dir=0;
				speed_idx=len;
				return 0;
		}
		
		if(dir==0&&i%5==0){
				height+=arr[speed_idx];
				speed_idx--;
				if(speed_idx<0) speed_idx=0;
		}
		
		if(dir==1&&i%5==0){
				height-=arr[speed_idx];
				speed_idx++;
				if(speed_idx>len) speed_idx=len;
			
		}
		
		if(height>=31){
				dir=1;
				height=31;
		}
		
		if(height<=0){
				dir=0;
				height=0;
		}
		
		if(height<=7) offset=0;
		else if(height<=15) offset=1;
		else if(height<=23) offset=2;
		else if(height<=31) offset=3;
		else offset=4;
		
		//offset = height/8;
		for(y=0;y<3;y++){
				OLED_Set_Pos(16,4-offset+y);
			for(x=0;x<16;x++){
					j=y*16+x;
					byte=dino_jump[height%8][j];
				  OLED_WR_Byte(byte,OLED_DATA);
			}
			//HAL_Delay(50);
		}
		
		if(dir==0) OLED_draw_black(16,7-offset,16);
		if(dir==1) OLED_draw_black(16,3-offset,16);
		return height;
}

unsigned char key_scan(){
	unsigned char num=0;
		if(wkup==1){
				num=1;
		}
		
		else if(key1==0){
				num=2;
		}
		return num;
}



void OLED_restart(){
	unsigned int j=0;
	static unsigned char byte;
	static int x,y;
		for(y=2;y<5;y++){
				OLED_Set_Pos(52,y);
			for(x=0;x<24;x++){
					byte=restart[j++];
					OLED_WR_Byte(byte,OLED_DATA);
			}
		}
		
		OLED_ShowString(10,3,"GAME");
		OLED_ShowString(86,3,"OVER");
}

void OLED_cover(){
	int j=0;
	unsigned char byte;

	int x,y;
	for(y=0;y<8;y++){
				OLED_Set_Pos(0,y);
			for(x=0;x<128;x++){
				byte=cover[j++];
					OLED_WR_Byte(byte,OLED_DATA);
			}
	}
}


//初始化SSD1306
void OLED_Init(void)
{

    OLED_RST_Clr();
    HAL_Delay(200);
    OLED_RST_Set();

    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x02,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0Xa0左右反置 0Xa1正常
    OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0Xc0上下反置 0Xc8正常
    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
    OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x02,OLED_CMD);//
    OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
    OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
    OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7)
    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel

    OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/
    OLED_Clear();
}

