#include <reg51.h>	         //���õ�Ƭ��ͷ�ļ�
//#include <REGX52.H>
#include <intrins.h>		  //_nop_()����ͷ�ļ�
#define uchar unsigned char  //�޷����ַ��� �궨��	������Χ0~255
#define uint  unsigned int	 //�޷������� �궨��	������Χ0~65535

sbit DSPORT = P1^5;	//18b20 IO�ڵĶ���
sbit beep = P1^4;  //������IO�ڶ���

//bit flag_250ms ;	//250����ı���
int Time;

uchar flag_lj_en;		 //��������ʹ��


sbit relay2 = P1^6;  //��ĭ�̵���IO�ڶ���

sbit rs=P1^0;	 //�Ĵ���ѡ���ź� H:���ݼĴ���  	L:ָ��Ĵ���
sbit rw=P1^1;	 //�Ĵ���ѡ���ź� H:���ݼĴ���  	L:ָ��Ĵ���
sbit e =P1^2;	 //Ƭѡ�ź�   �½��ش���
#define LCD_DataPort P0

sbit cs=P2^3;//ʹ��
sbit clk=P2^4;//ʱ��
sbit dio=P2^5;// ����

uchar menu_1;          //���ò�ͬ�����ı���
uchar menu_2;

int t_low = -5;	 //�¶�������ֵ
int t_high = 30;


uchar shi,fen,miao,flag_ds;	 //ʱ�����

sbit key1 = P3^5;	    //����IO�ڶ���
sbit key2 = P3^6;		//����IO�ڶ���
sbit key3 = P3^7;		//����IO�ڶ���

int yali;
int wendu;
int tep;
uint vvv;
float Tmp;
//char state;

/***********************1ms��ʱ����*****************************/
void delay_1ms(uint q)
{
	uint i,j;
	for(i=0;i<q;i++)
		for(j=0;j<120;j++);
}


void delay_uint(uint q)
{
	while(q--);
}

void delay_us()    //С��ʱ
{
	_nop_();
	_nop_();
}


void LCD_Delay()
{
	unsigned char i, j;

	i = 2;
	j = 239;
	do
	{
		while (--j);
	} while (--i);
}


void write_com(uchar com)	 //д����
{
	//delay_uint(300);
	rs=0;
	rw=0;
	e=0;
	_nop_();
	LCD_DataPort=com;
	_nop_();
	//delay_1ms(1);
	e=1;
	_nop_();
	_nop_();
	//LCD_Delay();
	//delay_1ms(1);
	e=0;
	_nop_();
	//LCD_Delay();
}

void write_data(uchar dat)	  //д����
{
	//delay_uint(300);
	rs=1;
	rw=0;
	e=0;
	_nop_();
	LCD_DataPort=dat;
	_nop_();
	//delay_1ms(1);
	e=1;
	_nop_();
	_nop_();
	//LCD_Delay();
	//delay_1ms(1);
	e=0;
	_nop_();
	//LCD_Delay();
	//rs=0;
}


void write_string(uchar hang,uchar add,uchar *p)
{
	if(hang==1)
		write_com(0x80|(add-1));	   //1602д��һ�еĵ�ַ
	else
		write_com(0x80|(add-1)+0x40);   //1602д�ڶ��еĵ�ַ
	while(1)
	{
		if(*p == '\0')  break;	//\0�ַ����Ľ�β��־  break����whileѭ�� ����д�ַ�
		write_data(*p);	    //д����
		p++;	            //ָ���ַ��1
	}
}


/***********************lcd1602����ʾ2λʮ������************************/
void write_lcd2(uchar hang,uchar add,uint date)
{
	if(hang==1)
		write_com(0x80|(add-1));	   //1602д��һ�еĵ�ַ
	else
		write_com(0x80|(add-1)+0x40);  //1602д�ڶ��еĵ�ַ
	write_data(0x30+date/10%10);   //��ʾʮλ��
	write_data(0x30+date%10);	   //��ʾ��λ��
}



