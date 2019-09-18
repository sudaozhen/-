//��ҵ�������   �յ���   AVR��Ƭ�����򲿷�
//���ڵ�Ƭ��+����MP3оƬ��Ƶĵ�������������վϵͳ
//     ����Ƭ��ϵͳ���á�   
//��Ƭ���ͺţ� ATMEGA8L��8PU
//����  Ƶ�ʣ� �ڲ�8M
//ͨѶ  �˿ڣ� # PD0��RXD��   # PD1��TXD��
//�ⲿ  �жϣ� # PD2��INT0��  # PD3��INT1��
//             ��վ���͵�ƽ  ������

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define  F_CPU 8000000 //����cpuƵ��
#define  BAUD 9600     //���ò�����

void send_rs232 (char data)   //���ڷ��ͺ���
{
	while(!(UCSRA & (1<<UDRE)));
	UDR=data;
}

void PLAY (void)   //���ź���
{
	send_rs232(0x7e);
	send_rs232(0x02);
	send_rs232(0x01);
	send_rs232(0xef);
}

void FOLDER (void) //�ļ���ѭ������
{
	send_rs232(0x7e);
	send_rs232(0x03);
	send_rs232(0x33);
	send_rs232(0x01);
	send_rs232(0xef);
}

void PAUSE (void) //��ͣ����
{
	send_rs232(0x7e);
	send_rs232(0x02);
	send_rs232(0x02);
	send_rs232(0xef);
}

void STOP (void)   //ֹͣ����
{
	send_rs232(0x7e);
	send_rs232(0x02);
	send_rs232(0x0e);
	send_rs232(0xef);
}

void VOICE3 (void) //�岥��վ����
{
	send_rs232(0x7e);
	send_rs232(0x04);
	send_rs232(0x44);
	send_rs232(0x01);
	send_rs232(0x03);
	send_rs232(0xef);
}

void VOL (int l)     //�������ú���
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

void init_rs232 (void)  //���ڳ�ʼ��
{
	UBRRH = (F_CPU/BAUD/16-1)/256;
	UBRRL = (F_CPU/BAUD/16-1)%256;
	UCSRA = 0x00;
	UCSRB |=(1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
}

int main(void)  //������
{
	int i;
	
	init_rs232();
	DDRD |= (1<<2);
	PORTD |= (1<<2);

	delay_xms(10000);
	FOLDER();//�ļ���ѭ��
	delay_xms(5000);
	
	PLAY(); //����
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
	MCUCR |= (1<<ISC10);//INT0�͵�ƽ�ش�����INT1�����ƽ����
	GICR |=(1<<INT0)|(1<<INT1);//ʹ��INT0��INT1
	delay_xms(400000);
	sei();//ʹ��ȫ���ж�
	
	while(1)
	{
	}
}

volatile long read_rs232;//���ڽ����жϷ���
SIGNAL (SIG_UART_RECV)
{
	read_rs232=UDR;
//	if (read_rs232 == 'err')
	{
	//	send_cmd(FOLDER);
	//	send_cmd(PLAY);
	}
}

SIGNAL (SIG_INTERRUPT0) //��վ�жϷ����½��أ�
{
	GICR &=~(1<<INT0);//��ֹINT0
	sei();
	delay_xms(1000);//���ͣ��ʱ�䣨###�����޸�###��
	if (!(PIND & (1<<2)))
	{
		VOICE3();
		while (!(PIND & (1<<2)))//�͵�ƽ��ѭ��
		{
		delay_xms(100000);
		}
	}
	GICR |=(1<<INT0);//ʹ��INT0
}
SIGNAL (SIG_INTERRUPT1) //�����жϷ��������أ�
{
	GICR &=~(1<<INT1);//��ֹINT1
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
	GICR |=(1<<INT1);//ʹ��INT1
}
