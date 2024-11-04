#ifndef DATAPACKETS_H
#define DATAPACKETS_H


enum PacketType{
    CHASSIS_TELEMETRY,
    CHASSIS_CONTROL,
    CHASSIS_CONFIG,
    CANNON_TELEMETRY,
    CANNON_CONTROL,
    CANNON_CONFIG,
    LED_TELEMETRY,
    LED_CONTROL,
    LED_CONFIG
};

/**
 * Used for defining radio sender/reciever addresses
 * and filtering messages based on target system
 * Version can be updated to invalidate messages to systems
 * that may not understand radio version
 */
// union{
//     struct data{
//         uint_8 version:8;
//         enum system{
//             CHASSIS,CONTROLLER,CANNON,LED
//         }:8;
//     uint_8 address;
//     }
// } SystemID;

struct Metadata{
    PacketType type: 4;
    uint8_t heartbeat: 4;
};
#define METADATA_SIZE_BYTES 1

// Common definitions for sender/recievers

/// Representation of a shifter state'
enum ChassisGear{
    Low=0,High=1
};
#define CHASSIS_GEAR_SIZE_BYTES 1

/** Representative Wheel Speeds in in/s
 * Holds a maximum of +/-127in/s
 * Not recommended as an intermediary for math due to overflow/underflow issues
 */
struct ChassisSpeeds{
    int8_t left: 8;
    int8_t right: 8;
};
#define CHASSIS_SPEEDS_SIZE_BYTES 2

///
/// Chassis packet definitions
/// 

struct ChassisTelemetry{
    Metadata metadata;
    /** inches/sec*/
    ChassisSpeeds speed;
    /** PSI*/
    uint8_t pressure: 8;
    /** Battery voltage, in deciVolts. Divide by 10 to get normal volts */
    uint8_t batteryVoltage: 8;
    /** Shifter state */
    ChassisGear gear: 1;
    /** If the chassis is operating*/
    boolean enable: 1;
};
#define CHASSIS_TELEMETRY_SIZE_BYTES 4

struct ChassisControl{
    Metadata metadata;
    ChassisSpeeds speed;
    ChassisGear gear: 1;
    boolean enable: 1;
};
#define CHASSIS_CONTROL_SIZE_BYTES 4

/**Allow remote systems to save some states
* This allows potential for re-configuration in different terrains
* or with different modules on top
*/
struct ChassisConfig{
    /// The physical gear arrangement expected for this configuration
    ChassisGear gear;
    /// The encoder ticks per inch travelled
    int encoderRatio;
    /// Max velocity in inches per second
    int maxForwardVelocity;
    /// Max velocity in inches per second
    int maxAngularVelocity;
    /// Feed Forward rate gain
    float kf;
    /// Static gain constant
    float ks;
    /// Proportional gain constant
    float kp;
    /// Integral gain constant
    float ki; //Unused in code
    /// Derivitative gain constant
    float kd; //unused in code
};
#define CHASSIS_CONFIG_SIZE_BYTES 4



enum CannonSimpleState{ NOTREADY=0, READY=1, FIRING=2, RELOADING=3 };
enum SolenoidState{ DISENGAGED=0,ENGAGED=1 };

struct CannonTelemetry{
    Metadata metadata;
    /** State of the internal FSM. Only valid if same code version on both systems*/
    uint8_t stateDebug;
    /** Stable, simplified FSM for user feedback*/
    CannonSimpleState state: 4;
    /** Selected barrel position */
    uint8_t position: 4 ;
    /// Bitfield representing each barrel's loaded state
    uint16_t barrelLoaded :10;

    /// Operating pressure (psi)
    uint16_t pressure :10;

    ///Current state of the firing plate
    SolenoidState firingPlate:1;
    /// Current state of the dump valve; This valve is responsible for opening/closing the primary air gate;
    SolenoidState dumpValve:1;
    ///Current state of the index pin
    SolenoidState indexPin:1;
};
#define CANNON_TELEMETRY_SIZE_BYES 5 //TODO Double check this for sanity


struct CannonConfig{

};


#endif
