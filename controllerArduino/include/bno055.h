#ifndef __BNO055_H__
#define __BNO055_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "serialATmega.h"

/** BNO055 Address A **/
#define BNO055_ADDRESS_A (0x28)
/** BNO055 Address B **/
#define BNO055_ADDRESS_B (0x29)
/** BNO055 ID **/
#define BNO055_ID (0xA0)

/** BNO055 Registers **/
#define BNO055_CHIP_ID_ADDR 0x00
#define BNO055_ACCEL_REV_ID_ADDR 0x01
#define BNO055_MAG_REV_ID_ADDR 0x02
#define BNO055_GYRO_REV_ID_ADDR 0x03
#define BNO055_SW_REV_ID_LSB_ADDR 0x04
#define BNO055_SW_REV_ID_MSB_ADDR 0x05
#define BNO055_BL_REV_ID_ADDR 0x06
#define BNO055_OPR_MODE_ADDR 0x3D
#define BNO055_SYS_TRIGGER_ADDR 0x3F
#define BNO055_PWR_MODE_ADDR 0x3E
#define BNO055_SYS_STAT_ADDR 0x39
#define BNO055_SELFTEST_RESULT_ADDR 0x36
#define BNO055_SYS_ERR_ADDR 0x3A
#define BNO055_CALIB_STAT_ADDR 0x35
#define BNO055_ACCEL_DATA_X_LSB_ADDR 0x08
#define BNO055_TEMP_ADDR 0x34
#define BNO055_QUATERNION_DATA_W_LSB_ADDR 0x20
#define BNO055_EULER_H_LSB_ADDR 0x1A  // Starting address for Euler angles
#define BNO055_EULER_H_MSB_ADDR 0x1B
#define BNO055_EULER_R_LSB_ADDR 0x1C
#define BNO055_EULER_R_MSB_ADDR 0x1D
#define BNO055_EULER_P_LSB_ADDR 0x1E
#define BNO055_EULER_P_MSB_ADDR 0x1F

/** Power modes **/
#define POWER_MODE_NORMAL 0x00
#define POWER_MODE_LOWPOWER 0x01
#define POWER_MODE_SUSPEND 0x02

/** A structure to represent offsets **/
typedef struct {
  int16_t accel_offset_x;
  int16_t accel_offset_y;
  int16_t accel_offset_z;
  int16_t mag_offset_x;
  int16_t mag_offset_y;
  int16_t mag_offset_z;
  int16_t gyro_offset_x;
  int16_t gyro_offset_y;
  int16_t gyro_offset_z;
  int16_t accel_radius;
  int16_t mag_radius;
} bno055_offsets_t;

/** Operation mode settings **/
typedef enum {
  OPERATION_MODE_CONFIG = 0X00,
  OPERATION_MODE_ACCONLY = 0X01,
  OPERATION_MODE_MAGONLY = 0X02,
  OPERATION_MODE_GYRONLY = 0X03,
  OPERATION_MODE_ACCMAG = 0X04,
  OPERATION_MODE_ACCGYRO = 0X05,
  OPERATION_MODE_MAGGYRO = 0X06,
  OPERATION_MODE_AMG = 0X07,
  OPERATION_MODE_IMUPLUS = 0X08,
  OPERATION_MODE_COMPASS = 0X09,
  OPERATION_MODE_M4G = 0X0A,
  OPERATION_MODE_NDOF_FMC_OFF = 0X0B,
  OPERATION_MODE_NDOF = 0X0C
} bno055_opmode_t;

typedef struct {
  int16_t yaw;
  int16_t pitch;
  int16_t roll;
} EulerAngles;

class BNO055 {
public:
  BNO055(uint8_t address = BNO055_ADDRESS_A);
  
  bool begin(bno055_opmode_t mode = OPERATION_MODE_NDOF);
  void setMode(bno055_opmode_t mode);
  bno055_opmode_t getMode();
  void getRevInfo();
  void setExtCrystalUse(bool usextal);
  void getSystemStatus();
  void getCalibration(uint8_t *system, uint8_t *gyro, uint8_t *accel, uint8_t *mag);
  void getVector();
  EulerAngles getEuler();
  void getQuat();
  int8_t getTemp();
  bool isFullyCalibrated();
  void enterSuspendMode();
  void enterNormalMode();

private:
  uint8_t _address;
  bno055_opmode_t _mode;
  
  uint8_t read8(uint8_t reg);
  bool readLen(uint8_t reg, uint8_t *buffer, uint8_t len);
  bool write8(uint8_t reg, uint8_t value);
  void writeData(uint8_t reg, uint8_t *data, uint8_t length);
  void readData(uint8_t reg, uint8_t *data, uint8_t length);
};

#endif
