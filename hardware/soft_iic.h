#ifndef __SOFT_IIC_H__
#define __SOFT_IIC_H__

#include "stm32f4xx.h"
#include "systick.h"
#define SCL_GPIO GPIOB
#define SCL_PIN GPIO_Pin_8
#define SDA_GPIO GPIOB
#define SDA_PIN GPIO_Pin_9

#define SCL_HIGH GPIO_SetBits(SCL_GPIO, SCL_PIN)
#define SCL_LOW GPIO_ResetBits(SCL_GPIO, SCL_PIN)
#define SCL_OUT(val) do { if(val) SCL_GPIO->BSRRL = SCL_PIN; else SCL_GPIO->BSRRH = SCL_PIN; } while(0)

#define SDA_HIGH GPIO_SetBits(SDA_GPIO, SDA_PIN)
#define SDA_LOW GPIO_ResetBits(SDA_GPIO, SDA_PIN)
#define SDA_OUT(val) do { if(val) SDA_GPIO->BSRRL = SDA_PIN; else SDA_GPIO->BSRRH = SDA_PIN; } while(0)

#define SDA_IN ((SDA_GPIO->IDR & SDA_PIN) ? 1 : 0)

# define USE_STD_400KHZ_TIMING 1 //是否使用标准400kHz时序,0:不使用,1:使用

void soft_iic_init(void);

static inline void soft_iic_IO_mode(GPIOMode_TypeDef mode){
	static GPIO_InitTypeDef GPIO_InitStructure;
	//PE6 TRIG
	GPIO_InitStructure.GPIO_Mode = mode;//
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin = SDA_PIN;//SDA引脚
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上拉下拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//2MHz
	GPIO_Init(SDA_GPIO,&GPIO_InitStructure);
}
static inline void soft_iic_idle(void){
    SCL_HIGH;
    SDA_HIGH;
    delay_us(2);
}
#if defined(USE_STD_400KHZ_TIMING) && (USE_STD_400KHZ_TIMING != 1)
//发送模式
static inline void soft_iic_start(void){
    //SCL高电平期间，SDA由高变低，表示开始传输
    soft_iic_IO_mode(GPIO_Mode_OUT);
    SCL_OUT(1);
    SDA_OUT(1);
    delay_us(5);

    SDA_OUT(0);
    delay_us(5);
    SCL_OUT(0);
    //发完START，此时SCL=0，SDA=0
}

static inline void soft_iic_send_addr(uint8_t addr_direction, uint8_t addr){
    soft_iic_IO_mode(GPIO_Mode_OUT);
    addr = (addr << 1) | addr_direction;
    SCL_OUT(0);
    delay_100ns(10);
    for(int i=7; i>=0; i--){    //高7位地址先发送
        SDA_OUT(addr & (1 << i)); 
        delay_100ns(10);
        SCL_OUT(1);
        delay_100ns(10);    //1.2μs，保持SCL高电平，传输数据位时间
        SCL_OUT(0);
    }
    //发完地址，此时SCL=0，SDA=0（向从机发数据）/1（收从机数据）
}
static inline void soft_iic_send_byte(uint8_t byte){
    soft_iic_IO_mode(GPIO_Mode_OUT);
    SCL_OUT(0);
    delay_100ns(10);
    for(int i=7; i>=0; i--){    //高7位数据先发送
        SDA_OUT(byte & (1 << i)); 
        delay_100ns(10);
        SCL_OUT(1);
        delay_100ns(10);    //1.2μs，保持SCL高电平，传输数据位时间
        SCL_OUT(0);
    }
}
static inline uint8_t soft_iic_recv_ack(void){
    uint8_t ack = 2;
    soft_iic_IO_mode(GPIO_Mode_IN);
    //等待应答，需先释放SDA总线：SDA=1
    SCL_OUT(1);   //信号高电平期间，
    delay_100ns(10);
    ack = SDA_IN;   //读取应答信号
    SCL_OUT(0);
    delay_100ns(10);
    return ack;//0:应答，1:不应答，2:未知错误
    //收完ACK，此时SCL=0，SDA=0（应答）或1（无响应）
}


