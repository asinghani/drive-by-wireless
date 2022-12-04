#include <string.h>
#include <string.h>

// Shared datatype definitions
#ifndef TYPES_H
#define TYPES_H

typedef struct cockpit_pkt {
    uint8_t src;
    uint8_t dst;

    int8_t angle;
    uint8_t throttle;
    bool brake;

    int32_t current_ts;
    int32_t blinker_basis_ts;
    bool blink_left;
    bool blink_right;

    bool is_failure_state;

} cockpit_pkt_t;

typedef struct steering_pkt {
    uint8_t src;
    uint8_t dst;

    int8_t feedback;

} steering_pkt_t;

typedef struct drivetrain_pkt {
    uint8_t src;
    uint8_t dst;

    float voltage_vbat;
    float voltage_vreg;

} drivetrain_pkt_t;

#endif
