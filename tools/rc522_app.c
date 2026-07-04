#include "rc522_app.h"
#include "RC522.h"
#include "beep.h"
#include "stdio.h"
#include "stm32f4xx.h"

// 卡片信息结构体
typedef struct {
    uint8_t card_serial[4];  // 卡序列号
    uint8_t card_type[2];    // 卡类型
    uint8_t is_registered;   // 是否已注册
} CardInfo_t;

// 存储已注册的卡片信息（实际项目中应该存储在 Flash 或 EEPROM）
#define MAX_CARDS 10
CardInfo_t registered_cards[MAX_CARDS];
uint8_t card_count = 0;

// 默认密钥（Mifare 卡默认密钥）
uint8_t default_key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// 数据块地址（示例：使用第 2 扇区的块 1）
#define DATA_BLOCK_ADDR 5

/**
 * @brief  检查卡片是否已注册
 * @param  serial: 卡序列号指针
 * @retval 1: 已注册, 0: 未注册
 */
uint8_t IsCardRegistered(uint8_t* serial) {
    uint8_t i, j;

    for (i = 0; i < card_count; i++) {
        uint8_t match = 1;
        for (j = 0; j < 4; j++) {
            if (registered_cards[i].card_serial[j] != serial[j]) {
                match = 0;
                break;
            }
        }
        if (match)
            return 1;
    }
    return 0;
}

/**
 * @brief  注册新卡片，注册到单片机内存
 * @param  serial: 卡序列号
 * @param  type: 卡类型
 * @retval MI_OK: 注册成功, MI_ERR: 注册失败
 */
uint8_t RegisterCard(uint8_t* serial, uint8_t* type) {
    if (card_count >= MAX_CARDS) {
        printf("卡片存储已满!\r\n");
        return MI_ERR;
    }

    // 检查是否已注册
    if (IsCardRegistered(serial)) {
        printf("该卡片已注册!\r\n");
        return MI_ERR;
    }

    // 保存卡片信息
    uint8_t i;
    for (i = 0; i < 4; i++)
        registered_cards[card_count].card_serial[i] = serial[i];

    registered_cards[card_count].card_type[0] = type[0];
    registered_cards[card_count].card_type[1] = type[1];
    registered_cards[card_count].is_registered = 1;
    card_count++;

    printf("卡片注册成功! 序列号: %02X %02X %02X %02X\r\n", serial[0], serial[1], serial[2], serial[3]);

    return MI_OK;
}

/**
 * @brief  验证卡片（读取卡片数据并验证）
 * @param  serial: 卡序列号
 * @retval MI_OK: 验证成功, MI_ERR: 验证失败
 */
uint8_t VerifyCard(uint8_t* serial) {
    uint8_t status;
    uint8_t read_data[18];
    uint8_t i;

    // 验证密钥 A
    status = MFRC522_Auth(PICC_AUTHENT1A, DATA_BLOCK_ADDR, default_key, serial);
    if (status != MI_OK) {
        printf("密钥验证失败!\r\n");
        return MI_ERR;
    }

    // 读取数据块, 16字节,传入块地址（50卡一共64块，每块16字节，第4i块为控制块（i=1,2,3,4）存储密码a（6B），存取控制字（4B），密码b）
    status = MFRC522_Read(DATA_BLOCK_ADDR, read_data);
    if (status != MI_OK) {
        printf("读取数据失败!\r\n");
        return MI_ERR;
    }

    // 验证数据（示例：检查第一个字节是否为特定标记）
    if (read_data[0] == 0xAA)  // 假设 0xAA 是注册标记
    {
        printf("卡片验证成功!\r\n");
        printf("读取数据: ");
        for (i = 0; i < 16; i++)
            printf("%02X ", read_data[i]);
        printf("\r\n");
        return MI_OK;
    } else {
        printf("卡片数据验证失败!\r\n");
        return MI_ERR;
    }
}

/**
 * @brief  写入注册标记到卡片
 * @param  serial: 卡序列号
 * @retval MI_OK: 写入成功, MI_ERR: 写入失败
 */
uint8_t WriteRegistrationMark(uint8_t* serial) {
    uint8_t status;
    uint8_t write_data[16];
    uint8_t i;

    // 初始化写入数据
    for (i = 0; i < 16; i++)
        write_data[i] = 0x00;

    // 设置注册标记
    write_data[0] = 0xAA;  // 注册标记
    write_data[1] = 0x01;  // 版本号
    // 可以在这里写入其他自定义数据

    // 验证密钥 A
    status = MFRC522_Auth(PICC_AUTHENT1A, DATA_BLOCK_ADDR, default_key, serial);
    if (status != MI_OK) {
        printf("密钥验证失败!\r\n");
        return MI_ERR;
    }

    // 写入数据块
    status = MFRC522_Write(DATA_BLOCK_ADDR, write_data);
    if (status != MI_OK) {
        printf("写入数据失败!\r\n");
        return MI_ERR;
    }

    printf("注册标记写入成功!\r\n");
    return MI_OK;
}