static inline void soft_iic_stop(void){
    //SCL高电平期间，SDA由低变高，表示结束传输
    soft_iic_IO_mode(GPIO_Mode_OUT);
    SCL_OUT(0);
    SDA_OUT(0);
    delay_100ns(10);
    SCL_OUT(1);
    delay_100ns(10);
    SDA_OUT(1);
    //发完STOP，此时SCL=1，SDA=1
}

//接收模式
static inline uint8_t soft_iic_recv_byte(void){
    uint8_t byte = 0;
    //此时SDA=0（从机应答），SCL = 0
    soft_iic_IO_mode(GPIO_Mode_IN);
    SCL_OUT(0);
    for(int i = 7; i >= 0; i--){
        delay_100ns(10);
        SCL_OUT(1);
        byte |= (SDA_IN << i);
        delay_100ns(10);
        SCL_OUT(0);
    }
    return byte;
}

static inline void soft_iic_ack(void){
    //SCL高电平期间，SDA保持低电平，表示应答
    soft_iic_IO_mode(GPIO_Mode_OUT);
    SCL_OUT(0);
    delay_100ns(10);
    SDA_OUT(0);
    delay_100ns(10);
    SCL_OUT(1);
    delay_100ns(10);
    SCL_OUT(0);

}

static inline void soft_iic_nack(void){
    soft_iic_IO_mode(GPIO_Mode_OUT);
    SCL_OUT(0);
    delay_100ns(10);
    SDA_OUT(1);
    delay_100ns(10);
    SCL_OUT(1);
    delay_100ns(10);
    SCL_OUT(0);
}
#else
/************************ 延时宏定义 ************************/
// 400kHz 标准延时 (delay_100ns(1) = 100ns)
#define I2C_DELAY_SU_DAT  delay_100ns(1)   // 100ns 数据建立时间
#define I2C_DELAY_HD_DAT  delay_100ns(3)   // 300ns 数据保持时间
#define I2C_DELAY_HIGH    delay_100ns(6)   // 600ns SCL高电平
#define I2C_DELAY_LOW     delay_100ns(13)  // 1.3μs SCL低电平
#define I2C_DELAY_STA     delay_100ns(6)   // 600ns 起始保持
#define I2C_DELAY_STO     delay_100ns(6)   // 600ns 停止建立
#define I2C_DELAY_BUF     delay_100ns(13)  // 1.3μs 总线空闲

/************************ 标准400kHz I2C函数 ************************/
/**
 * @brief  I2C起始信号 (标准400kHz)
 * @note   空闲: SCL=1, SDA=1 → SDA拉低 → 保持≥0.6μs → SCL拉低
 */
static inline void soft_iic_start(void)
{
    soft_iic_IO_mode(GPIO_Mode_OUT);
    // 总线空闲状态
    SCL_OUT(1);
    SDA_OUT(1);
    I2C_DELAY_BUF;  // 总线空闲延时 ≥1.3μs

    // 起始信号核心
    SDA_OUT(0);
    I2C_DELAY_STA;  // 起始保持时间 ≥0.6μs
    SCL_OUT(0);
}

/**
 * @brief  发送从机地址(7位) + 读写位
 * @param  addr_direction: 0=写, 1=读
 * @param  addr: 7位从机地址
 */
static inline void soft_iic_send_addr(uint8_t addr_direction, uint8_t addr)
{
    soft_iic_IO_mode(GPIO_Mode_OUT);
    // 拼接8位地址帧
    addr = (addr << 1) | addr_direction;

    I2C_DELAY_HD_DAT;  // 数据保持时间 ≥300ns
    for(int i=7; i>=0; i--)
    {
        // SCL低电平期间改变SDA
        SDA_OUT( (addr >> i) & 0x01 );
        I2C_DELAY_SU_DAT;  // 数据建立时间 ≥100ns

        // SCL高电平采样
        SCL_OUT(1);
        I2C_DELAY_HIGH;    // SCL高电平 ≥0.6μs
        SCL_OUT(0);
        I2C_DELAY_LOW;     // SCL低电平 ≥1.3μs
    }
}

