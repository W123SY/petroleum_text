#include <reg51.h>	         //调用单片机头文件
//#include <REGX52.H>
#include <intrins.h>		  //_nop_()函数头文件
#define uchar unsigned char  //无符号字符型 宏定义	变量范围0~255
#define uint  unsigned int	 //无符号整型 宏定义	变量范围0~65535

sbit DSPORT = P1^5;	//18b20 IO口的定义
sbit beep = P1^4;  //蜂鸣器IO口定义

//bit flag_250ms ;	//250毫秒的变量
int Time;

uchar flag_lj_en;		 //按键连加使能


sbit relay2 = P1^6;  //加沫继电器IO口定义

sbit rs=P1^0;	 //寄存器选择信号 H:数据寄存器  	L:指令寄存器
sbit rw=P1^1;	 //寄存器选择信号 H:数据寄存器  	L:指令寄存器
sbit e =P1^2;	 //片选信号   下降沿触发
#define LCD_DataPort P0

sbit cs=P2^3;//使能
sbit clk=P2^4;//时钟
sbit dio=P2^5;// 数据

uchar menu_1;          //设置不同参数的变量
uchar menu_2;

int t_low = -5;	 //温度上下限值
int t_high = 30;


uchar shi,fen,miao,flag_ds;	 //时间变量

sbit key1 = P3^5;	    //按键IO口定义
sbit key2 = P3^6;		//按键IO口定义
sbit key3 = P3^7;		//按键IO口定义

int yali;
int wendu;
int tep;
uint vvv;
float Tmp;
//char state;

/***********************1ms延时函数*****************************/
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

void delay_us()    //小延时
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


void write_com(uchar com)	 //写命令
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

void write_data(uchar dat)	  //写数据
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
		write_com(0x80|(add-1));	   //1602写第一行的地址
	else
		write_com(0x80|(add-1)+0x40);   //1602写第二行的地址
	while(1)
	{
		if(*p == '\0')  break;	//\0字符串的结尾标志  break结束while循环 结束写字符
		write_data(*p);	    //写数据
		p++;	            //指针地址加1
	}
}


/***********************lcd1602上显示2位十进制数************************/
void write_lcd2(uchar hang,uchar add,uint date)
{
	if(hang==1)
		write_com(0x80|(add-1));	   //1602写第一行的地址
	else
		write_com(0x80|(add-1)+0x40);  //1602写第二行的地址
	write_data(0x30+date/10%10);   //显示十位数
	write_data(0x30+date%10);	   //显示个位数
}



/***********************lcd1602上显示特定的字符****0XDF 度********************/
void write_zifu(uchar hang,uchar add,uchar date)
{
	if(hang==1)
		write_com(0x80|(add-1));	   //1602写第一行的地址
	else
		write_com(0x80|(add-1)+0x40);  //1602写第二行的地址
	write_data(date);		  //写数据
}

/***********************lcd1602初始化设置************************/
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
/*	write_string(1, 1,"Y:   N");//压力
    write_string(1, 8,"T:");//温度
    write_zifu(1,13,0xdf);
	write_zifu(1,14,'C'); 
	write_string(2,1,"00:00");
	write_com(0x0c);
	write_com(0x01);	*/
}


/***********************18b20初始化函数*****************************/
void init_18b20()
{
	bit q;
	DSPORT = 1;				//把总线拿高
	delay_uint(8);
	DSPORT = 0;				//给复位脉冲  再将数据线从高拉低，要求保持480~960us
	delay_uint(80);
	DSPORT = 1;				//把总线拿高 等待
	delay_uint(8);
	q = DSPORT;				//读取18b20初始化信号
	delay_uint(4);
	DSPORT = 1;				//把总线拿高 释放总线
}


/*************写18b20内的数据***************/
void write_18b20(uchar dat)
{
	uchar i;
	for(i=0;i<8;i++)	 //单总线写8位的数据需要循环8次才能写完
	{					 //写数据是低位开始
		DSPORT = 0;			 //把总线拿低写时间隙开始
		DSPORT = dat & 0x01; //向18b20总线写一位数
		delay_uint(4);
		DSPORT = 1;			 //释放总线
		dat >>= 1;		//将dat中的各二进制位数据右移1位
	}
	delay_uint(4);	//稍作延时,给硬件一点反应时间
}

/*************读取18b20内的数据***************/

uchar read_18b20()
{
	uchar byte,bi;
	uint i,j;	
	for(j=8;j>0;j--)
	{
		DSPORT=0;//先将总线拉低1us
		i++;
		DSPORT=1;//然后释放总线
		i++;
		i++;//延时6us等待数据稳定
		bi=DSPORT;	 //读取数据，从最低位开始读取
		/*将byte左移一位，然后与上右移7位后的bi，注意移动之后移掉那位补0。*/
		byte=(byte>>1)|(bi<<7);						  
		i=4;		//读取完之后等待48us再接着读取下一个数
		while(i--);
	}				
	return byte;
}