/***********************lcd1602����ʾ�ض����ַ�****0XDF ��********************/
void write_zifu(uchar hang,uchar add,uchar date)
{
	if(hang==1)
		write_com(0x80|(add-1));	   //1602д��һ�еĵ�ַ
	else
		write_com(0x80|(add-1)+0x40);  //1602д�ڶ��еĵ�ַ
	write_data(date);		  //д����
}

/***********************lcd1602��ʼ������************************/
void Init_1602()
{
	delay_1ms(15);
	//delay_uint(1000);
	write_com(0x38);
	delay_1ms(5);
	write_com(0x38);
	//write_com(0x38);
	write_com(0x0c);
	write_com(0x06);
	write_com(0x01);
/*	write_string(1, 1,"Y:   N");//ѹ��
    write_string(1, 8,"T:");//�¶�
    write_zifu(1,13,0xdf);
	write_zifu(1,14,'C'); 
	write_string(2,1,"00:00");
	write_com(0x0c);
	write_com(0x01);	*/
}


/***********************18b20��ʼ������*****************************/
void init_18b20()
{
	bit q;
	DSPORT = 1;				//�������ø�
	delay_uint(8);
	DSPORT = 0;				//����λ����  �ٽ������ߴӸ����ͣ�Ҫ�󱣳�480~960us
	delay_uint(80);
	DSPORT = 1;				//�������ø� �ȴ�
	delay_uint(8);
	q = DSPORT;				//��ȡ18b20��ʼ���ź�
	delay_uint(4);
	DSPORT = 1;				//�������ø� �ͷ�����
}


/*************д18b20�ڵ�����***************/
void write_18b20(uchar dat)
{
	uchar i;
	for(i=0;i<8;i++)	 //������д8λ��������Ҫѭ��8�β���д��
	{					 //д�����ǵ�λ��ʼ
		DSPORT = 0;			 //�������õ�дʱ��϶��ʼ
		DSPORT = dat & 0x01; //��18b20����дһλ��
		delay_uint(4);
		DSPORT = 1;			 //�ͷ�����
		dat >>= 1;		//��dat�еĸ�������λ��������1λ
	}
	delay_uint(4);	//������ʱ,��Ӳ��һ�㷴Ӧʱ��
}

/*************��ȡ18b20�ڵ�����***************/

uchar read_18b20()
{
	uchar byte,bi;
	uint i,j;	
	for(j=8;j>0;j--)
	{
		DSPORT=0;//�Ƚ���������1us
		i++;
		DSPORT=1;//Ȼ���ͷ�����
		i++;
		i++;//��ʱ6us�ȴ������ȶ�
		bi=DSPORT;	 //��ȡ���ݣ������λ��ʼ��ȡ
		/*��byte����һλ��Ȼ����������7λ���bi��ע���ƶ�֮���Ƶ���λ��0��*/
		byte=(byte>>1)|(bi<<7);						  
		i=4;		//��ȡ��֮��ȴ�48us�ٽ��Ŷ�ȡ��һ����
		while(i--);
	}				
	return byte;
}

/*************��ȡ�¶ȵ�ֵ ����������С��***************/
int read_temp()
{
	int value;
	uchar low,high;			   //�ڶ�ȡ�¶ȵ�ʱ������жϵ�̫Ƶ���ˣ���Ӧ�ð��жϸ����ˣ������Ӱ�쵽18b20��ʱ��
	init_18b20();		   //��ʼ��18b20
	write_18b20(0xcc);	   //����64λROM
	write_18b20(0x44);	   //����һ���¶�ת������
	delay_uint(50);		   //ת��һ���¶���Ҫʱ�� ��ʱһ��

	init_18b20();		   //��ʼ��18b20

	write_18b20(0xcc);	   //����64λROM
	write_18b20(0xbe);	   //������ȡ�ݴ�������

 	low = read_18b20();	   //���¶ȵ��ֽ�
	high = read_18b20();  //���¶ȸ��ֽ�
	value = high;
 	value <<= 8;		   //���¶ȵĸ�λ����8λ
	value |= low;		   //�Ѷ������¶ȵ�λ�ŵ�value�ĵͰ�λ��
	//value *= 0.625;	       //ת�����¶�ֵ С��
	return value;		   //���ض������¶� ��С��
}