/**
 * @brief  发送1字节数据 (标准400kHz)
 */
static inline void soft_iic_send_byte(uint8_t byte)
{
    soft_iic_IO_mode(GPIO_Mode_OUT);
    I2C_DELAY_HD_DAT;  // 数据保持时间 ≥300ns

    for(int i=7; i>=0; i--)
    {
        // SCL低改SDA
        SDA_OUT( (byte >> i) & 0x01 );
        I2C_DELAY_SU_DAT;

        // SCL高采样
        SCL_OUT(1);
        I2C_DELAY_HIGH;
        SCL_OUT(0);
        I2C_DELAY_LOW;
    }
}

/**
 * @brief  接收从机ACK应答 (标准400kHz)
 * @retval 0=ACK, 1=NACK
 */
static inline uint8_t soft_iic_recv_ack(void)
{
    uint8_t ack = 1;
    // 释放SDA总线 (输入模式)
    soft_iic_IO_mode(GPIO_Mode_IN);
    SDA_OUT(1);  // 开漏总线释放

    // 第9个时钟
    I2C_DELAY_HD_DAT;
    SCL_OUT(1);
    I2C_DELAY_HIGH;
    // 读取SDA电平
    ack = SDA_IN;
    SCL_OUT(0);
    I2C_DELAY_LOW;

    return ack;
}

/**
 * @brief  I2C停止信号 (标准400kHz)
 * @note   SCL=1时, SDA从低→高
 */
static inline void soft_iic_stop(void)
{
    soft_iic_IO_mode(GPIO_Mode_OUT);
    // 前提: SCL=0, SDA=0
    SCL_OUT(0);
    SDA_OUT(0);
    I2C_DELAY_HD_DAT;

    // 停止信号核心
    SCL_OUT(1);
    I2C_DELAY_HIGH;
    SDA_OUT(1);
    I2C_DELAY_STO;  // 停止建立时间 ≥0.6μs

    // 恢复总线空闲
    I2C_DELAY_BUF;
}

/**
 * @brief  主机接收1字节数据 (标准400kHz)
 */
static inline uint8_t soft_iic_recv_byte(void)
{
    uint8_t byte = 0;
    soft_iic_IO_mode(GPIO_Mode_IN);
    SDA_OUT(1);  // 释放总线

    for(int i=7; i>=0; i--)
    {
        I2C_DELAY_LOW;
        SCL_OUT(1);
        I2C_DELAY_HIGH;
        // SCL高电平读取数据
        byte |= (SDA_IN << i);
        SCL_OUT(0);
    }
    I2C_DELAY_HD_DAT;
    return byte;
}

/**
 * @brief  主机发送ACK (应答)
 */
static inline void soft_iic_ack(void)
{
    soft_iic_IO_mode(GPIO_Mode_OUT);
    SCL_OUT(0);
    I2C_DELAY_HD_DAT;

    SDA_OUT(0);  // ACK=低电平
    I2C_DELAY_SU_DAT;
    SCL_OUT(1);
    I2C_DELAY_HIGH;
    SCL_OUT(0);
    I2C_DELAY_LOW;
}

/**
 * @brief  主机发送NACK (非应答)
 */
static inline void soft_iic_nack(void)
{
    soft_iic_IO_mode(GPIO_Mode_OUT);
    SCL_OUT(0);
    I2C_DELAY_HD_DAT;

    SDA_OUT(1);  // NACK=高电平
    I2C_DELAY_SU_DAT;
    SCL_OUT(1);
    I2C_DELAY_HIGH;
    SCL_OUT(0);
    I2C_DELAY_LOW;
}
#endif


#endif




