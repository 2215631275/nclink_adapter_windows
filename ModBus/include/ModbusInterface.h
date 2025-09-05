#pragma once
/**
 * @brief  modbus tcp base class
 */
#ifndef __MODBUS_TCP_HPP__
#define __MODBUS_TCP_HPP__

#include <chrono>
#include <string>

#include <modbuslib/modbus.h>

namespace modbus {
    /**
     * modbus_tcp base class
     */
    class modbus_tcp
    {
    public:
        //static constexpr int   MODBUS_ERR = -1
        modbus_t* ctx_;

        ~modbus_tcp();

        //! libmodbus's error return value
        static constexpr int  MODBUS_ERR = -1;
        void modbustcp_connect(const std::string& ip, const short port);
        //0x01������Ȧ״̬: 00 01 00 00 00 06 01        01 67 89 00 05 
        int read_coil_status(int addr, int nb, uint8_t* dest);
        //0x02������ɢ������: 00 01 00 00 00 06 01      02 67 89 00 05 
        int read_input_status(int addr, int nb, uint8_t* dest);
        //0x03�������ּĴ���: 00 01 00 00 00 06 01      03 67 89 00 05 
        int read_holding_register(int addr, int nb, uint8_t* dest);
        //0x04��������Ĵ���: 00 01 00 00 00 06 01      04 67 89 00 05
        int read_input_register(int addr, int nb, uint8_t* dest);
        //0x05��д������Ȧ: 00 01 00 00 00 06 01        05 67 89 FF 00 
        int write_single_coil(int coil_addr, int status);
        //0x06��д�������ּĴ���: 00 01 00 00 00 06 01    06 67 89 12 34
        int write_single_register(int reg_addr, const uint16_t value);
        //0x0F��15����д�����Ȧ: 00 01 00 00 00 08 01     0F 67 89 00 05 01 1F
        int write_multiple_coil(int addr, int nb, const uint8_t* data);
        //0x10��16����д������ּĴ���: 00 01 00 00 00 11 01   10 67 89 00 05 0A 00 00 00 00 00 00 00 00 00 05 
        int write_multiple_registers(int addr, int nb, const uint16_t* data);

        void modbus_set_float(float f, uint16_t* dest);
        void modbus_set_int(int n, uint16_t* dest);
    };
}  // namespace modbus
#endif
