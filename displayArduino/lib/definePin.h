#ifndef DEFINEPIN_H
#define DEFINEPIN_H

#include "st77display.h"
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "spiAVR.h"
#include "serialATmega.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "helper.h"

#define GREEN_LED PC5
#define YELLOW_LED PC4
#define RED_LED PC3
#define COUNT_ONE_LED PC2
#define COUNT_TWO_LED PC1
#define COUNT_THREE_LED PC0

#define ACTIVE_BUZZ PD2
#define PASS_BUZZ PD6

#define TXD_PIN PD3
#define RXD_PIN PD4

#endif