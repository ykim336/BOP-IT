#ifndef DEFINEPIN_H
#define DEFINEPIN_H

#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "serialATmega.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "helper.h"

#define BOP_BUTTON PC3
#define POTENTIOMETER_PIN PC2
#define TILTBALL_PIN PC1

#define SDA_PIN PC4
#define SCL_PIN PC5

#define RXD_PIN PB0
#define TXD_PIN PC0

#endif