/*****************0832ת����������*****************/
uchar ADC_read_data(uchar ch)
{
	uchar i,dat0=0,dat1=0;
	cs=0;
	clk=0;
	dio=1;
	delay_us();
	clk=1;
	delay_us();	  	//��һ���½���֮ǰDIO�øߣ���ʼ�ź�

	clk=0;
	dio=1;
	delay_us();
	clk=1;		 //�ڶ����½�������dio=1
	delay_us();

	clk=0;
	dio=ch; //�������½���,��DIO=0;
	delay_us();
	clk=1;
	delay_us();

/*********���������½���ѡ��ͨ��1************************/

	clk=0;
	dio=1;  //���ĸ��½���֮ǰ����DIO=1
	delay_us();


	for(i=0;i<8;i++)//��4~11��8���½��ض����ݣ�MSB->LSB��
	{
		clk=1;
		delay_us();
		clk=0;
		delay_us();
		dat0=dat0<<1|dio;
	}
	for(i=0;i<8;i++) //��11~18��8���½��ض����ݣ�LSB->MSB��
	{
		dat1=dat1|((uchar)(dio)<<i);
		clk=1;
		delay_us();
		clk=0;
		delay_us();
	}
	cs=1;				  //�ж�dat0��dat1�Ƿ����
	return (dat0==dat1)?dat0:0;
}

/*************��ʱ��0��ʼ������***************/
void time_init()
{
	EA   = 1;	 	  //�����ж�
	TMOD = 0X01;	  //��ʱ��0��������ʽ1
	ET0  = 1;		  //����ʱ��0�ж�
	TR0  = 1;		  //����ʱ��0��ʱ
	TH0 = 0x3c;
	TL0 = 0xb0;     // ��ʱ50ms�ж�һ��
}

/********************������������*****************/
uchar key_can;	 //����ֵ

void key()	 //������������
{
	key_can = 0;                //����ֵ��ԭ��0
 	if(key1 == 0 || key2 == 0 || key3 == 0)		//�а�������
	{
		delay_1ms(1);	     	//������ʱ������
		if(key1 == 0)		//ȷ���ǰ�������
			key_can = 3; 	//�õ�����ֵ
		if(key2 == 0)		//ȷ���ǰ�������
			key_can = 2; 	//�õ�����ֵ
		if(key3 == 0)		//ȷ���ǰ�������
			key_can = 1; 	//�õ�����ֵ
		flag_lj_en ++;   //���ӱ�����1
	}
	else
		flag_lj_en = 0;		   //�ر�����ʹ��
}

