//
// Created by gutou on 2017/4/11.
//

#ifndef WP_ANDROID_ORIENTATIONEKF_H
#define WP_ANDROID_ORIENTATIONEKF_H

#include "Vector3d.h"
#include "Matrix3x3d.h"

class OrientationEKF {
public:
    OrientationEKF();
    virtual ~OrientationEKF();

    void reset();

    __attribute__((unused))
    bool isReady();

    void processGyro(Vector3d gyro, double sensorTimeStamp);
    void processAcceleration(Vector3d acc, double sensorTimeStamp);

    double getHeadingDegrees();

    __attribute__((unused))
    void setHeadingDegrees(double heading);

    void getPredictedGLMatrix(double secondsAfterLastGyroEvent, double* matrix);


private:
    Matrix3x3d _so3SensorFromWorld;
    Matrix3x3d _so3LastMotion;
    Matrix3x3d _mP;
    Matrix3x3d _mQ;
    Matrix3x3d _mR;
    Matrix3x3d _mRAcceleration;
    Matrix3x3d _mS;
    Matrix3x3d _mH;
    Matrix3x3d _mK;
    Vector3d _vNu;
    Vector3d _vZ;
    Vector3d _vH;
    Vector3d _vU;
    Vector3d _vX;
    Vector3d _vDown;
    Vector3d _vNorth;
    double _sensorTimeStampGyro;
    Vector3d _lastGyro;
    double _previousAccelNorm;
    double _movingAverageAccelNormChange;
    double _filteredGyroTimestep;
    bool _timestepFilterInit;
    int _numGyroTimestepSamples;
    bool _gyroFilterValid;
    bool _alignedToGravity;

    __attribute__((unused))
    bool _alignedToNorth;

    void filterGyroTimestep(double timestep);
    void updateCovariancesAfterMotion();

    __attribute__((unused))
    void updateAccelerationCovariance(double currentAccelNorm);
    void accelerationObservationFunctionForNumericalJacobian(Matrix3x3d *so3SensorFromWorldPred, Vector3d *result);
};


#endif //WP_ANDROID_ORIENTATIONEKF_H
