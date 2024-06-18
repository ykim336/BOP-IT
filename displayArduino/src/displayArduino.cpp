#include "st77display.h"
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "buzzer.h"
#include "spiAVR.h"
#include "serialATmega.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "definePin.h"
#include <stdlib.h>   
#include <time.h>       
#include "usart_ATmega328p.h"

// CROSS_TASK VARIABLES
bool twist_g = false, bop_g = false, tiltd_g = false, tiltu_g = false, tiltr_g = false, tiltl_g = false, shake_g = false;
bool twist_r = false, bop_r = false, tiltd_r = false, tiltu_r = false, tiltr_r = false, tiltl_r = false, shake_r = false;bool twist_t = false, bop_t = false, tiltd_t = false, tiltu_t = false, tiltr_t = false, tiltl_t = false, shake_t = false;
int lives = 3, speed = 1, countdown = 3, score = 0, multiplier = 1, counter = 0;
bool begin_game = false, game_over = false, bop_screen = false, twist_screen = false, tiltu_screen = false, tiltr_screen = false, tiltd_screen = false, tiltl_screen = false, shake_screen = false, damage_screen = false, correct_screen = false, stage_two_screen = false, stage_three_screen = false, menu_screen = false, game_over_screen = false, b_read = false, play_sfx = false, play_music = false, finished_print = false;

void set_false() {
    twist_t = false, bop_t = false, tiltd_t = false, tiltu_t = false, tiltr_t = false, tiltl_t = false, shake_t = false,
    begin_game = false, game_over = false, bop_screen = false, twist_screen = false, tiltu_screen = false, tiltr_screen = false, tiltd_screen = false, tiltl_screen = false, shake_screen = false, damage_screen = false, correct_screen = false, stage_two_screen = false, stage_three_screen = false, countdown = 3, menu_screen = false, game_over_screen = false;
    return;
}
void set_display_off() {
    begin_game = false, game_over = false, bop_screen = false, twist_screen = false, tiltu_screen = false, tiltr_screen = false, tiltd_screen = false, tiltl_screen = false, shake_screen = false, damage_screen = false, correct_screen = false, stage_two_screen = false, stage_three_screen = false, menu_screen = false, game_over_screen = false;
    return;
}
void reset() {
    twist_g = false, bop_g = false, tiltd_g = false, tiltu_g = false, tiltr_g = false, tiltl_g = false, shake_g = false;
}

typedef struct _task {
    signed char state;          // Task's current state
    unsigned long period;       // Task period
    unsigned long elapsedTime;  // Time elapsed since last task tick
    int (*TickFct)(int);        // Task tick function
} task;

#define NUM_TASKS 7

task tasks[NUM_TASKS];
const unsigned long GCD_PERIOD = 1;

const unsigned long DISPLAY_PERIOD = 1;
enum DISPLAY_STATES { D_INIT, D_WAIT, D_TITLE, D_BOPSCREEN, D_TWISTSCREEN, D_SHAKESCREEN, D_TILTRIGHT, D_TILTLEFT, D_TILTDOWN, D_TILTUP, D_DAMAGE, D_CORRECT, D_STAGE1, D_STAGE2, D_STAGE3, D_GAMEOVER };
int TickFct_DISPLAY(int);

const unsigned long LIVES_PERIOD = 1;
enum LIVES_STATES { L_INIT, L_RUN };
int TickFct_LIVES(int);

const unsigned long COUNTDOWN_PERIOD = 1;
enum COUNTDOWN_STATES { C_INIT, C_RUN };
int TickFct_COUNTDOWN(int);

const unsigned long MUSIC_PERIOD = 1;
enum MUSIC_STATES { M_INIT, M_RUN };
int TickFct_MUSIC(int);

const unsigned long MANAGER_PERIOD = 10;
enum MANAGER_STATES { GM_INIT, GM_RUN, GM_WAIT, GM_END };
int TickFct_MANAGER(int);

