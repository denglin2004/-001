#include  "I2C_soft.h"

static void Init(void);
static void Start(void);
static void Stop(void);
static ack_t SendData(uint8_t);
static uint8_t ReceiveData(ack_t);

I2C_soft_t I2C_Soft =
{
	Init,
	Start,
	Stop,
	SendData,
	ReceiveData,
};

static void Init(void)
{
    //声明结构体
     GPIO_InitTypeDef GPIO_InitStructure = {0};

     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);    //使能APB2 GPIO外设时钟

     GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN1| I2C_SDA_PIN1;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;         //设置为开漏输出模式
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_Init(I2C_PORT, &GPIO_InitStructure);

     I2C_SET_SCL();
     I2C_SET_SDA();
}

static void Start(void)
{
	I2C_SET_SDA();
	I2C_SET_SCL();
	I2C_SOFT_GLOBAL_DELAY();
	
	I2C_RESET_SDA();
	I2C_SOFT_GLOBAL_DELAY();
	
	I2C_RESET_SCL();
	I2C_SOFT_GLOBAL_DELAY();
}

static void Stop(void)
{
	I2C_RESET_SDA();
	I2C_SET_SCL();
	I2C_SOFT_GLOBAL_DELAY();

	I2C_SET_SDA();
}

static ack_t SendData(uint8_t _data)
{
	uint8_t i;
	ack_t  ack;
	
	for(i = 0;i < 8;i++)
	{
		I2C_RESET_SCL();
		I2C_SOFT_GLOBAL_DELAY();

		if(_data & 0x80)I2C_SET_SDA();
		else I2C_RESET_SDA();
		I2C_SOFT_GLOBAL_DELAY();

		I2C_SET_SCL();
		I2C_SOFT_GLOBAL_DELAY();

		_data <<= 1;
	}
	
	I2C_RESET_SCL();
	I2C_SET_SDA();//释放SDA，等待从机应答
	I2C_SOFT_GLOBAL_DELAY();
	
	I2C_SET_SCL();
	I2C_SOFT_GLOBAL_DELAY();
	
	ack = (ack_t)I2C_RD_SDA();
	
	I2C_RESET_SCL();
	I2C_SOFT_GLOBAL_DELAY();
	
	//返回从机的应答信号
	return ack;
}

static uint8_t ReceiveData(ack_t ack)
{
	uint8_t _data = 0,i;

	for(i = 0;i < 8;i++)
	{
	    _data <<= 1;

		I2C_RESET_SCL();//SCL清零，从机准备数据
		I2C_SOFT_GLOBAL_DELAY();
		
		I2C_SET_SCL();//SCL置高，读取SDA总线数据
		I2C_SOFT_GLOBAL_DELAY();

		_data |= I2C_RD_SDA();
	}
	
	I2C_RESET_SCL();//SCL清零，主机准备应答信号
	I2C_SOFT_GLOBAL_DELAY();
	
	//主机发送应答信号	
	if(ack == ACK)I2C_RESET_SDA();
	else I2C_SET_SDA();
	I2C_SOFT_GLOBAL_DELAY();
	
	I2C_SET_SCL();
	I2C_SOFT_GLOBAL_DELAY();
	
	//SCL先清零，再释放SDA，防止连续传输数据时，从机错将SDA释放信号当成NACk信号
	I2C_RESET_SCL();
	I2C_SET_SDA();
	I2C_SOFT_GLOBAL_DELAY();

	return _data;
}
