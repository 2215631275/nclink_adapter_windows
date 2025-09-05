#include <stdexcept>
#include <iostream>
#include <ModbusInterface.h>
#include <plog/Log.h>

namespace modbus {
    // modbus_tcp���캯��
    void  modbus_tcp::modbustcp_connect(const std::string& ip, const short port) {
        ctx_ = modbus_new_tcp(ip.c_str(), port);
        PLOG_DEBUG << "modbus_tcp is connecting to " << ip.c_str() << ":" << port << std::endl;
        std::cout << "modbus_tcp is connecting to " << ip.c_str() << ":" << port << std::endl;
        if (MODBUS_ERR == modbus_set_slave(ctx_, 1) || MODBUS_ERR == modbus_connect(ctx_))
        {
            modbus_free(ctx_);
            PLOG_DEBUG << "modbus_tcp init or connect failed" << std::endl;
            throw std::runtime_error("modbus_tcp init or connect failed");
        }
        else PLOG_DEBUG << "modbus_tcp init and connect succeeded" << std::endl;
 }

    // modbus_tcp��������
    modbus_tcp::~modbus_tcp()
    {
        if (ctx_) {
            modbus_close(ctx_);
            modbus_free(ctx_);
            PLOG_DEBUG << "modbus_tcp connection is finished." << std::endl;
        }
        
    }

    //0x01������Ȧ״̬: 00 01 00 00 00 06 01        01 67 89 00 05 
    int modbus_tcp::read_coil_status(int addr, int nb, uint8_t* dest)
    {
        return modbus_read_bits(ctx_, addr, nb, dest);       // return the number of read bits if successful. Otherwise it shall return -1 and set errno.
    }
    //0x02������ɢ������: 00 01 00 00 00 06 01      02 67 89 00 05 
    int modbus_tcp::read_input_status(int addr, int nb, uint8_t* dest)
    {
        return modbus_read_input_bits(ctx_, addr, nb, dest);  // return the number of read input status if successful. Otherwise it shall return -1 and set errno.
    }
    //0x03�������ּĴ���: 00 01 00 00 00 06 01      03 67 89 00 05 
    int modbus_tcp::read_holding_register(int addr, int nb, uint8_t* dest)
    {
        return modbus_read_registers(ctx_, addr, nb, dest);   //�ɹ�ʱ���ض�ȡ����λ����Ŀ��nb��ʧ��ʱ����-1.
    }
    //0x04��������Ĵ���: 00 01 00 00 00 06 01      04 67 89 00 05    
    int modbus_tcp::read_input_register(int addr, int nb, uint8_t* dest)
    {
        return modbus_read_input_registers(ctx_, addr, nb, dest);  //return the number of read input registers if successful. Otherwise it shall return -1 and set errno.
    }
    //0x05��д������Ȧ: 00 01 00 00 00 06 01        05 67 89 FF 00 
    int modbus_tcp::write_single_coil(int coil_addr, int status)
    {
        return modbus_write_bit(ctx_, coil_addr, status);  //�ɹ�ʱ������1��ʧ��ʱ������-1��
    }
    //0x06��д�������ּĴ���: 00 01 00 00 00 06 01    06 67 89 12 34
    int modbus_tcp::write_single_register(int reg_addr, const uint16_t value)
    {
        return modbus_write_register(ctx_, reg_addr, value); //�ɹ�ʱ����1��ʧ��ʱ����-1.
    }
    //0x0F��15����д�����Ȧ: 00 01 00 00 00 08 01     0F 67 89 00 05 01 1F
    int modbus_tcp::write_multiple_coil(int addr, int nb, const uint8_t* data)
    {
        return modbus_write_bits(ctx_, addr, nb, data); //return the number of written bits if successful. Otherwise it shall return -1 and set errno.
    }
    //0x10��16����д������ּĴ���: 00 01 00 00 00 11 01   10 67 89 00 05 0A 00 00 00 00 00 00 00 00 00 05 
    int modbus_tcp::write_multiple_registers(int addr, int nb, const uint16_t* data)
    {
        return modbus_write_registers(ctx_, addr, nb, data);  //�ɹ�ʱ����д��Ĵ���������nb��ʧ��ʱ����-1.
    }

    void modbus_tcp::modbus_set_float(float f, uint16_t* dest) {
        return modbus_set_float_badc(f, dest);
    }

    void modbus_tcp::modbus_set_int(int n, uint16_t* dest) {
        return modbus_set_int_badc(n, dest);
    }


}  // namespace modbus
