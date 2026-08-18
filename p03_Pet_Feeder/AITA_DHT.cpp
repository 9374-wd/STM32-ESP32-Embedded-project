
#include "AITA_DHT.h"

/* Private functions ---------------------------------------------- */
//DHT11初始化函数
uint8_t dht11_init(void) {
    uint8_t errcode = SUCC;
    uint8_t i = 0;

    IO_OUTMODE     //设置IO输出
    IO_SET         //IO输出1
    DELAY_MS(2);   //延时2ms，为下降沿做准备
    IO_CLR         //IO输出0，产生下降沿

    DELAY_MS(20);  //延时20ms，主机下降沿并保持，18ms以上的低电平启动DHT11
    IO_SET         //再次拉高IO，等待DHT11拉低以作应答

    IO_INMODE      //设置IO输入
    DELAY_US(40);  //延时40us，等待DHT11输入的低电平应答信号

    //等待DHT11拉低IO，最高持续MAXCOUNT个us
    while(READ_IO==1 && i<MAXCOUNT) {
        i++;
        DELAY_US(1);   
    }
    if(i>=MAXCOUNT) {
        errcode = INITTOUT;
        return errcode;
    } else i = 0;
    //DHT11拉低IO持续80us，后会在拉高IO持续80us作为完整的应答
    while (READ_IO==0 && i<MAXCOUNT) {
        i++;
        DELAY_US(1);
    }
    if(i>=MAXCOUNT) {
        errcode = INITTOUT;
        return errcode;
    }
    return errcode;
}
//读取一个bit函数
uint8_t dht11_readBit(void) {
    uint8_t i = 0;
    //等待IO变低
    while(READ_IO==1 && i<MAXCOUNT) {
        i++;
        DELAY_US(1);
    }
    i = 0;
    //等待IO变高——先低再高表示开始传输一个bit
    while (READ_IO==0 && i<MAXCOUNT) {
        i++;
        DELAY_US(1);
    }
    DELAY_US(40);    //等待40us（0 bit）
    if(READ_IO==1) return 1;
    else return 0;
}
//读取一个Byte函数
uint8_t dht11_readByte(void) {
    uint8_t i = 0;
    uint8_t data = 0;
    for(i=0; i<8; i++) {
        data <<= 1;
        data |= dht11_readBit();
    } 
    return data;
}

/* Class functions ------------------------------------------------ */
//读取温湿度值
uint8_t AITA_DHT::dht11Read(uint8_t *temp, uint8_t *humi) {
    uint8_t buf[5];
    uint8_t i;
    
    if(!dht11_init()) {
        for(i=0; i<5; i++) {
            buf[i] = dht11_readByte();
        }
        
        humidity_integer = buf[0];
        humidity_decimal = buf[1];
        temperature_integer = buf[2];
        temperature_decimal = buf[3];
        checksum = buf[0]+buf[1]+buf[2]+buf[3];
        
        if((buf[0]+buf[1]+buf[2]+buf[3]) == buf[4]) {
            *humi = buf[0];
            *temp = buf[2];
        } else {
            return CHECKSUM;
        }
        
    } else {
        return INITTOUT;
    }
    return SUCC;
}


