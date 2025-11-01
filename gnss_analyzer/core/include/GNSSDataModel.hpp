#pragma once
#include <QDateTime>
#include <QString>
#include <QMap>

enum class DATAType : short
{
    Unknown = 0,
    GGA = 1,
    GSV = 2,
};

struct SATInfo{
    double elevation;
    double azimuth;
    double snr;
};

struct GNSSData {
    u_int8_t satellites = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double snrAvg = 0.0;
    double hdop = 0.0;
    double vdop = 0.0;
    QMap <int, SATInfo> satMap;
    QString fixType = "No fix";
    QDateTime timestamp;   
};