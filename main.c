#include <reg52.h>
#include <intrins.h>
#include <stdio.h>
#define uchar unsigned char
#define uint unsigned int
sbit mode=P2^0;			//mode=0为按键和WiFi控制  mode=1为传感器控制
sbit S1=P1^7;
sbit S2=P1^3;
sbit light=P2^1;
sbit key1=P3^2;    //按键3
sbit key2=P3^3;		 //按键4		

sbit DIN=P3^4;
sbit CS=P3^5;
sbit DCLK=P3^6;
sbit DOUT=P3^7;

sbit EN=P2^7;
sbit RS=P2^6;
sbit lcdWR=P2^5;

uchar code table1[]="loading...";
uchar code table2[]="OK";
uchar code table3[]="status";
uchar code table4[]="Light";
uchar code table5[]="low ";
uchar code table6[]="high";
uchar code table7[]="on ";
uchar code table8[]="off";
void lcdWriteCom(uchar dat);
void lcdString(uchar *str,uint length);
/*延时ms函数*/
void delayms(uint z)
{
	uint i,j;
	for(i=z;i>0;i--)
		for(j=110;j>0;j--);
}
/*灯的开关*/
void switchLight(uchar i)
{
	if(i==1)
		light=0;
	if(i==2)
		light=1;
}
/*按键控制小灯开关*/
void keyControl(void)
{
	if(key1==0)
	{
		delayms(10);
		if(key1==0)
		{
			switchLight(1);
		}
		while(!key1);
	}
	if(key2==0)
	{
		delayms(10);
		if(key2==0)
		{
			switchLight(2);
		}
		while(!key2);
	}
}
/*光敏传感器控制灯开关*/

//AD写入操作
void adWrite(uchar dat)
{
	uint i;
	CS=0;
	DCLK=0;
	for(i=0;i<8;i++)
	{
		DIN=dat>>7;
		dat<<=1;
		DCLK=0;
		
		DCLK=1;
	}
}
//AD读取数据操作
uint adRead()
{
	uint i,dat=0;
	CS=0;
	DCLK=0;
	for(i=0;i<12;i++)
	{
		
		dat<<=1;
		DCLK=1;
		DCLK=0;
		dat|=DOUT;
		
	}
	return dat;
}
//命令转换操作
uint adReadData()		//光敏电阻为 0xa4
{
	uint i,AD_value;
	DCLK=0;
	CS=0;
	adWrite(0xa4);
	for(i=6;i>0;i--);
	DCLK=1;
	_nop_();
	_nop_();
	DCLK=0;
	_nop_();
	_nop_();
	AD_value=adRead();
	CS=1;
	return AD_value;
}
//封装开关
void adSwitch()
{
	if(adReadData()<150)
	{
		switchLight(1);
		delayms(10);
	}
	else
	{
		switchLight(2);
		delayms(10);
	}
}

/*WIFI控制小灯的亮灭*/
//初始化
void wifiInit()
{
	TMOD=0x20;		//波特率9600
	TL1=0xfd;
	TH1=0xfd;
	TR1=1;
	SM0=0;
	SM1=1;
	REN=1;
	EA=1;
	ES=1;
}
//设置WiFi打开端口号
void wifiConfig()
{
	//液晶显示正在设置
	lcdWriteCom(0x80+0x02);
	lcdString(table1,10);
	ES=0;
	TI=1;
	delayms(3000);

	printf("AT+CIPMUX=1\r\n");
	delayms(2000);
	printf("AT+CIPSERVER=1,8080\r\n");
	delayms(2000);
	while(!TI);
	TI=0;
	ES=1;
	//液晶显示设置成功
	lcdWriteCom(0x01);
	lcdWriteCom(0x80);
	lcdString(table2,2);
}
//封装开关
/*void wifiSwitch()
{
	uchar a;
	a=SBUF;
	if(a=='1')
		switchLight(1);
	if(a=='2')
		switchLight(2);
}*/
/*液晶实时状态回显*/
//液晶写命令
void lcdWriteCom(uchar dat)
{
	RS=0;
	lcdWR=0;
	P0=dat;
	delayms(5);
	EN=1;
	delayms(5);
	EN=0;
}
//液晶写数据
void lcdWriteData(uchar dat)
{
	RS=1;
	lcdWR=0;
	P0=dat;
	delayms(5);
	EN=1;
	delayms(5);
	EN=0;
}
//初始化液晶
void lcdInit()
{
	lcdWriteCom(0x38);
	lcdWriteCom(0x0c);
	lcdWriteCom(0x06);
	lcdWriteCom(0x01);
}
//字符串显示函数
void lcdString(uchar *str,uint length)
{
	uint i;
	for(i=0;i<length;i++)
	{
		lcdWriteData(str[i]);
	}
}
//光强显示
void lcdLight()
{
	lcdWriteCom(0x80+0x40);
	lcdString(table4,5);
	lcdWriteCom(0x80+0x46);
	if(adReadData()<200)
		lcdString(table5,4);
	else
		lcdString(table6,4);
}
//液晶显示灯的状态变化
void lcdStatus()
{
	lcdWriteCom(0x80);
	if(light==0)
		lcdString(table7,3);
	if(light==1)
		lcdString(table8,3);
}
/*模式设置*/
void modeConfig()
{
	if(S1==0)
	{
		delayms(10);
		if(S1==0)
		{
			mode=~mode;
		}
		while(!S1);
	}
	if(mode==1)
	{
		adSwitch();
	}
}
/*主函数*/
void main()
{
	wifiInit();
	lcdInit();
	wifiConfig();
	mode=0;
	S1=1;
	S2=0;
	while(1)
	{
		lcdStatus();
		lcdLight();
		keyControl();
	//	wifiSwitch();
		modeConfig();
	}
}
void wifi() interrupt 4
{
	uchar a;
	a=SBUF;
	if(a=='1')
		switchLight(1);
	if(a=='2')
		switchLight(2);
	RI=0;
}