/**
 * @brief  完整的卡片注册流程
 * @retval 无
 */
uint8_t CardRegistrationProcess(void) {
    uint8_t status = MI_ERR;
    uint8_t card_type[2];
    uint8_t card_serial[5];

    // 寻卡
    int32_t timeout = 400;
    while (timeout > 0 && status != MI_OK) {
        status = MFRC522_Request(PICC_REQIDL, card_type);
        timeout--;
        // delay_us(1);
        timeout % 40 == 0 ? printf("timeout: %d \r\n", timeout / 40) : 0;
    }
    if (status != MI_OK) {
        printf("没有检测到卡片, 验证超时！ \r\n");
        return MI_ERR;  // 没有卡片
    }
    BEEP1_ON
    delay_ms(100);
    BEEP1_OFF
    delay_ms(1000);
    printf("检测到卡片，类型: %02X %02X\r\n", card_type[0], card_type[1]);

    // 防冲突，获取卡序列号
    status = MFRC522_Anticoll(card_serial);
    if (status != MI_OK)
        return MI_ERR;

    printf("卡序列号: %02X %02X %02X %02X\r\n", card_serial[0], card_serial[1], card_serial[2], card_serial[3]);

    // 选卡
    MFRC522_SelectTag(card_serial);

    // 检查是否已注册
    if (!IsCardRegistered(card_serial)) {
        // 写入注册标记
        if (WriteRegistrationMark(card_serial) == MI_OK) {
            BEEP1_ON
            delay_ms(100);
            BEEP1_OFF
            delay_ms(100);
            BEEP1_ON
            delay_ms(100);
            BEEP1_OFF
            // 注册卡片
            uint8_t ret = RegisterCard(card_serial, card_type);
        }
    } else {
        printf("卡片已注册，跳过注册流程\r\n");
    }

    // 卡片休眠
    MFRC522_Halt();
}

/**
 * @brief  完整的卡片验证流程
 * @retval 无
 */
uint8_t CardVerificationProcess(void) {
    uint8_t status = MI_ERR;
    uint8_t card_type[2];
    uint8_t card_serial[5];

    // 寻卡
    int32_t timeout = 400;
    while (timeout > 0 && status != MI_OK) {
        status = MFRC522_Request(PICC_REQIDL, card_type);
        timeout--;
        // delay_us(1);
        timeout % 40 == 0 ? printf("timeout: %d \r\n", timeout / 40) : 0;
    }
    if (status != MI_OK) {
        printf("没有检测到卡片, 验证超时！ \r\n");
        return MI_ERR;  // 没有卡片
    }
    BEEP1_ON
    delay_ms(100);
    BEEP1_OFF
    delay_ms(1000);
    printf("检测到卡片，类型: %02X %02X\r\n", card_type[0], card_type[1]);

    // 防冲突，获取卡序列号
    status = MFRC522_Anticoll(card_serial);
    if (status != MI_OK)
        return MI_ERR;

    // 选卡
    MFRC522_SelectTag(card_serial);

    // 验证卡片
    if (VerifyCard(card_serial) == MI_OK) {
        printf("验证通过，允许访问!\r\n");
        // 在这里执行允许访问的操作
        BEEP1_ON
        delay_ms(300);
        BEEP1_OFF
        delay_ms(100);
        BEEP1_ON
        delay_ms(300);
        BEEP1_OFF
    } else {
        printf("验证失败，拒绝访问!\r\n");
        // 在这里执行拒绝访问的操作
    }

    // 卡片休眠
    MFRC522_Halt();
}

/**
 * @brief  主循环示例
 * @retval 无
 */
void RC522_Main_app(uint8_t mode) {
    // 0: 验证模式, 1: 注册模式

    // 初始化 RC522
    MF522_Init();
    // MFRC522_Init();

    printf("RC522 初始化完成 \r\n");
    // 检测模式切换按钮（需要根据实际硬件实现）
    // if (ButtonPressed()) mode = !mode;

    if (mode == 1) {
        printf("=== 注册RFID卡片 ===\r\n");
        CardRegistrationProcess();
    } else {
        printf("=== 验证RFID卡片 ===\r\n");
        CardVerificationProcess();
    }

    delay_ms(500);  // 延时 500ms
}