/*************读取温度的值 读出来的是小数***************/
int read_temp()
{
	int value;
	uchar low,high;			   //在读取温度的时候如果中断的太频繁了，就应该把中断给关了，否则会影响到18b20的时序
	init_18b20();		   //初始化18b20
	write_18b20(0xcc);	   //跳过64位ROM
	write_18b20(0x44);	   //启动一次温度转换命令
	delay_uint(50);		   //转换一次温度需要时间 延时一下

	init_18b20();		   //初始化18b20

	write_18b20(0xcc);	   //跳过64位ROM
	write_18b20(0xbe);	   //发出读取暂存器命令

 	low = read_18b20();	   //读温度低字节
	high = read_18b20();  //读温度高字节
	value = high;
 	value <<= 8;		   //把温度的高位左移8位
	value |= low;		   //把读出的温度低位放到value的低八位中
	//value *= 0.625;	       //转换到温度值 小数
	return value;		   //返回读出的温度 带小数
}

/*****************0832转换成数字量*****************/
uchar ADC_read_data(uchar ch)
{
	uchar i,dat0=0,dat1=0;
	cs=0;
	clk=0;
	dio=1;
	delay_us();
	clk=1;
	delay_us();	  	//第一次下降沿之前DIO置高，起始信号

	clk=0;
	dio=1;
	delay_us();
	clk=1;		 //第二次下降沿输入dio=1
	delay_us();

	clk=0;
	dio=ch; //第三个下降沿,设DIO=0;
	delay_us();
	clk=1;
	delay_us();

/*********二，三个下降沿选择通道1************************/

	clk=0;
	dio=1;  //第四个下降沿之前，设DIO=1
	delay_us();


	for(i=0;i<8;i++)//第4~11共8个下降沿读数据（MSB->LSB）
	{
		clk=1;
		delay_us();
		clk=0;
		delay_us();
		dat0=dat0<<1|dio;
	}
	for(i=0;i<8;i++) //第11~18共8个下降沿读数据（LSB->MSB）
	{
		dat1=dat1|((uchar)(dio)<<i);
		clk=1;
		delay_us();
		clk=0;
		delay_us();
	}
	cs=1;				  //判断dat0与dat1是否相等
	return (dat0==dat1)?dat0:0;
}

/*************定时器0初始化程序***************/
void time_init()
{
	EA   = 1;	 	  //开总中断
	TMOD = 0X01;	  //定时器0、工作方式1
	ET0  = 1;		  //开定时器0中断
	TR0  = 1;		  //允许定时器0定时
	TH0 = 0x3c;
	TL0 = 0xb0;     // 定时50ms中断一次
}

/********************独立按键程序*****************/
uchar key_can;	 //按键值

void key()	 //独立按键程序
{
	key_can = 0;                //按键值还原成0
 	if(key1 == 0 || key2 == 0 || key3 == 0)		//有按键按下
	{
		delay_1ms(1);	     	//按键延时消抖动
		if(key1 == 0)		//确认是按键按下
			key_can = 3; 	//得到按键值
		if(key2 == 0)		//确认是按键按下
			key_can = 2; 	//得到按键值
		if(key3 == 0)		//确认是按键按下
			key_can = 1; 	//得到按键值
		flag_lj_en ++;   //连加变量加1
	}
	else
		flag_lj_en = 0;		   //关闭连加使能
}

/****************按键显示函数***************/
void key_with()
{
	if(menu_1 == 0)
	{
		if(key_can == 3)
			relay2 = ~relay2;//打开关闭加沫继电器
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
                flag_ds = 1;    //定时模式开
            }
            if(menu_2 == 2)
            {
                flag_ds = 0;   //定时模式关
            }
        }
	}

 	if(key_can == 1)	   //设置键
	{
		menu_1 ++;
		if(menu_1 > 2)
		{
			menu_1 = 0;		 //menu_1 = 0 退出设置了，在正常显示界面下
			Init_1602();     //1602初始化
		}
 	}
	if(menu_1 == 1)			//设置时钟
	{
 		if(key_can == 2)	 //加键
		{
			if(flag_lj_en <= 3)
				shi ++ ;		//按键按下未松开加1 加三次
			else
				shi += 5;   	//按键按下未松开加三次后加5
			if(shi > 23)
				shi = 0;
		}
		if(key_can == 3)	   //减键
		{
			if(flag_lj_en <= 3)
			{
				if(shi == 0)
					shi = 24;
				shi -- ;			//按键按下未松开减1 减三次
			}
			else
			{
				if(shi < 5)
					shi = 28;
				shi -= 5;		   	//按键按下未松开减三次后减5
			}
 		}
 		write_lcd2(2,1,shi);  			 //显示时
		write_com(0xc0+0);              //将光标移动到第1行第10位
		write_com(0x0f);                 //显示光标并且闪烁
	}
	if(menu_1 == 2)			//设置分钟
	{
  		if(key_can == 2)	 //加键
		{
			if(flag_lj_en <= 3)
				fen ++ ;		//按键按下未松开加1 加三次
			else
				fen += 5;   	//按键按下未松开加三次后加5
			if(fen > 60)
				fen = 0;
		}
		if(key_can == 3)	   //减键
		{
			if(flag_lj_en <= 3)
			{
				if(fen == 0)
					fen = 60;
				fen -- ;			//按键按下未松开减1 减三次
			}
			else
			{
				if(fen < 5)
					fen = 65;
				fen -= 5;		   	//按键按下未松开减三次后减5
			}
 		}
 		write_lcd2(2,4,fen);		 //显示分钟
		write_com(0xc0+3);          //将光标移动到第1行第13位
		write_com(0x0f);             //显示光标并且闪烁
	}
	delay_1ms(400);
}



