#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "definePin.h"
#include "bno055.h"
#include "usart_ATmega328p.h"

// CROSS_TASK VARIABLES
bool tiltu_r = false, tiltd_r = false, tiltr_r = false, tiltl_r = false, bop_r = false, twist_r = false, shake_r = false;
int score = 9999, displayValue = 0;
BNO055 bno055;
float euler[3];
int digits[4];
union ByteToInt16 {
  struct {
    uint8_t lsb;
    uint8_t msb;
  } bytes;
  int16_t value;
};

BNO055::BNO055(uint8_t address) : _address(address), _mode(OPERATION_MODE_CONFIG) {}
bool BNO055::begin(bno055_opmode_t mode) {
  // Perform I2C setup and configuration here
  TWBR = ((F_CPU / 100000UL) - 16) / 2; // Set I2C frequency to 100kHz

  // Verify the BNO055 ID
  uint8_t id = read8(BNO055_CHIP_ID_ADDR);
  if (id != BNO055_ID) {
    return false;
  }

  setMode(OPERATION_MODE_CONFIG);
  _delay_ms(25);

  // Initialize the sensor with the desired mode
  setMode(mode);
  _delay_ms(20);

  return true;
}
void BNO055::setMode(bno055_opmode_t mode) {
  write8(BNO055_OPR_MODE_ADDR, mode);
  _mode = mode;
  _delay_ms(30);
}
bno055_opmode_t BNO055::getMode() {
  return _mode;
}
void BNO055::setExtCrystalUse(bool usextal) {
  bno055_opmode_t modebackup = _mode;
  setMode(OPERATION_MODE_CONFIG);
  _delay_ms(25);
  write8(BNO055_SYS_TRIGGER_ADDR, usextal ? 0x80 : 0x00);
  setMode(modebackup);
  _delay_ms(20);
}
void BNO055::getCalibration(uint8_t *system, uint8_t *gyro, uint8_t *accel, uint8_t *mag) {
  uint8_t cal = read8(BNO055_CALIB_STAT_ADDR);
  *system = (cal >> 6) & 0x03;
  *gyro = (cal >> 4) & 0x03;
  *accel = (cal >> 2) & 0x03;
  *mag = cal & 0x03;
}
// void BNO055::getVector() {
//   // Read vector data
//   uint8_t buffer[6];
//   readData(BNO055_ACCEL_DATA_X_LSB_ADDR, buffer, 6);
//   int16_t x = (int16_t)(((uint16_t)buffer[1] << 8) | buffer[0]);
//   int16_t y = (int16_t)(((uint16_t)buffer[3] << 8) | buffer[2]);
//   int16_t z = (int16_t)(((uint16_t)buffer[5] << 8) | buffer[4]);
// }
// void BNO055::getQuat() {
//   uint8_t buffer[8];
//   readData(BNO055_QUATERNION_DATA_W_LSB_ADDR, buffer, 8);
//   int16_t w = (int16_t)(((uint16_t)buffer[1] << 8) | buffer[0]);
//   int16_t x = (int16_t)(((uint16_t)buffer[3] << 8) | buffer[2]);
//   int16_t y = (int16_t)(((uint16_t)buffer[5] << 8) | buffer[4]);
//   int16_t z = (int16_t)(((uint16_t)buffer[7] << 8) | buffer[6]);
// }
int8_t BNO055::getTemp() {
  return (int8_t)(read8(BNO055_TEMP_ADDR));
}
bool BNO055::isFullyCalibrated() {
  uint8_t system, gyro, accel, mag;
  getCalibration(&system, &gyro, &accel, &mag);
  return (system >= 3 && gyro >= 3 && accel >= 3 && mag >= 3);
}
void BNO055::enterSuspendMode() {
  write8(BNO055_PWR_MODE_ADDR, POWER_MODE_SUSPEND);
}
void BNO055::enterNormalMode() {
  write8(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
}
uint8_t BNO055::read8(uint8_t reg) {
  uint8_t value;
  readData(reg, &value, 1);
  return value;
}
bool BNO055::readLen(uint8_t reg, uint8_t *buffer, uint8_t len) {
  readData(reg, buffer, len);
  return true;
}
bool BNO055::write8(uint8_t reg, uint8_t value) {
  writeData(reg, &value, 1);
  return true;
}
void BNO055::writeData(uint8_t reg, uint8_t *data, uint8_t length) {
  // Start I2C transmission
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));

  // Send I2C address
  TWDR = (_address << 1);
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));

  // Send register address
  TWDR = reg;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));

  // Send data
  for (uint8_t i = 0; i < length; i++) {
    TWDR = data[i];
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
  }

  // Stop I2C transmission
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
  _delay_ms(10);
}
void BNO055::readData(uint8_t reg, uint8_t *data, uint8_t length) {
    // Start I2C transmission
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    // Send I2C address with write flag
    TWDR = (_address << 1);
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    // Send register address
    TWDR = reg;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    // Repeated start
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    // Send I2C address with read flag
    TWDR = (_address << 1) | 1;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    // Read data
    for (uint8_t i = 0; i < length; i++) {
        if (i == (length - 1)) { TWCR = (1 << TWINT) | (1 << TWEN); } 
        else { TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN); }
        while (!(TWCR & (1 << TWINT)));
        data[i] = TWDR; 
    }
}
EulerAngles BNO055::getEuler() {
  uint8_t buffer[6];
  readData(BNO055_EULER_H_LSB_ADDR, buffer, 6);

  ByteToInt16 yaw, pitch, roll;

  yaw.bytes.lsb = buffer[0];
  yaw.bytes.msb = buffer[1];
  
  pitch.bytes.lsb = buffer[2];
  pitch.bytes.msb = buffer[3];
  
  roll.bytes.lsb = buffer[4];
  roll.bytes.msb = buffer[5];

  EulerAngles angles;
  angles.yaw = yaw.value;
  angles.pitch = pitch.value;
  angles.roll = roll.value;

  return angles;
}
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