const unsigned long BLUETOOTH_PERIOD = 1;
enum BLUETOOTH_STATES { B_INIT, B_READ };
int TickFct_BLUETOOTH(int);

const unsigned long SFX_PERIOD = 1;
enum SFX_STATES { S_INIT, S_RUN };
int TickFct_SFX(int);

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
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    SPI_INIT();
    srand(time(NULL));
    initUSART();

    // Initialize tasks
    int q = 0;
    tasks[q].period = MANAGER_PERIOD;
    tasks[q].state = GM_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_MANAGER;
    q++;
    tasks[q].period = DISPLAY_PERIOD;
    tasks[q].state = D_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_DISPLAY;
    q++;
    tasks[q].period = COUNTDOWN_PERIOD;
    tasks[q].state = C_INIT;
    tasks[q].elapsedTime = tasks[1].period;
    tasks[q].TickFct = &TickFct_COUNTDOWN;
    q++;
    tasks[q].period = LIVES_PERIOD;
    tasks[q].state = L_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_LIVES;
    q++;
    tasks[q].period = MUSIC_PERIOD;
    tasks[q].state = M_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_MUSIC;
    q++;
    tasks[q].period = BLUETOOTH_PERIOD;
    tasks[q].state = B_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_BLUETOOTH;
    q++;
    tasks[q].period = SFX_PERIOD;
    tasks[q].state = S_INIT;
    tasks[q].elapsedTime = tasks[q].period;
    tasks[q].TickFct = &TickFct_SFX;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}
    return 0;
}
int TickFct_DISPLAY(int state) {
    int size, size2;
    int rate_value = 1000;
    static int count = 0;
    switch(state) {
        case D_INIT:
            ST7735_Init();
            populate_screen(BLACK);
            state = D_WAIT;
            break;
        case D_WAIT:
            count++;
            if (count >= rate_value) { finished_print = false; }
            if (menu_screen) { populate_screen(BLACK); state = D_TITLE; count = 0; }
            if (bop_screen) { populate_screen(BLACK); state = D_BOPSCREEN; count = 0; }
            if (twist_screen) { populate_screen(BLACK); state = D_TWISTSCREEN; count = 0; }
            if (shake_screen) { populate_screen(BLACK); state = D_SHAKESCREEN; count = 0; }
            if (tiltr_screen) { populate_screen(BLACK); state = D_TILTRIGHT; count = 0; }
            if (tiltl_screen) { populate_screen(BLACK); state = D_TILTLEFT; count = 0; }
            if (tiltu_screen) { populate_screen(BLACK); state = D_TILTUP; count = 0; }
            if (tiltd_screen) { populate_screen(BLACK); state = D_TILTDOWN; count = 0; }
            if (damage_screen) { populate_screen(BLACK); state = D_DAMAGE; count = 0; }
            if (correct_screen) { populate_screen(BLACK); state = D_CORRECT; count = 0; }
            if (begin_game) { populate_screen(BLACK); state = D_STAGE1; count = 0; }
            if (stage_two_screen) { populate_screen(BLACK); state = D_STAGE2; count = 0; }
            if (stage_three_screen) { populate_screen(BLACK); state = D_STAGE3; count = 0; }
            if (game_over_screen) { populate_screen(BLACK); state = D_GAMEOVER; count = 0;}
            break;
        case D_TITLE:  
            if (menu_screen) {
                if (count == 0) { draw_char(20, 7, 'B', PINK, 4.0); }
                if (count == 1) { draw_char(20, 47, 'O', PINK, 4.0); }
                if (count == 2) { draw_char(20, 83, 'P', PINK, 4.0); }
                if (count == 3) { }
                if (count == 4) { draw_char(60, 69, '*', YELLOW, 3.0);}
                if (count == 5) { draw_char(60, 12, 'I', WHITE, 3.0); }
                if (count == 6) { draw_char(60, 42, 'T', WHITE, 3.0); }
                if (count == 7) { }
                if (count == 8) { draw_char(63, 98, 'S', GRAY, 1.25); }
                if (count == 9) { draw_char(74, 98, 'T', GRAY, 1.29);}
                if (count == 10) { draw_char(85, 98, 'A', GRAY, 1.25); }
                if (count == 11) { draw_char(96, 98, 'R', GRAY, 1.25); }
                if (count == 12) { draw_char(107, 98, 'T', GRAY, 1.29); }
                if (count == 13) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_BOPSCREEN:
            if (bop_screen) {
                size = 2.75;
                size2 = 4.5;
                if (count == 0) { draw_char(12, 25, 'B', BLUE, size); }
                if (count == 1) { draw_char(31, 25, 'O', BLUE, size); }
                if (count == 2) { draw_char(50, 25, 'P', BLUE, size); }
                if (count == 3) { }
                if (count == 4) { draw_char(10, 45, 'I', YELLOW, size2); }
                if (count == 5) { draw_char(43, 45, 'T', YELLOW, size2); }
                if (count == 6) { draw_char(68, 40, '!', YELLOW, size2 + 0.5); }
                if (count == 7) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_TWISTSCREEN:
            if (twist_screen) {
                size = 2.75;
                size2 = 4.5;
                if (count == 0) { draw_char(12, 25, 'T', RED, size); }
                if (count == 1) { draw_char(30, 25, 'W', RED, size); }
                if (count == 2) { draw_char(48, 25, 'I', RED, size); }
                if (count == 3) { draw_char(66, 25, 'S', RED, size); }
                if (count == 4) { draw_char(84, 25, 'T', RED, size); }
                if (count == 5) { }
                if (count == 4) { draw_char(10, 45, 'I', MAGENTA, size2); }
                if (count == 5) { draw_char(43, 45, 'T', MAGENTA, size2); }
                if (count == 6) { draw_char(68, 40, '!', MAGENTA, size2 + 0.5); }
                if (count == 7) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_SHAKESCREEN:
            if (shake_screen) {
                size = 2.75;
                size2 = 4.5;
                if (count == 0) { draw_char(12, 25, 'S', CYAN, size); }
                if (count == 1) { draw_char(30, 25, 'H', CYAN, size); }
                if (count == 2) { draw_char(48, 25, 'A', CYAN, size); }
                if (count == 3) { draw_char(66, 25, 'K', CYAN, size); }
                if (count == 4) { draw_char(84, 25, 'E', CYAN, size); }
                if (count == 5) { }
                if (count == 4) { draw_char(10, 45, 'I', YELLOW, size2); }
                if (count == 5) { draw_char(43, 45, 'T', YELLOW, size2); }
                if (count == 6) { draw_char(68, 40, '!', YELLOW, size2 + 0.5); }
                if (count == 7) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_TILTRIGHT: 
            if (tiltr_screen) {
                size = 2.75;
                size2 = 3;
                if (count == 0) { draw_char(12, 25, 'R', GREEN, size); }
                if (count == 1) { draw_char(29, 26, 'I', GREEN, size); }
                if (count == 2) { draw_char(46, 26, 'G', GREEN, size); }
                if (count == 3) { draw_char(64, 26, 'H', GREEN, size); }
                if (count == 4) { draw_char(82, 26, 'T', GREEN, size); }
                if (count == 5) { }
                if (count == 6) { draw_char(12, 45, 'T', PINK, size2); }
                if (count == 7) { draw_char(36, 45, 'I', PINK, size2); }
                if (count == 8) { draw_char(60, 45, 'L', PINK, size2); }
                if (count == 9) { draw_char(75, 45, 'T', PINK, size2); }
                if (count == 10) { draw_char(94, 40, '!', PINK, size2 + 0.5); }
                if (count == 11) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_TILTLEFT: 
            if (tiltl_screen) {
                size = 2.75;
                size2 = 3;
                if (count == 0) { draw_char(12, 25, 'L', YELLOW, size); }
                if (count == 1) { draw_char(30, 26, 'E', YELLOW, size); }
                if (count == 2) { draw_char(48, 26, 'F', YELLOW, size); }
                if (count == 3) { draw_char(66, 26, 'T', YELLOW, size); }
                if (count == 5) { }
                if (count == 6) { draw_char(12, 45, 'T', PINK, size2); }
                if (count == 7) { draw_char(36, 45, 'I', PINK, size2); }
                if (count == 8) { draw_char(60, 45, 'L', PINK, size2); }
                if (count == 9) { draw_char(75, 45, 'T', PINK, size2); }
                if (count == 10) { draw_char(94, 40, '!', PINK, size2 + 0.5); }
                if (count == 11) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_TILTUP: 
            if (tiltu_screen) {
                size = 2.75;
                size2 = 3;
                if (count == 0) { draw_char(12, 25, 'U', RED, size); }
                if (count == 1) { draw_char(30, 25, 'P', RED, size + 0.1); }
                if (count == 2) { }
                if (count == 3) { draw_char(12, 45, 'T', PINK, size2); }
                if (count == 4) { draw_char(36, 45, 'I', PINK, size2); }
                if (count == 5) { draw_char(60, 45, 'L', PINK, size2); }
                if (count == 6) { draw_char(75, 45, 'T', PINK, size2); }
                if (count == 7) { draw_char(94, 40, '!', PINK, size2 + 0.5); }
                if (count == 8) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_TILTDOWN: 
            if (tiltd_screen) {
                size = 2.75;
                size2 = 3;
                if (count == 0) { draw_char(12, 25, 'D', MAGENTA, size); }
                if (count == 1) { draw_char(30, 26, 'O', MAGENTA, size); }
                if (count == 2) { draw_char(48, 26, 'W', MAGENTA, size); }
                if (count == 3) { draw_char(66, 26, 'N', MAGENTA, size); }
                if (count == 4) { }
                if (count == 5) { draw_char(12, 45, 'T', PINK, size2); }
                if (count == 6) { draw_char(36, 45, 'I', PINK, size2); }
                if (count == 7) { draw_char(60, 45, 'L', PINK, size2); }
                if (count == 8) { draw_char(75, 45, 'T', PINK, size2); }
                if (count == 9) { draw_char(94, 40, '!', PINK, size2 + 0.5); }
                if (count == 10) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_DAMAGE:
            if (damage_screen) {
                size = 2;
                if (count == 0) { draw_char(10, 92, 'X', RED, size); }
                if (count == 1) { draw_char(25, 92, 'X', RED, size); }
                if (count == 2) { draw_char(40, 92, 'X', RED, size); }
                if (count == 3) { draw_char(57, 92, 'X', RED, size); }
                if (count == 4) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_CORRECT:
            if (correct_screen) {
                size = 2;
                if (count == 0) { draw_char(10, 92, 'N', GREEN, size); }
                if (count == 1) { draw_char(25, 92, 'I', GREEN, size); }
                if (count == 2) { draw_char(40, 92, 'C', GREEN, size); }
                if (count == 3) { draw_char(57, 92, 'E', GREEN, size); } 
                if (count == 4) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_STAGE1:
            if (begin_game) {
                size = 2.75;
                size2 = 4.5;
                if (count == 0) { draw_char(12, 25, 'S', WHITE, size); }
                if (count == 1) { draw_char(30, 25, 'T', WHITE, size); }
                if (count == 2) { draw_char(48, 25, 'A', WHITE, size); }
                if (count == 3) { draw_char(66, 25, 'G', WHITE, size); }
                if (count == 4) { draw_char(84, 25, 'E', WHITE, size); }
                if (count == 5) { }
                if (count == 4) { draw_char(10, 45, 'O', CYAN, size2); }
                if (count == 5) { draw_char(43, 45, 'N', CYAN, size2); }
                if (count == 6) { draw_char(76, 45, 'E', CYAN, size2); }
                if (count == 7) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_STAGE2:
            if (stage_two_screen) {
                size = 2.75;
                size2 = 4.5;
                if (count == 0) { draw_char(12, 25, 'S', WHITE, size); }
                if (count == 1) { draw_char(30, 25, 'T', WHITE, size); }
                if (count == 2) { draw_char(48, 25, 'A', WHITE, size); }
                if (count == 3) { draw_char(66, 25, 'G', WHITE, size); }
                if (count == 4) { draw_char(84, 25, 'E', WHITE, size); }
                if (count == 5) { }
                if (count == 4) { draw_char(10, 45, 'T', MAGENTA, size2); }
                if (count == 5) { draw_char(43, 45, 'W', MAGENTA, size2); }
                if (count == 6) { draw_char(76, 45, 'O', MAGENTA, size2); }
                if (count == 7) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_STAGE3: 
            if (stage_three_screen) {
                size = 2.75;
                size2 = 2.75;
                if (count == 0) { draw_char(12, 25, 'S', WHITE, size); }
                if (count == 1) { draw_char(30, 25, 'T', WHITE, size); }
                if (count == 2) { draw_char(48, 25, 'A', WHITE, size); }
                if (count == 3) { draw_char(66, 25, 'G', WHITE, size); }
                if (count == 4) { draw_char(84, 25, 'E', WHITE, size); }
                if (count == 4) { }
                if (count == 5) { draw_char(12, 45, 'T', RED, size2); }
                if (count == 6) { draw_char(30, 45, 'H', RED, size2); }
                if (count == 7) { draw_char(48, 45, 'R', RED, size2); }
                if (count == 8) { draw_char(66, 45, 'E', RED, size2); }
                if (count == 9) { draw_char(84, 45, 'E', RED, size2); }
                if (count == 10) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
        case D_GAMEOVER: 
            if (game_over_screen) {
                size = 2.75;
                size2 = 3;
                if (count == 0) { draw_char(12, 25, 'G', RED, size); }
                if (count == 1) { draw_char(30, 25, 'A', RED, size); }
                if (count == 2) { draw_char(48, 25, 'M', RED, size); }
                if (count == 3) { draw_char(66, 25, 'E', RED, size); }
                if (count == 4) { }
                if (count == 5) { draw_char(12, 45, 'O', WHITE, size2); }
                if (count == 6) { draw_char(36, 45, 'V', WHITE, size2); }
                if (count == 7) { draw_char(60, 45, 'E', WHITE, size2); }
                if (count == 8) { draw_char(84, 45, 'R', WHITE, size2); }
                if (count == 10) { finished_print = true;}
                if (count < 100) { count++; }
            }
            else { finished_print = true; state = D_WAIT; count = 0; }
            break;
    }
    return state;
}
int TickFct_LIVES(int state) {
    switch(state) {
        case L_INIT:
            state = L_RUN;
            PORTC = SetBit(PORTC, GREEN_LED, 0);
            PORTC = SetBit(PORTC, YELLOW_LED, 0);
            PORTC = SetBit(PORTC, RED_LED, 0);
            break;
        case L_RUN:
            if (lives == 3) {
                PORTC = SetBit(PORTC, GREEN_LED, 1);
                PORTC = SetBit(PORTC, YELLOW_LED, 0);
                PORTC = SetBit(PORTC, RED_LED, 0);
            }
            else if (lives == 2) {
                PORTC = SetBit(PORTC, GREEN_LED, 0);
                PORTC = SetBit(PORTC, YELLOW_LED, 1);
                PORTC = SetBit(PORTC, RED_LED, 0);
            }
            else if (lives == 1) {
                PORTC = SetBit(PORTC, GREEN_LED, 0);
                PORTC = SetBit(PORTC, YELLOW_LED, 0);
                PORTC = SetBit(PORTC, RED_LED, 1);
            }
            else {
                PORTC = SetBit(PORTC, GREEN_LED, 0);
                PORTC = SetBit(PORTC, YELLOW_LED, 0);
                PORTC = SetBit(PORTC, RED_LED, 0);
            }
            break;
    }
    return state;
}
int TickFct_COUNTDOWN(int state) {
    switch(state) {
        case C_INIT:
            state = C_RUN;
            PORTC = SetBit(PORTC, COUNT_ONE_LED, 0);
            PORTC = SetBit(PORTC, COUNT_TWO_LED, 0);
            PORTC = SetBit(PORTC, COUNT_THREE_LED, 0);
            break;
        case C_RUN:
            if (countdown == 2) {
                PORTC = SetBit(PORTC, COUNT_ONE_LED, 1);
                PORTC = SetBit(PORTC, COUNT_TWO_LED, 0);
                PORTC = SetBit(PORTC, COUNT_THREE_LED, 0);
            }
            else if (countdown == 1) {
                PORTC = SetBit(PORTC, COUNT_ONE_LED, 0);
                PORTC = SetBit(PORTC, COUNT_TWO_LED, 1);
                PORTC = SetBit(PORTC, COUNT_THREE_LED, 0);
            }
            else if (countdown == 0) {
                PORTC = SetBit(PORTC, COUNT_ONE_LED, 0);
                PORTC = SetBit(PORTC, COUNT_TWO_LED, 0);
                PORTC = SetBit(PORTC, COUNT_THREE_LED, 1);
            }
            else {
                PORTC = SetBit(PORTC, COUNT_ONE_LED, 0);
                PORTC = SetBit(PORTC, COUNT_TWO_LED, 0);
                PORTC = SetBit(PORTC, COUNT_THREE_LED, 0);
            }
            break;
    }
    return state;
}
int TickFct_MUSIC(int state) {
    static unsigned int noteIndex = 0, noteTickCounter = 0;
    unsigned int tempo = 140;
    unsigned int wholenote = (60000 * 2) / tempo;
    static int music_rate = 2 , music_counter = 0;
    switch (state) {
        case M_INIT:
            if (play_music) {
                state = M_RUN;
            }
            break;
        case M_RUN:
            if (play_music) {
                if (music_counter % music_rate == 0) {
                    if (noteTickCounter >= (wholenote / abs(noteDurations[noteIndex]))) {
                        noteTickCounter = 0;
                        noteIndex++;
                        if (noteIndex >= sizeof(cantinaBandNotes) / sizeof(cantinaBandNotes[0])) {
                            noteIndex = 0;
                        }
                    }
                    play_note(cantinaBandNotes[noteIndex]);
                    noteTickCounter++;
                }
                music_counter++;
            }
            else { noteIndex = 0; noteTickCounter = 0; music_counter = 0; play_note(0); }
            break;
    }
    return state;
}
int TickFct_MANAGER(int state) {
    static int randomNumber, counter;
    static int period = 150;
    switch(state) {
        case GM_INIT:
            b_read = true;
            menu_screen = true;
            if (bop_g) { 
                state = GM_WAIT; set_false(); begin_game = true; b_read = false; menu_screen = false;
                lives = 3; speed = 1; score = 0; multiplier = 1; play_sfx = true; play_music = true;
            }
            break;
        case GM_RUN:
            if (finished_print) {
                set_false(); // NOTE: display can only be played once
                counter = 1;
                period = 175 - (25 * speed);
                state = GM_WAIT;
                if (score == 15 && !stage_two_screen) { speed = 2; stage_two_screen = true; multiplier = 2; score++; } // Set Stage 2
                if (score == 30 && !stage_three_screen) { speed = 3; stage_three_screen = true; multiplier = 3; score++; } // Set Stage 3
                if (score == 60) { multiplier = 5; speed = 4; }
                if (score == 100) { multiplier = 7; speed = 5; }
                if (lives == 0) { b_read = false; game_over = true; state = GM_END; counter = 0; game_over_screen = true; } // Set Game Over
                if (!(stage_two_screen || stage_three_screen || game_over)) {
                    randomNumber = rand() % 7;
                    switch (randomNumber) { // Choose Random Task
                        case 0: twist_t = true; twist_screen = true; break;
                        case 1: bop_t = true; bop_screen = true; play_sfx = true; break;
                        case 2: tiltd_t = true; tiltd_screen = true; break;
                        case 3: tiltu_t = true; tiltu_screen = true; break;
                        case 4: tiltr_t = true; tiltr_screen = true; break;
                        case 5: tiltl_t = true; tiltl_screen = true; break;
                        case 6: shake_t = true; shake_screen = true; break;
                    }
                }
                play_sfx = true; 
            }
            break;
        case GM_WAIT:
            if (finished_print) {
                if (counter % period == 0) {
                    countdown--; // Double Check
                    b_read = true;
                    if (countdown < 0) {  
                        b_read = false;
                        if (!(begin_game || stage_two_screen || stage_three_screen)) { 
                            lives--; set_false(); damage_screen = true; play_sfx = true; 
                        } 
                        state = GM_RUN;
                    }
                    if (twist_t && twist_g) { score++; set_false(); b_read = false; correct_screen = true; state = GM_RUN; play_sfx = true; }
                    if (bop_t && bop_g) { score++; set_false(); b_read = false; correct_screen = true; state = GM_RUN; play_sfx = true; }
                    if (tiltl_t && tiltl_g) { score++; set_false(); b_read = false; correct_screen = true; state = GM_RUN; play_sfx = true; } 
                    if (tiltr_t && tiltr_g) { score++; set_false(); b_read = false; correct_screen = true; state = GM_RUN; play_sfx = true; } 
                    if (tiltu_t && tiltu_g) { score++; set_false(); b_read = false; correct_screen = true; state = GM_RUN; play_sfx = true; } 
                    if (tiltd_t && tiltd_g) { score++; set_false(); b_read = false; correct_screen = true; state = GM_RUN; play_sfx = true; }
                    if (shake_t && shake_g) { score++; set_false(); b_read = false; correct_screen = true; state = GM_RUN; play_sfx = true; }
                }
                counter++;
            }
            break;
        case GM_END:
            if (counter == 300) { state = GM_INIT; game_over_screen = false; counter = 0; }
            counter++;
            play_music = false;
            break;
    }
    return state;
}
int TickFct_BLUETOOTH(int state) {
    static int recievedValue = 0x00;
    switch(state) {
        case B_INIT:
            state = B_READ;
            break;
        case B_READ:
            if (b_read) {
                if(!USART_HasReceived()) break;
                recievedValue = USART_Receive();
                bop_g = recievedValue & 0x01;
                twist_g = recievedValue & 0x02;
                tiltd_g = recievedValue & 0x04;
                tiltu_g = recievedValue & 0x08;
                tiltr_g = recievedValue & 0x10;
                tiltl_g = recievedValue & 0x20;
                shake_g = recievedValue & 0x40;
            }
            else if (!b_read) {
                recievedValue = 0x00;
                bop_g = false;
                twist_g = false; 
                tiltd_g = false;
                tiltu_g = false;
                tiltr_g = false;
                tiltl_g = false;
                shake_g = false;
            }
            recievedValue = 0x00;
            break;
    }
    return state;
}
int TickFct_SFX(int state) {
    static int n = 0;
    int duration_sfx = 3;
    switch(state) {
        case S_INIT:
            state = S_RUN;
            break;
        case S_RUN:
            if (play_sfx) {
                if (n <= duration_sfx) { PORTD = SetBit(PORTD, ACTIVE_BUZZ, 1); n++; }
                else { PORTD = SetBit(PORTD, ACTIVE_BUZZ, 0); play_sfx = false; }
            }
            if (!play_sfx) { n = 0; PORTD = SetBit(PORTD, ACTIVE_BUZZ, 0); }
            break;
    }
    return state;
}
