#ifndef DATAPACKETS_H
#define DATAPACKETS_H


enum PacketType{
    CHASSIS_TELEMETRY,
    CHASSIS_CONTROL,
    CANNON_TELEMETRY,
    CANNON_CONTROL,
    LED_TELEMETRY,
    LED_CONTROL
};

struct Metadata{
    PacketType type: 4;
    uint8_t heartbeat: 4;
};

// Common definitions for sender/recievers

// Representation of the shifter gear
enum ChassisGear{
    Low=0,High=1
};

/** Representative Wheel Speeds in in/s */
struct ChassisSpeeds{
    int8_t left: 8;
    int8_t right: 8;
};

///
/// Chassis packet definitions
/// 

struct ChassisTelemetry{
    Metadata metadata;
    /** inches/sec*/
    ChassisSpeeds speed;
    /** PSI*/
    uint8_t pressure: 8;
    /** deciVolts */
    uint8_t batteryVoltage: 8;
    ChassisGear gear: 1;
    boolean enable: 1;
};

struct ChassisControl{
    Metadata metadata; 
    ChassisSpeeds speed;
    ChassisGear gear: 1;
    boolean enable: 1;
};
#define CHASSIS_CONTROL_SIZE 32




enum CannonSimpleState{ NOTREADY=0, READY=1, FIRING=2, RELOADING=3 };
enum SolenoidState{ DISENGAGED=0,ENGAGED=1 };

struct CANNON_TELEMETRY{
    Metadata metadata;
    /** State of the internal FSM. Only valid if same code version on both systems*/
    uint8_t stateDebug;
    /** Stable, simplified FSM for user feedback*/
    CannonSimpleState state: 4;
    /** Selected barrel position */
    uint8_t position: 4 ;
    /**Bitfield representing each barrel's loaded state*/
    uint16_t barrelLoaded :10;


    uint16_t pressure :10;

    SolenoidState firingPlate:1;
    SolenoidState dumpValve:1;
    SolenoidState indexPin:1;
};



#endif