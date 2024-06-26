#ifndef _JANITZA604_H_INCLUDED
#define _JANITZA604_H_INCLUDED

#include <memory> 
#include <vector>
#include <WiFi.h>
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>

constexpr int JANITZA604_REG_DATE_DAY = 6;              // DAY
constexpr int JANITZA604_REG_DATE_MONTH = 7;            // MONTH
constexpr int JANITZA604_REG_DATE_YEAR = 8;             // YEAR
constexpr int JANITZA604_REG_DATE_HOUR = 9;             // HOUR
constexpr int JANITZA604_REG_DATE_MIN = 10;             // MIN
constexpr int JANITZA604_REG_DATE_SEC = 11;             // SEC
constexpr int JANITZA604_REG_VOLT_L1 = 19000;           // VOLT on L1
constexpr int JANITZA604_REG_VOLT_L2 = 19002;           // VOLT on L2
constexpr int JANITZA604_REG_VOLT_L3 = 19004;           // VOLT on L3




#define INVALID_DATA 0xFFFFFFFF            

class Janitza604{
public:
    Janitza604() = default;

    /**
     * Set preDelay and postDelay and start the Modbus RTU client
     * with the parameters for the Janitza UMG 604.
     *
     * @param baudrate Defaults to 38400, if not specified.
     * @param serialParameters Defaults to 8N2, if not specified.
     *
     * @return true in case of success, false otherwise.
     */
    bool init(uint32_t baudrate = 38400, uint32_t serialParameters = SERIAL_8N2);
    
    /**
     * Set IP address of server and Start Modbus TCP Connection
     *
     * @param first_octet First octet of IPv4 of Slave Address
     * @param second_octet Second octet of IPv4 of Slave Address
     * @param third_octet Third octet of IPv4 of Slave Address
     * @param fourth_octet Fourth octet of IPv4 of Slave Address
     *
     * @return true in case of success, false otherwise.
     */
    bool init(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
    
    /**
     * Start Modbus TCP Connection to Slave
     *
     * @return true in case of success, false otherwise.
     */
    bool checkSMConnection();
    
    /**
     * Read date parameters 
     *
     * @param address Address of slave device
     * 
     * @return vector of date parameters.
     */
    std::vector<int> getDateTCP(uint8_t address);
    
    /**
     * Read voltage parameters 
     *
     * @param address Address of slave device
     * 
     * @return voltage
     */
    float getVoltage(uint8_t address, uint32_t line = 1);
    
    /**
     * Read a 16-bits register.
     *
     * @param addr Modbus id of the target device.
     * @param reg Start address of the register.
     *
     * @return The read value or INVALID_DATA.
     */
    uint32_t modbusRead16(uint8_t addr, uint16_t reg);

    /**
     * Read two consecutive 16-bits registers and compose them
     * into a single 32-bits value, by shifting the first value
     * left by 16 bits.
     *
     * @param addr Modbus id of the target device.
     * @param reg Start address of the register.
     *
     * @return The composed value or INVALID_DATA.
     */
    uint32_t modbusRead32(uint8_t addr, uint16_t reg);
    
    /**
     * Write 8-bits or 16-bits values to a given register.
     *
     * @param address Modbus id of the target device.
     * @param reg Start address of the destination register.
     * @param toWrite Content that will be written to the destination register.
     *
     * @return true in case of success, false otherwise.
     */
    bool modbusWrite16(uint8_t address, uint16_t reg, uint16_t toWrite);

    /**
     * Convert unsigned data to float 
     *
     * @param val unsigned value 
     * 
     * @return converted value
     */
    float convert32Float(uint32_t val);


private:
    std::unique_ptr<WiFiClient>       wifiClient;
    std::unique_ptr<ModbusTCPClient>  modbusTCPClient;
    std::unique_ptr<IPAddress>        server;
};

#endif