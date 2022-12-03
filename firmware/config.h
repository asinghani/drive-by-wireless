#ifndef CONFIG_H
#define CONFIG_H

#define IO_1 28
#define IO_2 27
#define IO_3 26
#define IO_4 25
#define IO_5 24
#define IO_6 23
#define IO_7 22
#define IO_8 21
#define IO_9 20
#define IO_10 19

// Cockpit zone config
#define STEERING_UART uart1
#define STEERING_RX IO_8
#define STEERING_TX IO_9

// Steering zone config
#define STEERING_SERVO_IO IO_10
#define STEERING_SERVO_CENTER 1385 // Determined experimentally
#define STEERING_SERVO_RANGE 300
#define STEERING_FEEDBACK_IO IO_1
#define STEERING_FEEDBACK_ADC 2 // ch2 = gpio28

// Drivetrain zone config


// Blinker I/O pins
#define IO_BLINKL 18
#define IO_BLINKR 29

// UWB I/O pins
#define IO_DW_WAKEUP 10
#define IO_DW_7 11
#define IO_DW_CSn 13
#define IO_DW_RSTn 16
#define IO_DW_IRQ 17

// Test-point I/O pins
#define IO_STATUS 0
#define IO_TP1 1
#define IO_TP2 2
#define IO_TP3 3
#define IO_TP4 4
#define IO_TP5 5
#define IO_TP6 6

// UWB config
#define UWB_PIN_MISO 12
#define UWB_PIN_CS   13
#define UWB_PIN_SCK  14
#define UWB_PIN_MOSI 15
#define UWB_PIN_RSTN 16
#define UWB_SPI_PORT spi1

// Microseconds to raise test-point for
#define TP_TIME_US 100

#define ZID_COCKPIT 0
#define ZID_STEERING 1
#define ZID_DRIVETRAIN 2

#ifdef ZONE_COCKPIT
#define ZONE_ID ZID_COCKPIT 
#endif

#ifdef ZONE_STEERING
#define ZONE_ID ZID_STEERING
#endif

#ifdef ZONE_DRIVETRAIN
#define ZONE_ID ZID_DRIVETRAIN
#endif

#endif
