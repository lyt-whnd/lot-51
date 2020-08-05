#include <reg52.h>
#include <intrins.h>
#include <stdio.h>
#define uchar unsigned char
#define uint unsigned int
sbit mode=P2^0;			//mode=0Ϊ������WiFi����  mode=1Ϊ����������
sbit S1=P1^7;
sbit S2=P1^3;
sbit light=P2^1;
sbit key1=P3^2;    //����3
sbit key2=P3^3;		 //����4		

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
uchar code table5[]="low";
uchar code table6[]="high";
uchar code table7[]="on";
uchar code table8[]="off";
void lcdWriteCom(uchar dat);
void lcdString(uchar *str,uint length);
/*��ʱms����*/
void delayms(uint z)
{
	uint i,j;
	for(i=z;i>0;i--)
		for(j=110;j>0;j--);
}
/*�ƵĿ���*/
void switchLight(uchar i)
{
	if(i==1)
		light=0;
	if(i==2)
		light=1;
}
/*��������С�ƿ���*/
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
/*�������������Ƶƿ���*/

//ADд�����
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
//AD��ȡ���ݲ���
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
//����ת������
uint adReadData()		//��������Ϊ 0xc4
{
	uint i,AD_value;
	DCLK=0;
	CS=0;
	adWrite(0xc4);
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
//��װ����
void adSwitch()
{
	if(adReadData()<200)
		switchLight(1);
	else
		switchLight(2);
}

/*WIFI����С�Ƶ�����*/
//��ʼ��
void wifiInit()
{
	TMOD=0x20;		//������9600
	TL1=0xfd;
	TH1=0xfd;
	TR1=1;
	SM0=0;
	SM1=1;
	REN=1;
}
//����WiFi�򿪶˿ں�
void wifiConfig()
{
	//Һ����ʾ��������
	lcdWriteCom(0x80+0x02);
	lcdString(table1,10);
	
	printf("AT+CWMODE+1");
	delayms(1000);
	printf("AT+RST");
	delayms(1000);
	printf("AT+CWJAP=\"XXX\",\"XXX\"");		//�˴�ΪҪ���ӵ�WIFI���ƺ�����
	delayms(1000);
	printf("AT+CIPMUX=1");
	delayms(1000);
	printf("AT+CIPSERVER=1,8080");
	delayms(1000);
	//Һ����ʾ���óɹ�
	lcdWriteCom(0x01);
	lcdWriteCom(0x80);
	lcdString(table2,2);
}
//��װ����
void wifiSwitch()
{
	uchar a;
	a=SBUF;
	if(a=='1')
		switchLight(1);
	if(a=='2')
		switchLight(2);
}
/*Һ��ʵʱ״̬����*/
//Һ��д����
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
//Һ��д����
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
//��ʼ��Һ��
void lcdInit()
{
	lcdWriteCom(0x38);
	lcdWriteCom(0x0c);
	lcdWriteCom(0x06);
	lcdWriteCom(0x01);
}
//�ַ�����ʾ����
void lcdString(uchar *str,uint length)
{
	uint i;
	for(i=0;i<length;i++)
	{
		lcdWriteData(str[i]);
	}
}
//��ǿ��ʾ
void lcdLight()
{
	lcdWriteCom(0x80+0x40);
	lcdString(table4,5);
	lcdWriteCom(0x80+0x46);
	if(adReadData()<200)
		lcdString(table5,3);
	else
		lcdString(table6,4);
}
//Һ����ʾ�Ƶ�״̬�仯
void lcdStatus()
{
	lcdWriteCom(0x80);
	if(light==0)
		lcdString(table7,2);
	if(light==1)
		lcdString(table8,3);
}
/*ģʽ����*/
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
/*������*/
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
		wifiSwitch();
		modeConfig();
	}
}