/****************������ʾ����***************/
void key_with()
{
	if(menu_1 == 0)
	{
		if(key_can == 3)
			relay2 = ~relay2;//�򿪹رռ�ĭ�̵���
        if(key_can == 2)
        {
            menu_2 ++;
            if(menu_2 > 2)
            {
                menu_2 = 0;
                Init_1602();
            }
            if(menu_2 == 1)
            {
                flag_ds = 1;    //��ʱģʽ��
            }
            if(menu_2 == 2)
            {
                flag_ds = 0;   //��ʱģʽ��
            }
        }
	}

 	if(key_can == 1)	   //���ü�
	{
		menu_1 ++;
		if(menu_1 > 2)
		{
			menu_1 = 0;		 //menu_1 = 0 �˳������ˣ���������ʾ������
			Init_1602();     //1602��ʼ��
		}
 	}
	if(menu_1 == 1)			//����ʱ��
	{
 		if(key_can == 2)	 //�Ӽ�
		{
			if(flag_lj_en <= 3)
				shi ++ ;		//��������δ�ɿ���1 ������
			else
				shi += 5;   	//��������δ�ɿ������κ��5
			if(shi > 23)
				shi = 0;
		}
		if(key_can == 3)	   //����
		{
			if(flag_lj_en <= 3)
			{
				if(shi == 0)
					shi = 24;
				shi -- ;			//��������δ�ɿ���1 ������
			}
			else
			{
				if(shi < 5)
					shi = 28;
				shi -= 5;		   	//��������δ�ɿ������κ��5
			}
 		}
 		write_lcd2(2,1,shi);  			 //��ʾʱ
		write_com(0xc0+0);              //������ƶ�����1�е�10λ
		write_com(0x0f);                 //��ʾ��겢����˸
	}
	if(menu_1 == 2)			//���÷���
	{
  		if(key_can == 2)	 //�Ӽ�
		{
			if(flag_lj_en <= 3)
				fen ++ ;		//��������δ�ɿ���1 ������
			else
				fen += 5;   	//��������δ�ɿ������κ��5
			if(fen > 60)
				fen = 0;
		}
		if(key_can == 3)	   //����
		{
			if(flag_lj_en <= 3)
			{
				if(fen == 0)
					fen = 60;
				fen -- ;			//��������δ�ɿ���1 ������
			}
			else
			{
				if(fen < 5)
					fen = 65;
				fen -= 5;		   	//��������δ�ɿ������κ��5
			}
 		}
 		write_lcd2(2,4,fen);		 //��ʾ����
		write_com(0xc0+3);          //������ƶ�����1�е�13λ
		write_com(0x0f);             //��ʾ��겢����˸
	}
	delay_1ms(400);
}



/************ѹ��ת��*************/
void Yali_()
{
    vvv=ADC_read_data(1);
    Tmp=vvv*1000.0/255.0;//ת��
    yali=Tmp;
}
/*******************************/

/****************���Ƽ̵�������***************/

void clock_h_l()
{
	//uchar value;
	Yali_();
	if(flag_ds == 0)
	{
		if(wendu <= t_low||yali <= 10)  //�¶�С�ڵ����¶��¶�����
		{
			relay2 = 0;	     //�̵������Ϲ���
			beep = 0;     //����������ʾ
			delay_1ms(1500);
			beep = 1;   //�رշ�����
		}
		if(wendu >= t_high||yali >= 30)  //�¶ȴ��ڵ����¶��¶�����
        {
            relay2 = 1;	     //�رռ̵���
            delay_1ms(1500);
        }
	}
}		

/****************��ʱ����***************/


void dingshi_dis()  //��ʱ����
{
	uchar value;
	uchar a,b;
	a = shi;
	b = fen;
	if((shi == 0) && (fen == 0))   //��ʱʱ��Ϊ0ʱ�˳���ʱģʽ
		flag_ds = 0;
	if(flag_ds == 1)
	{
		  //��ʱģʽ
		value ++;
		if(value == 2)
			write_zifu(2,3,' ');
		if(value >= 4)	  //1S
		{
			write_zifu(2,3,':');
			value = 0;
			if(fen + shi > 0)   //��ʼ��ʱ
			{
				miao ++;
				if(miao >= 60)
				{
					miao = 0;
					if(fen == 0)
					{
						fen = 60;
						if(shi != 0)
							shi --;
					}
					fen --;

					if((fen == 0) && (shi == 0))	 //��ʱʱ�䵽
					{
						//relay1 = 0; // �򿪼��ȼ̵���
						//relay2=0;//�򿪼���ĭ�̵���
						relay2=~relay2;
						beep = 0;     //����������ʾ
						delay_1ms(1500);
						beep = 1;   //�رշ�����
						shi = a;
						fen = b;
					}
 				}
				write_lcd2(2,4,fen);	 //��ʾ����
				write_lcd2(2,1,shi);    //��ʾʱ��
			}
		}
	}
}



