#include "janitza604.h"

bool Janitza604::init(uint32_t baudrate, uint32_t serialParameters)
{
    uint32_t preDelay, postDelay, timeout;
    float bitDuration = 1.f / baudrate;

    preDelay = postDelay = bitDuration * 9.6f * 3.5f * 1e6;
    timeout = 200;

    RS485.setDelays(preDelay, postDelay);
    ModbusRTUClient.setTimeout(timeout);
    return ModbusRTUClient.begin(baudrate, serialParameters) == 1;
}


bool Janitza604::init(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet)
{
    wifiClient      = std::make_unique<WiFiClient>();
    modbusTCPClient = std::make_unique<ModbusTCPClient>(*wifiClient);
    server          = std::make_unique<IPAddress>(first_octet, second_octet, third_octet, fourth_octet);
    return checkSMConnection();
}


bool Janitza604::checkSMConnection()
{
    if (!modbusTCPClient->connected()) {
        // client not connected, start the Modbus TCP client
        Serial.println("Attempting to connect to Modbus TCP server");
        if (!modbusTCPClient->begin(*server)) {
            Serial.println("Modbus TCP Client failed to connect!");
            return false;
        } else {
            Serial.println("Modbus TCP Client connected");
            return true;
        }
    }
    return true;
}


std::vector<int> Janitza604::getDateTCP(uint8_t address)
{
    std::vector<int> date;
    if(checkSMConnection()){
        date.push_back(modbusTCPClient->holdingRegisterRead(address, JANITZA604_REG_DATE_DAY));
        date.push_back(modbusTCPClient->holdingRegisterRead(address, JANITZA604_REG_DATE_MONTH));
        date.push_back(modbusTCPClient->holdingRegisterRead(address, JANITZA604_REG_DATE_YEAR));
        date.push_back(modbusTCPClient->holdingRegisterRead(address, JANITZA604_REG_DATE_HOUR));
        date.push_back(modbusTCPClient->holdingRegisterRead(address, JANITZA604_REG_DATE_MIN));
        date.push_back(modbusTCPClient->holdingRegisterRead(address, JANITZA604_REG_DATE_SEC));
    }

    return date;
}


float Janitza604::getVoltage(uint8_t address, uint32_t line){
    uint32_t reg;

    switch (line){
        case 1: reg = JANITZA604_REG_VOLT_L1; break;
        case 2: reg = JANITZA604_REG_VOLT_L2; break;
        case 3: reg = JANITZA604_REG_VOLT_L3; break;
        default: reg = JANITZA604_REG_VOLT_L1; break;
    }

    return convert32Float(modbusRead32(address, reg));
}


uint32_t Janitza604::modbusRead16(uint8_t addr, uint16_t reg)
{
    uint32_t attempts = 3;
    while (attempts > 0)
    {
        ModbusRTUClient.requestFrom(addr, HOLDING_REGISTERS, reg, 1);
        uint32_t data = ModbusRTUClient.read();
        if (data != INVALID_DATA)
        {
            return data;
        }
        else
        {
            attempts -= 1;
            delay(100);
        }
    }
    return INVALID_DATA;
};


uint32_t Janitza604::modbusRead32(uint8_t addr, uint16_t reg)
{
    uint8_t attempts = 3;
    while (attempts > 0)
    {
        ModbusRTUClient.requestFrom(addr, HOLDING_REGISTERS, reg, 2);
        uint32_t data1 = ModbusRTUClient.read();
        uint32_t data2 = ModbusRTUClient.read();
        if (data1 != INVALID_DATA && data2 != INVALID_DATA)
        {
            return data1 << 16 | data2;
        }
        else
        {
            attempts -= 1;
            delay(100);
        }
    }
    return INVALID_DATA;
};


bool Janitza604::modbusWrite16(uint8_t address, uint16_t reg, uint16_t toWrite)
{
    uint8_t attempts = 3;
    while (attempts > 0)
    {
        if (ModbusRTUClient.holdingRegisterWrite(address, reg, toWrite) == 1)
        {
            return true;
        }
        else
        {
            attempts -= 1;
            delay(100);
        }
    }
    return false;
};


float Janitza604::convert32Float(uint32_t val){
    float data = *((float*)&val);
    return data;
}