typedef struct _task {
    signed char state;          // Task's current state
    unsigned long period;       // Task period
    unsigned long elapsedTime;  // Time elapsed since last task tick
    int (*TickFct)(int);        // Task tick function
} task;

#define NUM_TASKS 6

task tasks[NUM_TASKS];
const unsigned long GCD_PERIOD = 1;

const unsigned long IMU_PERIOD = 1;
enum IMU_STATES { I_INIT, I_RUN };
int TickFct_IMU(int);

const unsigned long BUTTON_PERIOD = 5;
enum BUTTON_STATES { B_INIT, B_RUN };
int TickFct_BUTTON(int);

const unsigned long POTENTIOMETER_PERIOD = 5;
enum POTENTIOMETER_STATES { P_INIT, P_RUN};
int TickFct_POTENTIOMETER(int);

const unsigned long TILTBALL_PERIOD = 50;
enum TILTBALL_STATES { T_INIT, T_RUN};
int TickFct_TILTBALL(int);

const unsigned long SCOREBOARD_PERIOD = 1;
enum SCOREBOARD_STATES { S_INIT, S_D0, S_D1, S_D2, S_D3 };
int TickFct_SCOREBOARD(int state);

const unsigned long BLUETOOTH_PERIOD = 2;
enum BLUETOOTH_STATES { BL_INIT, BL_SEND };
int TickFct_BLUETOOTH(int state);

void TimerISR() {
    for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
        if ( tasks[i].elapsedTime >= tasks[i].period ) {               // Check if the task is ready to tick
            tasks[i].state = tasks[i].TickFct(tasks[i].state);         // Tick and set the next state for this task
            tasks[i].elapsedTime = 0;                                  // Reset the elapsed time for the next tick
        }
        tasks[i].elapsedTime += GCD_PERIOD;                            // Increment the elapsed time by GCD_PERIOD
    }
}

