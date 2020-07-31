#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int
sbit light=P2^1;
sbit key1=P3^1;
sbit key2=P3^2;

sbit DIN=P3^4;
sbit CS=P3^5;
sbit DCLK=P3^6;
sbit DOUT=P3^7;
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
		DIN=dat;
		dat<<=1;
		DCLK=1;
		
		DCLK=0;
	}
}
//AD读取数据操作
uint adRead()
{
	uint i,dat=0;
	CS=0;
	for(i=0;i<12;i++)
	{
		DCLK=0;
		dat<<=1;
		dat|=DOUT;
		DCLK=1;
	}
	return dat;
}
//命令转换操作
uint adReadData(uchar dat)		//光敏电阻为 0xc4
{
	uint i,AD_value;
	DCLK=0;
	CS=0;
	adWrite(dat);
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
void adSwitch(uchar dat)
{
	if(adReadData(dat)<200)
		switchLight(1);
	else
		switchLight(2);
}