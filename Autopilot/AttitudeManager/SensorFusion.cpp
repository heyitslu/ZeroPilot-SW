/*
* Sensor Fusion Algorithms - uses Madgwick/Kalmann/etc. tbd PLEASE EDIT THIS AND ADD MORE INFO :))))))))) 
* Author: Lucy Gong
*/
#include "SensorFusion.hpp"
#include "IMU.hpp"
#include "airspeed.hpp"
#include "MadgwickAHRS.h"
#include <math.h>

IMUData_t imudata;
airspeedData_t airspeeddata;

// ICM20602 imusns;
// dummyairspeed airspeedsns;

SFError_t SF_GetResult(SFOutput_t *Output, IMU *imusns, airspeed *airspeedsns){
    

    //Error output
    SFError_t SFError;

    SFError.errorCode = 0;

    //IMU integration outputs
    float imu_RollAngle = 0;
    float imu_PitchAngle = 0;
    float imu_YawAngle = 0;

    float imu_RollRate = 0;
    float imu_PitchRate = 0;
    float imu_YawRate = 0;

    //Retrieve raw IMU and Airspeed data
    imusns->GetResult(imudata);
    airspeedsns->GetResult(airspeeddata);

    //Abort if both sensors are busy or failed data collection
    if(imudata.sensorStatus != 0 || airspeeddata.sensorStatus != 0)
    {  
        SFError.errorCode = -1;
        return SFError;
    }

    //Check if data is old
    if(!imudata.isDataNew || !airspeeddata.isDataNew){
        SFError.errorCode = 1;
    }

    MadgwickAHRSupdate(imudata.gyrx, imudata.gyry, imudata.gyrz, imudata.accx, imudata.accy, imudata.accz, imudata.magx, imudata.magy, imudata.magz);

    //Convert quaternion output to angles (in deg)
    imu_RollAngle = atan2f(q0 * q1 + q2 * q3, 0.5f - q1 * q1 - q2 * q2) * 57.29578f;
    imu_PitchAngle = asinf(-2.0f * (q1 * q3 - q0 * q2)) * 57.29578f;
    imu_YawAngle = atan2f(q1 * q2 + q0 * q3, 0.5f - q2 * q2 - q3 * q3) * 57.29578f + 180.0f;

    //Convert rate of change of quaternion to angular velocity (in deg/s)
    imu_RollRate = atan2f(qDot1 * qDot2 + qDot3 * qDot4, 0.5f - qDot2 * qDot2 - qDot3 * qDot3) * 57.29578f;
    imu_PitchRate = asinf(-2.0f * (qDot2 * qDot4 - qDot1 * qDot3)) * 57.29578f;
    imu_YawRate = atan2f(qDot2 * qDot3 + qDot1 * qDot4, 0.5f - qDot3 * qDot3 - qDot4 * qDot4) * 57.29578f + 180.0f;

    //Transfer Fused IMU data into SF Output struct
    Output->IMUpitch = imu_PitchAngle;
    Output->IMUroll = imu_RollAngle;
    Output->IMUyaw = imu_YawAngle;

    Output->IMUpitchrate = imu_PitchRate;
    Output->IMUrollrate = imu_RollRate;
    Output->IMUyawrate = imu_YawRate;

    //Transfer Airspeed data
    Output->Airspeed = airspeeddata.airspeed;

    return SFError;
}