/*************��ʱ��0�жϷ������***************/

void time0() interrupt 1
{
	TH0 = 0x4C;	 //���ó�ʼֵ
    TL0 = 0x00; 
	Time++;
}

/******************���ڳ�ʼ��*********************/

void init_uart(){
    EA = 1;
    //ES=1;
    TR1 = 1;
    SCON = 0x50;
    TMOD = 0x21;
    PCON = 0x00;
    TH1 = 0xFD;
    TL1 = 0xFD;
}

/***************���ڷ�һ������******************/

void send_uart(uchar ch)
{
    SBUF = ch;
    while(!TI);
    TI = 0;
}

/***************���ڷ�һ������******************/

void send_uart0_dat(char *s)
{
    while(*s)
    {
        send_uart(*s++);
    }
}




/****************�¶���ʾ����***************/
void LcdDisplay(int temp)
{
    float tp;
	if(temp< 0)				//���¶�ֵΪ����
  	{
		//��Ϊ��ȡ���¶���ʵ���¶ȵĲ��룬���Լ�1����ȡ�����ԭ��
		temp=temp-1;
		temp=~temp;
		tp=temp;
		temp=tp*0.0625*10+0.5;
  	}
 	else
  	{
		tp=temp;//��Ϊ���ݴ�����С�������Խ��¶ȸ���һ�������ͱ���
		//����¶���������ô����ô������ԭ����ǲ���������
		temp=tp*0.0625*10+0.5;
	}
	wendu=temp;

	write_com(0x80+9);		  //д��ַ 80��ʾ��ʼ��ַ
	write_data('0'+wendu/100%10); //ʮλ
	write_data('0'+wendu/10%10); //��λ
	write_data('.'); 		//��ʾ ��.��
	write_data('0'+wendu%10); //��ʾС����
	//write_data(0xdf);
	//write_data('C');

	send_uart0_dat("Temp: ");
	send_uart('0'+wendu/100%10); //ʮλ
	send_uart('0'+wendu/10%10); //��λ
	send_uart('.'); 		//��ʾ ��.��
	send_uart('0'+wendu%10); //��ʾС����
	send_uart('C');
	send_uart('\r');
	send_uart('\n');

	send_uart('\r');
	send_uart('\n');
}


/*************ѹ����ʾ����*************/
void LcdDisplay_Y()
{
    Yali_();
    write_com(0x80+2);
	write_data('0'+yali/1000%10);
	write_data('0'+yali/100%10);
	write_data('0'+yali/10%10);
	write_data('0'+yali%10);

	send_uart0_dat("Yali: ");
	send_uart('0'+yali/1000%10);
	send_uart('0'+yali/100%10);
	send_uart('0'+yali/10%10);
	send_uart('0'+yali%10);
	send_uart('N');
	send_uart('\r');
	send_uart('\n');
}

/****************������***************/

void main()
{
	//beep = 0;		                //��������һ��
	delay_1ms(150);
	P0 = P1 = P2 = P3 = 0xff;		//���е�Ƭ��IO������ߵ�ƽ
	Init_1602();                    //1602��ʼ��
	time_init();                    //��ʼ����ʱ��
	init_uart();	//���ڳ�ʼ��
	write_com(0x01);
	write_string(1, 1,"Y:   N");//ѹ��
    write_string(1, 8,"T:     ");//�¶�
    write_zifu(1,14,0xdf);
    write_zifu(1,15,'C');
    write_string(2,1,"00:00");
	while(1)
	{
	    
 		//while(state==0){
        if(Time>=20)	        //250ms ����һ���¶ȳ���
        {
           Time=0;
           LcdDisplay_Y();
           LcdDisplay(read_temp());
           //clock_h_l();                //��������
           dingshi_dis();  //��ʱ����
         }
 		//}
		key();					//��������
		
 		key_with();			    //�����¶�
 	}
}