int main(void) {
    // Initialize all inputs and outputs
    DDRB = 0xFE; PORTB = 0x01; 
    DDRD = 0xFF; PORTD = 0x00; 
    DDRC = 0x00; PORTC = 0xFF;

    ADC_init();
    // serial_init(9600);
    if (!bno055.begin()) {}
    initUSART();
  
    // Initialize Tasks
    int q = 0;
    tasks[q].period = IMU_PERIOD;
    tasks[q].state = I_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_IMU;
    q++;
    tasks[q].period = BUTTON_PERIOD;
    tasks[q].state = B_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_BUTTON;
    q++;
    tasks[q].period = POTENTIOMETER_PERIOD;
    tasks[q].state = P_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_POTENTIOMETER;
    q++;
    tasks[q].period = TILTBALL_PERIOD;
    tasks[q].state = T_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_TILTBALL;
    q++;
    tasks[q].period = SCOREBOARD_PERIOD;
    tasks[q].state = S_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_SCOREBOARD;
    q++;
    tasks[q].period = BLUETOOTH_PERIOD;
    tasks[q].state = BL_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_BLUETOOTH;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}
    return 0;
}
int TickFct_IMU(int state) {
    static int16_t pitchVal, rollVal;
    static int16_t maxVal_roll = 20;
    static int16_t minVal_roll = -20;
    static int16_t maxVal_pitch = 20;
    static int16_t minVal_pitch = -20;

    switch(state) {
        case I_INIT:
            state = I_RUN;
            break;
        case I_RUN:
            EulerAngles euler = bno055.getEuler();
            pitchVal = euler.pitch;
            rollVal = euler.roll;
            
            pitchVal = map(pitchVal, -2879, 2879, -180, 180);
            rollVal = map(rollVal, -2879, 2879, -180, 180);

            if (rollVal >= maxVal_roll) { tiltd_r = true; }
            else { tiltd_r = false; }
            if (rollVal <= minVal_roll) { tiltu_r = true; }
            else { tiltu_r = false; }
            if (pitchVal >= maxVal_pitch) { tiltr_r = true; }
            else { tiltr_r = false; }
            if (pitchVal <= minVal_pitch) { tiltl_r = true; }
            else { tiltl_r = false; }
            break;
    }
    return state;
}
int TickFct_BUTTON(int state) {
    static int button_val;
    switch(state) {
        case B_INIT:
            state = B_RUN;
            break;
        case B_RUN:
            button_val = ADC_read(3);
            if (button_val > 500) { bop_r = true; }
            else { bop_r = false; }
            // serial_println(bop_r);
            break;
    }
    return state;
}
int TickFct_POTENTIOMETER(int state) {
    static int prev_value, curr_value;
    int shift_value = 4;
    switch(state) {
        case P_INIT:
            // break;
            prev_value = ADC_read(2); // Assuming potentiometer is connected to ADC channel 3 (PC3)
            state = P_RUN;
            break;
        case P_RUN:
            curr_value = ADC_read(2);
            if ((curr_value <= prev_value - shift_value) || (curr_value >= prev_value + shift_value)) { twist_r = true; } else { twist_r = false; }
            prev_value = curr_value;
            // serial_println(twist_r);
            break;
    }
    return state;
}
int TickFct_TILTBALL(int state) {
    int tilt_val;
    switch(state) {
        case T_INIT:
            state = T_RUN;
            break;
        case T_RUN:
            tilt_val = ADC_read(1);
            if (tilt_val > 500) { shake_r = true; }
            else { shake_r = false; }
            // serial_println(shake_r);
            break;
    }
    return state;
}
int TickFct_SCOREBOARD(int state) {
    switch (state) { // State Actions
        case S_INIT:
            state = S_D0;
            break;
        case S_D0:
            if (score > displayValue) { 
              displayValue++; 
              if (displayValue == 9999) {
                displayValue = 0;
              }
            }
            digits[0] = displayValue % 10;
            digits[1] = (displayValue/10) % 10;
            digits[2] = (displayValue/100) % 10;
            digits[3] = (displayValue/1000) % 10;
            outNum(digits[0]);
            PORTB = SetBit(PORTB, 5, 1);
            PORTB = SetBit(PORTB, 4, 1);
            PORTB = SetBit(PORTB, 3, 1);
            PORTB = SetBit(PORTB, 2, 0);
            state = S_D1;
            break;
        case S_D1:
            outNum(digits[1]);
            PORTB = SetBit(PORTB, 5, 1);
            PORTB = SetBit(PORTB, 4, 1);
            PORTB = SetBit(PORTB, 3, 0);
            PORTB = SetBit(PORTB, 2, 1);
            state = S_D2;
            break;
        case S_D2:
            outNum(digits[2]);
            PORTB = SetBit(PORTB, 5, 1);
            PORTB = SetBit(PORTB, 4, 0);
            PORTB = SetBit(PORTB, 3, 1);
            PORTB = SetBit(PORTB, 2, 1);
            state = S_D3;
            break;
        case S_D3:
            outNum(digits[3]);
            PORTB = SetBit(PORTB, 5, 0);
            PORTB = SetBit(PORTB, 4, 1);
            PORTB = SetBit(PORTB, 3, 1);
            PORTB = SetBit(PORTB, 2, 1);
            state = S_D0;
            break;
        default:
            break;
    }
    return state;
}
int TickFct_BLUETOOTH(int state) {
  static int sendValue = 0x00;
  switch(state) {
    case BL_INIT:
        state = BL_SEND;
        break;
    case BL_SEND:
        sendValue = 0x00;
        if (bop_r) { sendValue += 0x01; }
        if (twist_r) { sendValue += 0x02; }
        if (tiltd_r) { sendValue += 0x04; }
        if (tiltu_r) { sendValue += 0x08; }
        if (tiltr_r) { sendValue += 0x10; }
        if (tiltl_r) { sendValue += 0x20; }
        if (shake_r) { sendValue += 0x40; }
        // serial_println(sendValue);
        USART_Send(sendValue);
        break;
  }
  return state;
}
