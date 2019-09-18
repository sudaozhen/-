//毕业论文设计   苏道桢   AVR单片机程序部分
//基于单片机+语音MP3芯片设计的电梯智能语音报站系统
//     《单片机系统配置》   
//单片机型号： ATMEGA8L—8PU
//晶振  频率： 内部8M
//通讯  端口： # PD0（RXD）   # PD1（TXD）
//外部  中断： # PD2（INT0）  # PD3（INT1）
//             到站检测低电平  红外检测

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define  F_CPU 8000000 //设置cpu频率
#define  BAUD 9600     //设置波特率

void send_rs232 (char data)   //串口发送函数
{
	while(!(UCSRA & (1<<UDRE)));
	UDR=data;
}

void PLAY (void)   //播放函数
{
	send_rs232(0x7e);
	send_rs232(0x02);
	send_rs232(0x01);
	send_rs232(0xef);
}

void FOLDER (void) //文件夹循环函数
{
	send_rs232(0x7e);
	send_rs232(0x03);
	send_rs232(0x33);
	send_rs232(0x01);
	send_rs232(0xef);
}

void PAUSE (void) //暂停函数
{
	send_rs232(0x7e);
	send_rs232(0x02);
	send_rs232(0x02);
	send_rs232(0xef);
}

void STOP (void)   //停止函数
{
	send_rs232(0x7e);
	send_rs232(0x02);
	send_rs232(0x0e);
	send_rs232(0xef);
}

void VOICE3 (void) //插播报站语音
{
	send_rs232(0x7e);
	send_rs232(0x04);
	send_rs232(0x44);
	send_rs232(0x01);
	send_rs232(0x03);
	send_rs232(0xef);
}

void VOL (int l)     //音量设置函数
{
	send_rs232(0x7e);
	send_rs232(0x03);
	send_rs232(0x31);
	send_rs232(l);//0 ~ 1e HEX
	send_rs232(0xef);
	delay_xms(680);
}

void delay_1ms (void)
{
	unsigned int i;
	for(i=1;i<(unsigned int)(1*143-2);i++);
}
void delay_xms (unsigned int n)
{
	unsigned int i;
	for (i=0;i<n;i++)
	{
	delay_1ms();
	}
}

void init_rs232 (void)  //串口初始化
{
	UBRRH = (F_CPU/BAUD/16-1)/256;
	UBRRL = (F_CPU/BAUD/16-1)%256;
	UCSRA = 0x00;
	UCSRB |=(1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
}

int main(void)  //主程序
{
	int i;
	
	init_rs232();
	DDRD |= (1<<2);
	PORTD |= (1<<2);

	delay_xms(10000);
	FOLDER();//文件夹循环
	delay_xms(5000);
	
	PLAY(); //播放
	delay_xms(7000);
		VOL(0x06);
		VOL(0x07);
		VOL(0x08);
		VOL(0x09);
		VOL(0x10);
		VOL(0x1a);
		VOL(0x1b);
		VOL(0x1c);
		VOL(0x1d);
		VOL(0x1e);
	delay_xms(400000);
	MCUCR |= (1<<ISC10);//INT0低电平沿触发，INT1任意电平触发
	GICR |=(1<<INT0)|(1<<INT1);//使能INT0，INT1
	delay_xms(400000);
	sei();//使能全局中断
	
	while(1)
	{
	}
}

volatile long read_rs232;//串口接受中断服务
SIGNAL (SIG_UART_RECV)
{
	read_rs232=UDR;
//	if (read_rs232 == 'err')
	{
	//	send_cmd(FOLDER);
	//	send_cmd(PLAY);
	}
}

SIGNAL (SIG_INTERRUPT0) //到站中断服务（下降沿）
{
	GICR &=~(1<<INT0);//禁止INT0
	sei();
	delay_xms(1000);//检测停留时间（###调试修改###）
	if (!(PIND & (1<<2)))
	{
		VOICE3();
		while (!(PIND & (1<<2)))//低电平死循环
		{
		delay_xms(100000);
		}
	}
	GICR |=(1<<INT0);//使能INT0
}
SIGNAL (SIG_INTERRUPT1) //红外中断服务（上升沿）
{
	GICR &=~(1<<INT1);//禁止INT1
	delay_xms(5000);
	if (!(PIND & (1<<3)))
	{
		PAUSE();
	}
	if (PIND & (1<<3))
	{
		VOL(0x05);
		PLAY();
		VOL(0x06);
		VOL(0x07);
		VOL(0x08);
		VOL(0x09);
		VOL(0x10);
		VOL(0x1a);
		VOL(0x1b);
		VOL(0x1c);
		VOL(0x1d);
		VOL(0x1e);
	}
	delay_xms(10000);
	GICR |=(1<<INT1);//使能INT1
}
