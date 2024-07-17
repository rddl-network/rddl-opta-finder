#include <string.h>
#include <stdio.h>
#include <Arduino.h>
#include <Wire.h>
extern "C"
{
#include <cryptoauthlib.h>
}


#define MAX_I2C_BUSES 2 // ESP32 has 2 I2C bus
/**
 * @brief Minimum size, in bytes, of the internal private structure used to describe
 * I2C commands link.
 */
#define I2C_INTERNAL_STRUCT_SIZE (24)

typedef struct atcaI2Cmaster
{
    int id;
    // i2c_config_t conf;
    int ref_ct;
} ATCAI2CMaster_t;

ATCAI2CMaster_t i2c_hal_data[MAX_I2C_BUSES];

auto i2cModuleP = &Wire1;

const char *TAG = "HAL_I2C";

/** \brief method to change the bus speec of I2C
 * \param[in] iface  interface on which to change bus speed
 * \param[in] speed  baud rate (typically 100000 or 400000)
 */
ATCA_STATUS hal_i2c_change_baud(ATCAIface iface, uint32_t speed)
{

    i2cModuleP->setClock(speed);
    return ATCA_SUCCESS;
}

void printBufferHex2(const byte input[], int inputLength) {
  for (int i = 0; i < inputLength; i++) {
    Serial.print(input[i] >> 4, HEX);
    Serial.print(input[i] & 0x0f, HEX);
  }
  Serial.println();
}

/** \brief
    - this HAL implementation assumes you've included the START Twi libraries in your project, otherwise,
    the HAL layer will not compile because the START TWI drivers are a dependency *
 */

/** \brief hal_i2c_init manages requests to initialize a physical interface.  it manages use counts so when an interface
 * has released the physical layer, it will disable the interface for some other use.
 * You can have multiple ATCAIFace instances using the same bus, and you can have multiple ATCAIFace instances on
 * multiple i2c buses, so hal_i2c_init manages these things and ATCAIFace is abstracted from the physical details.
 */

/** \brief initialize an I2C interface using given config
 * \param[in] hal - opaque ptr to HAL data
 * \param[in] cfg - interface configuration
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_init(ATCAIface iface, ATCAIfaceCfg *cfg)
{
    int bus = cfg->atcai2c.bus;
    i2c_hal_data[bus].id = bus;
    i2c_hal_data[bus].ref_ct = 1;

    bool isBegin = 1;

    i2cModuleP->begin();

    // bool isBegin = i2cModuleP->begin(I2C1_SDA_PIN, I2C1_SCL_PIN);
    if (isBegin == 1)
    {
        Serial.println("I2C bus initiated successfully.");
    }
    else if (isBegin == 0 || isBegin == -1)
    {
        Serial.println("ERROR: Not able to initialize i2C communications.");
        return ATCA_WAKE_FAILED;
    }

    iface->hal_data = &i2c_hal_data[bus];
    return ATCA_SUCCESS;
}

/** \brief HAL implementation of I2C post init
 * \param[in] iface  instance
 * \return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t word_address, uint8_t *txdata, int txlength)
{
    
    
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;
    String payload = "";

    int address = cfg->atcai2c.slave_address;

    i2cModuleP->beginTransmission(address);
    i2cModuleP->write(txdata, txlength);
    i2cModuleP->endTransmission();

    // Serial.print("TX: ");
    // printBufferHex2(txdata, txlength);
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t address, uint8_t *rxdata, uint16_t *rxlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;
    int wantRead = *rxlength;
    i2cModuleP->requestFrom((int)cfg->atcai2c.slave_address, wantRead);
    int i = 0;
    while (i2cModuleP->available() && i < *rxlength)
    {
        rxdata[i++] = i2cModuleP->read();
    }
    *rxlength = i;


    // Serial.print("RX: ");
    // printBufferHex2(rxdata, *rxlength);
    // Serial.println("==========");
    if (*rxlength == 0)
        return ATCA_COMM_FAIL;
    return ATCA_SUCCESS;
}

/** \brief manages reference count on given bus and releases resource if no more refences exist
 * \param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_release(void *hal_data){
    ATCAI2CMaster_t *hal = (ATCAI2CMaster_t *)hal_data;
    if (hal && --(hal->ref_ct) <= 0)
    {
        // I2C_one.end();
        // I2C_two.end();
        i2cModuleP->end();
    }
    return ATCA_SUCCESS;
}

/** \brief Perform control operations for the kit protocol
 * \param[in]     iface          Interface to interact with.
 * \param[in]     option         Control parameter identifier
 * \param[in]     param          Optional pointer to parameter value
 * \param[in]     paramlen       Length of the parameter
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_control(ATCAIface iface, uint8_t option, void *param, size_t paramlen)
{
    (void)param;
    (void)paramlen;

    if (iface && iface->mIfaceCFG)
    {
        if (ATCA_HAL_CHANGE_BAUD == option)
        {
            return hal_i2c_change_baud(iface, *(uint32_t *)param);
        }
        else
        {
            return ATCA_UNIMPLEMENTED;
        }
    }
    return ATCA_BAD_PARAM;
}
