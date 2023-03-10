/*
 * @file: i2c.c
 * @desc: This file provides all the functions required foe I2C Communication
 * @author: Jahnavi Pinnamaneni; japi8358@colorado.edu
 */

#include "i2c.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define CMD_DATA_TO_READ 0xF3
#define MULTIPLIER 175.72
#define MAX_RES 65536
#define CONSTANT 46.85
#define SETUP_WAIT_TIME 80000
#define READ_TEMP_WAIT_TEMP 11000
//Global variables
I2C_TransferReturn_TypeDef transferStatus;
I2C_TransferSeq_TypeDef transferSequence;
uint8_t cmd_data;
uint16_t read_data;

/*
 * @brief: This function initializes the SCL and SDA GPIO pins to pull up configuration. Additionally the clock is also set to the High frequency peripheral clock (HFPERCLK)
 */
void i2c_init()
{
  //uint32_t i2c_bus_frequency;
  I2CSPM_Init_TypeDef I2C_Config = {
  .port = I2C0,
  .sclPort = gpioPortC,
  .sclPin = 10,
  .sdaPort = gpioPortC,
  .sdaPin = 11,
  .portLocationScl = 14,
  .portLocationSda = 16,
  .i2cRefFreq = 0,
  .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
  .i2cClhr = i2cClockHLRStandard
  };

  I2CSPM_Init(&I2C_Config);
  //i2c_bus_frequency = I2C_BusFreqGet(I2C0);   //90566
}

/*
 *  @brief: This function writes to the I2C peripheral the command that is passed in its argument.
 */
void i2c_write(uint8_t cmd)
{
  cmd_data = cmd;
  transferSequence.addr = SI7021_DEVICE_ADDR << 1;
  transferSequence.flags = I2C_FLAG_WRITE;
  transferSequence.buf[0].data = &cmd_data;
  transferSequence.buf[0].len = sizeof(cmd_data);

  transferStatus = I2CSPM_Transfer(I2C0, &transferSequence);
  if(transferStatus != i2cTransferDone)
    {
      LOG_ERROR("I2CSPM Transfer: I2C bus write of cmd = %d failed", cmd);
    }
}

/*
 *  @brief: This function writes to the I2C peripheral the command that is passed in its argument.
 */
void i2c_read(void)
{
  transferSequence.addr = SI7021_DEVICE_ADDR << 1;
  transferSequence.flags = I2C_FLAG_READ;
  transferSequence.buf[0].data = (uint8_t *)&read_data;
  transferSequence.buf[0].len = sizeof(read_data);

  transferStatus = I2CSPM_Transfer(I2C0, &transferSequence);
  if(transferStatus != i2cTransferDone)
    {
      LOG_ERROR("I2CSPM Transfer: I2C bus read failed");
    }
}

/*
 *  @brief: This function performs a series of I2C actions to request temperature from the Si7021 sensor and read the measured temperature.
 */
void i2c_read_temp()
{
  uint8_t cmd = CMD_DATA_TO_READ;
  uint16_t read;
  uint16_t temp = 0;
  //Initialize the I2C peripheral
  i2c_init();
  //Power ON the peripheral
  gpioSensor_enSetOn();
  //provide a delay of 80ms to allow for setup time
  timerWaitUs(SETUP_WAIT_TIME);
  //write the cmd to read temperature
  i2c_write(cmd);
  //wait for temperature measurement
  timerWaitUs(READ_TEMP_WAIT_TEMP);
  //read the measured temperature value
  i2c_read();
  //Convert the temperature reading into Celcius
  read = read_data;
  temp = ((read & 0x00FF) << 8) & 0xFF00;
  read = (read >> 8) | temp;
  temp = (int)( ((MULTIPLIER * read)/MAX_RES) - CONSTANT );
  LOG_INFO("I2CSPM Read Measurement %d C \r\n", temp);

  //Power OFF the peripheral
  gpioSensor_enSetOff();
}
