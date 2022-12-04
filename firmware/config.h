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

// Maximum packet loss before failure state
#define MAX_COMM_FAILURES 5
#define COMM_TIMEOUT_MS 80

// Blinker rates
#define ERROR_BLINKER_RATE_MS 150
#define BLINKER_RATE_MS 500

// Blinker control config
#define STEER_THRESH1 30
#define STEER_THRESH2 60

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
#define STEERING_LED_STRIP_IO IO_7

// Drivetrain zone config
#define DRIVETRAIN_EN  IO_8
#define DRIVETRAIN_IN1 IO_9
#define DRIVETRAIN_IN2 IO_10
#define VMON_VBAT IO_1
#define VMON_VBAT_ADC 2 // ch2 = gpio28
#define VMON_5V   IO_2
#define VMON_5V_ADC   1 // ch1 = gpio27

// Calibrated emperically
#define VMON_VBAT_SCALE 0.00618871
#define VMON_5V_SCALE 0.00619207

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

// Test point definitions
#define ZC_TP_USB_IN  1
#define ZC_TP_USB_OUT 2
#define ZC_TP_UWB_RX  3
#define ZC_TP_UWB_TX  4

#define ZS_TP_UWB_RX     1
#define ZS_TP_UWB_TX     2
#define ZS_TP_STEER_SET  3
#define ZS_TP_FORCE_READ 4

#define ZD_TP_UWB_RX     1
#define ZD_TP_UWB_TX     2
#define ZD_TP_MOTOR_SET  3
#define ZD_TP_IS_BRAKE   4

#define ZDZS_TP_BLINK_L  5
#define ZDZS_TP_BLINK_R  6

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
