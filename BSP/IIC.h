//
// Created by Valentina_HP on 2023/3/17.
//

#ifndef FOC_IIC_H
#define FOC_IIC_H

#include "common_inc.h"

class IIC {
public:
    void Init(GPIO_TypeDef *GPIO_CLK, GPIO_TypeDef *GPIO_SDA,uint16_t GPIO_Pin_CLK,uint16_t GPIO_Pin_SDA);
    void iic_write_data(unsigned char device,unsigned char registers, unsigned char value);
    uint8_t iic_read_data(unsigned char device,unsigned char registers);

public:
    static void _iic_delay();
    void _SDA_Out();
    void _SDA_In();
    void _SCL_High();
    void _SCL_Low();
    uint8_t _SDA_Read();

    void _SDA_High();
    void _SDA_Low();
     void _IIC_Start();
     void _IIC_Stop();
     void _IIC_Write(uint8_t data);
     uint8_t _IIC_Read();
     bool _IIC_Ack();
private:
    GPIO_TypeDef *_GPIO_CLK;
    GPIO_TypeDef *_GPIO_SDA;
    uint16_t     _GPIO_Pin_CLK;
    uint16_t     _GPIO_Pin_SDA;

};


#endif //FOC_IIC_H