/************压力转换*************/
void Yali_()
{
    vvv=ADC_read_data(1);
    Tmp=vvv*1000.0/255.0;//转换
    yali=Tmp;
}
/*******************************/

/****************控制继电器函数***************/

void clock_h_l()
{
	//uchar value;
	Yali_();
	if(flag_ds == 0)
	{
		if(wendu <= t_low||yali <= 10)  //温度小于等于温度温度下限
		{
			relay2 = 0;	     //继电器吸合工作
			beep = 0;     //蜂鸣器叫提示
			delay_1ms(1500);
			beep = 1;   //关闭蜂鸣器
		}
		if(wendu >= t_high||yali >= 30)  //温度大于等于温度温度上限
        {
            relay2 = 1;	     //关闭继电器
            delay_1ms(1500);
        }
	}
}		

/****************定时控制***************/


void dingshi_dis()  //定时控制
{
	uchar value;
	uchar a,b;
	a = shi;
	b = fen;
	if((shi == 0) && (fen == 0))   //定时时间为0时退出定时模式
		flag_ds = 0;
	if(flag_ds == 1)
	{
		  //定时模式
		value ++;
		if(value == 2)
			write_zifu(2,3,' ');
		if(value >= 4)	  //1S
		{
			write_zifu(2,3,':');
			value = 0;
			if(fen + shi > 0)   //开始计时
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

					if((fen == 0) && (shi == 0))	 //定时时间到
					{
						//relay1 = 0; // 打开加热继电器
						//relay2=0;//打开加泡沫继电器
						relay2=~relay2;
						beep = 0;     //蜂鸣器叫提示
						delay_1ms(1500);
						beep = 1;   //关闭蜂鸣器
						shi = a;
						fen = b;
					}
 				}
				write_lcd2(2,4,fen);	 //显示分钟
				write_lcd2(2,1,shi);    //显示时钟
			}
		}
	}
}



/*************定时器0中断服务程序***************/

void time0() interrupt 1
{
	TH0 = 0x4C;	 //设置初始值
    TL0 = 0x00; 
	Time++;
}

/******************串口初始化*********************/

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

/***************串口发一个数据******************/

void send_uart(uchar ch)
{
    SBUF = ch;
    while(!TI);
    TI = 0;
}

/***************串口发一串数据******************/

void send_uart0_dat(char *s)
{
    while(*s)
    {
        send_uart(*s++);
    }
}




/****************温度显示发送***************/
void LcdDisplay(int temp)
{
    float tp;
	if(temp< 0)				//当温度值为负数
  	{
		//因为读取的温度是实际温度的补码，所以减1，再取反求出原码
		temp=temp-1;
		temp=~temp;
		tp=temp;
		temp=tp*0.0625*10+0.5;
  	}
 	else
  	{
		tp=temp;//因为数据处理有小数点所以将温度赋给一个浮点型变量
		//如果温度是正的那么，那么正数的原码就是补码它本身
		temp=tp*0.0625*10+0.5;
	}
	wendu=temp;

	write_com(0x80+9);		  //写地址 80表示初始地址
	write_data('0'+wendu/100%10); //十位
	write_data('0'+wendu/10%10); //个位
	write_data('.'); 		//显示 ‘.’
	write_data('0'+wendu%10); //显示小数点
	//write_data(0xdf);
	//write_data('C');

	send_uart0_dat("Temp: ");
	send_uart('0'+wendu/100%10); //十位
	send_uart('0'+wendu/10%10); //个位
	send_uart('.'); 		//显示 ‘.’
	send_uart('0'+wendu%10); //显示小数点
	send_uart('C');
	send_uart('\r');
	send_uart('\n');

	send_uart('\r');
	send_uart('\n');
}


/*************压力显示发送*************/
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

/****************主函数***************/

void main()
{
	//beep = 0;		                //蜂鸣器响一声
	delay_1ms(150);
	P0 = P1 = P2 = P3 = 0xff;		//所有单片机IO口输出高电平
	Init_1602();                    //1602初始化
	time_init();                    //初始化定时器
	init_uart();	//串口初始化
	write_com(0x01);
	write_string(1, 1,"Y:   N");//压力
    write_string(1, 8,"T:     ");//温度
    write_zifu(1,14,0xdf);
    write_zifu(1,15,'C');
    write_string(2,1,"00:00");
	while(1)
	{
	    
 		//while(state==0){
        if(Time>=20)	        //250ms 处理一次温度程序
        {
           Time=0;
           LcdDisplay_Y();
           LcdDisplay(read_temp());
           //clock_h_l();                //报警函数
           dingshi_dis();  //定时控制
         }
 		//}
		key();					//按键程序
		
 		key_with();			    //设置温度
 	}
}
