# BOP-IT GAME PROJECT

## Introduction

BOP-IT is a modern rendition of the classic "Simon Says" game created by Hasbro Gaming. This project brings the excitement of BOP-IT to life with a combination of hardware components and software libraries, creating an engaging game where players respond to commands by performing specific actions.

https://github.com/ykim336/BOP-IT/assets/117234817/73df20df-1fd7-4ba1-ab52-3ee5c305cc77 

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Hardware Components](#hardware-components)
- [Software Libraries](#software-libraries)
- [Setup](#setup)
  - [Hardware Connections](#hardware-connections)
  - [Software Setup](#software-setup)
  - [Calibration](#calibration)
- [How to Play](#how-to-play)
  - [Starting the Game](#starting-the-game)
  - [Responding to Commands](#responding-to-commands)
  - [Game Feedback](#game-feedback)
  - [Game Over](#game-over)
- [License](#license)

## Features

- **ST7735 Display**: Provides visual feedback and game status updates, displaying stages, scores, and game over messages.
- **BNO055 IMU**: Detects tilt actions (up, down, left, right) with precise measurements using Euler's angles.
- **Passive Buzzer Music**: Plays the Cantina Band theme from Star Wars at 140 BPM, enhancing the gaming experience with background music.
- **Bluetooth Module (USART)**: Enables wireless communication between the display and the controller.
- **LED Indicators**: Show the timer and remaining lives, providing real-time feedback to the player.
- **Interactive Commands**: Respond to various commands such as BOP, TWIST, TILT, and SHAKE with corresponding physical actions.
- **Sound Effects**: Includes beep sounds for screen changes and actions, making the game more interactive.
- **Random Score Display**: Adds a dynamic visual element by displaying random scores.
- **Volume Control**: Adjust the music volume using a potentiometer.
- **Power Toggle Switch**: Allows for turning the system on and off.

## Hardware Components

- **Bluetooth Module** (2x): Facilitates communication between the game components.
- **ST7735 Display**: Provides visual feedback and game information.
- **LEDs** (6x): Indicate timer and lives.
- **Toggle Switch**: Powers the system on and off.
- **Tilt Ball Switch**: Detects shake actions.
- **Passive Buzzer**: Plays background music.
- **Active Buzzer**: Produces sound effects.
- **BNO055 IMU**: Measures tilt actions.
- **4-Digit Display**: Shows scores and other game information.
- **Button**: Used for the BOP action.
- **Potentiometers** (2x): One for volume control and one for the TWIST action.
- **Arduino Nano and Arduino**: Microcontrollers for running the game logic.
- **Resistors** (7x): Used for various electronic circuits.

## Software Libraries

- **spiAVR.h**: Used for controlling the ST7735 display.
- **buzzer.h**: Manages the music playback on the passive buzzer.
- **bno055.h**: Custom library for interfacing with the BNO055 IMU.
- **usart_ATMEGA.h**: Facilitates Bluetooth communication.
- **definePin.h**: Used for defining pin names for better code readability.

## Setup

### Hardware Connections

1. **Bluetooth Modules**: Connect to the Arduino using USART communication pins.
2. **ST7735 Display**: Connect to the SPI pins on the Arduino.
3. **LEDs**: Connect to digital pins for indicating the timer and lives.
4. **Tilt Ball Switch**: Connect to a digital input pin.
5. **Buzzers**: Connect the passive buzzer for music and the active buzzer for sound effects.
6. **BNO055 IMU**: Connect to the I2C pins on the Arduino.
7. **4-Digit Display**: Connect to the appropriate pins for displaying scores.
8. **Button**: Connect to a digital input pin for the BOP action.
9. **Potentiometers**: Connect one for volume control and one for the TWIST action.
10. **Toggle Switch**: Connect to the power supply to control the system power.

### Software Setup

1. **Install Libraries**: Ensure the following libraries are installed in your Arduino IDE:
    - `spiAVR.h`
    - `buzzer.h`
    - `bno055.h`
    - `usart_ATMEGA.h`
    - `definePin.h`
2. **Upload Code**: Upload the provided Arduino code to the respective microcontrollers (Arduino Nano and Arduino).

### Calibration

1. **BNO055 IMU**: Calibrate the IMU for accurate tilt detection. Follow the calibration procedure provided in the BNO055 documentation.
2. **Testing**: Test the buzzer sound, display functionality, and Bluetooth communication to ensure all components are working correctly.

## How to Play

### Starting the Game

- Press the BOP button to begin the game.
- The display shows "STAGE 1" and the LEDs indicate the timer and your three lives.

### Responding to Commands

- **BOP IT!**: Press the BOP button.
- **TWIST IT!**: Twist the potentiometer.
- **TILT (UP, DOWN, LEFT, RIGHT)!**: Tilt the controller in the specified direction.
- **SHAKE IT!**: Shake the controller, detected by the tilt-ball switch.

### Game Feedback

- Sound effects play when changing screens and performing actions.
- The Cantina Band music plays in the background during gameplay.
- The score is displayed randomly to add a visual element.
- Adjust the music volume using the blue potentiometer.
- Turn off the system using the toggle switch.

### Game Over

- If you run out of lives, the display shows "GAME OVER."

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

Enjoy playing BOP-IT and experience the thrill of this engaging game!
