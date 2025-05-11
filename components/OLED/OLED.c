#include "OLED.h"
#define OLED_W_SCL(a) gpio_set_level_mod(SCL, (a))
#define OLED_W_SDA(a) gpio_set_level_mod(SDA, (a))

static void OLED_I2CHW_Init()
{
	gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = 1ULL<<SCL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = 1ULL<<SDA;
	gpio_config(&io_conf);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

static void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	esp_rom_delay_us(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

static void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	esp_rom_delay_us(1);
	OLED_W_SDA(1);
}

static void OLED_I2C_SendByte(uint8_t Byte)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		esp_rom_delay_us(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);
	esp_rom_delay_us(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

static void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}
static void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
static void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

void OLED_Clear(void)
{  
	for (uint8_t j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(uint8_t i = 0; i < 128; i++)
			OLED_WriteData(0x00);
	}
}

void OLED_Point(uint8_t x, uint8_t data)
{
	static const uint8_t mask_table[8] = {
        0x00, 0x01, 0x03, 0x07,
        0x0F, 0x1F, 0x3F, 0x7F
    };
	static uint8_t buffer[8] = {0};

	for(int8_t i = 0; i < 8; i++)
    {
        buffer[i] = ~((data >= 8) ? 0xFF : mask_table[data]); //data
        data = (data > 8) ? (data - 8) : 0; //data > 8 : data - 8; data <= 8: 0
	}

	for(uint8_t i = 0; i < 8; i++)
	{
		OLED_SetCursor(i, x);
		OLED_WriteData(buffer[i]);
	}
}

void OLED_Init(void)
{
	vTaskDelay(10);//延时100ms
	OLED_I2CHW_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xAF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}