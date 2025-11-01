#include "NMEAParser.hpp"
#include "NMEAException.hpp"
#include <QtMath>
#include <QDebug>
#include <QTime>

static QMap<int, SATInfo> gsvTempSatellites;
static int expectedGSVParts = 0;


namespace NMEAParser {

    void parseGGA(const QStringList &tokens, GNSSData &data)
    {
        // Ex: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,,*47
        // Fields:
        //  0 = $GPGGA
        //  1 = UTC Time (hhmmss.sss)
        //  2 = Latitude (ddmm.mmmm)
        //  3 = N/S
        //  4 = Longitude (dddmm.mmmm)
        //  5 = E/W
        //  6 = Fix quality (0=Invalid, 1=GPS fix, 2=DGPS fix)
        //  7 = Number of satellites
        //  8 = HDOP
        //  9 = Altitude (meters)
        // 10 = 'M' (meters)
        // 11 = Height of geoid (optionnel)
        // 12 = 'M'
        // 13 = (optional) time since last DGPS update
        // 14 = (optional) DGPS station ID
        try {
            // --- UTC ---
            if (tokens.size() < 10)
            {
                throw ParsingError("GGA frame too short: expected >=10 fields");
            }

            QString timeStr = tokens[1];
            if (timeStr.size() < 6 )
            {
                throw InvalidDataError("Invalid UTC time in GGA frame");
            }
            int hour = timeStr.mid(0, 2).toInt();
            int minute = timeStr.mid(2, 2).toInt();
            int second = timeStr.mid(4, 2).toInt();
            QTime time(hour, minute, second);
            if (time.isValid()) {
                data.timestamp = QDateTime(QDate::currentDate(), time, Qt::UTC);
            }
            
            // --- Latitude ---
            data.latitude = convertToDecimalDegrees(tokens[2], tokens[3]);

            // --- Longitude ---
            data.longitude = convertToDecimalDegrees(tokens[4], tokens[5]);
            if (data.longitude == -qInf())
            {
                throw InvalidDataError("Longitude conversion failed");
            }

            // --- Fix type ---
            int fixQuality = tokens[6].toInt();
            switch (fixQuality)
            {
                case 0: data.fixType = "No Fix"; break;
                case 1: data.fixType = "GPS Fix"; break;
                case 2: data.fixType = "DGPS Fix"; break;
                case 4: data.fixType = "RTK Fix"; break;
                default:
                    throw InvalidDataError(QString("Unknown fix quality code: %1").arg(fixQuality));
            }

            // --- Nb of satellites ---
            data.satellites = tokens[7].toInt();
            if (data.satellites < 0 || data.satellites > 50)
            {
                throw InvalidDataError("Number of satellites out of range");
            }
            
            // --- HDOP ---
            data.hdop = tokens[8].toDouble();
            if (data.hdop <= 0 || data.hdop > 50.0)
            {
                throw InvalidDataError("HDOP value out of range");
            }
            
            // --- Altitude ---
            data.altitude = tokens[9].toDouble();
            if (data.altitude < -500 || data.altitude > 10000)
            {
                throw InvalidDataError("Altitude out of realistic bounds");
            }
        } catch (const NMEAException &e) {
            qWarning() << "[parseGGA] Exception:" << e.what();
            throw;
        }
    }

    /**
     * @brief Parse RMC (Recommended Minimum Navigation Information) sentence.
     *
     * Example:
     *   $GPRMC,130559.00,A,4517.27361,N,00552.34637,E,0.018,,220623,,,A*6C
     *
     * Fields:
     *   1 - UTC Time (hhmmss.sss)
     *   2 - Status (A=active, V=void)
     *   3 - Latitude (ddmm.mmmm)
     *   4 - N/S Indicator
     *   5 - Longitude (dddmm.mmmm)
     *   6 - E/W Indicator
     *   7 - Speed over ground (knots)
     *   8 - Track angle (degrees)
     *   9 - Date (ddmmyy)
     *  10 - Magnetic variation
     *  11 - Magnetic variation E/W
     *  12 - Mode (A=Autonomous, D=Differential, etc.)
     */
    /*static void parseRMC(const QStringList &tokens, GNSSData &data)
    {    
    }
    */

    /**
     * @brief Parse GSV (Satellites in View) sentence.
     *
     * Example:
     *   $GPGSV,3,1,12,02,65,290,42,04,40,150,38,09,55,050,44,12,32,200,36*7A
     *
     * Fields:
     *   1 - Total number of GSV messages
     *   2 - Message number (1..N)
     *   3 - Total satellites in view
     *   4+ - Repeated blocks of 4 values per satellite:
     *        [PRN number, elevation (°), azimuth (°), SNR (dB-Hz)]
     */
    void parseGSV(const QStringList &tokens, GNSSData &data)
    {
        try
        {
            if (tokens.size() < 4)
            {
                throw ParsingError("GSV frame too short: expected >=4 fields");
            }

            int totalMsgs = tokens[1].toInt();
            int msgNum = tokens[2].toInt();
            int totalSats = tokens[3].toInt();

            // Reset temporary storage when starting a new sequence
            if (msgNum == 1)
            {
                gsvTempSatellites.clear();
                expectedGSVParts = totalMsgs;
            }

            for (int i = 4; i+3 < tokens.size(); i+4){
                if (i + 3 >= tokens.size())
                // Ensure that all required tokens exist before reading
                if (i + 3 >= tokens.size())
                    break;

                bool okId = false, okElev = false, okAzim = false, okSnr = false;

                int id         = tokens[i].toInt(&okId);
                double elev    = tokens[i + 1].toDouble(&okElev);
                double azimuth = tokens[i + 2].toDouble(&okAzim);
                double snr     = tokens[i + 3].toDouble(&okSnr);

                if (!okId || id <= 0)
                {
                    continue; // invalid satellite ID
                }

                SATInfo info;
                info.elevation = okElev ? elev : -qInf();
                info.azimuth   = okAzim ? azimuth : -qInf();
                info.snr       = okSnr ? snr : -qInf();
                gsvTempSatellites[id] = info;  
            }
            
        } catch (const NMEAException &e)
        {
            qWarning() << "[parseGSV] Exception:" << e.what();
            throw;
        }
        
    }

    /**
     * @brief Parse GSA (GNSS DOP and Active Satellites) sentence.
     *
     * Example:
     *   $GPGSA,A,3,04,05,09,12,24,25,29,31,,,,,1.8,1.0,1.4*30
     *
     * Fields:
     *   1 - Mode (M=Manual, A=Automatic)
     *   2 - Fix type (1=No fix, 2=2D, 3=3D)
     *   3-14 - PRNs of satellites used (optional)
     *  15 - PDOP
     *  16 - HDOP
     *  17 - VDOP
     */
    /*
    static void parseGSA(const QStringList &tokens, GNSSData &data)
    {

    }*/
    double convertToDecimalDegrees(const QString &value, const QString &direction)
    {
        
        if (value.isEmpty() || direction.isEmpty()) {
            throw InvalidDataError("InvalidData: Empty latitude/longitude or direction");
        }

        int degDigits = (direction == "N" || direction == "S") ? 2 : 3;

        if (value.size() < degDigits) {
            throw InvalidDataError("InvalidData: String too short for degrees");
        }

        bool okDeg = false;
        bool okMin = false;

        int degreePart = value.left(degDigits).toInt(&okDeg);
        
        double minutePart = value.mid(degDigits).toDouble(&okMin);
        
        if (!okDeg || !okMin) {
            throw InvalidDataError("InvalidData: Conversion failed");
        }

        double decimalDegrees = degreePart + (minutePart / 60.0);
        if (direction == "S" || direction == "W") {
            decimalDegrees = -decimalDegrees;
        }

        return decimalDegrees;
    }


    DATAType DataType(const QString &line)
    {
        if (line.startsWith("$GPGGA"))
        {
            return DATAType::GGA;
        }
        else if (line.startsWith("$GPGSV"))
        {
            return DATAType::GSV;
        }else
        {
            return DATAType::Unknown;
        }
        
    }

    void parseLine(const QString &line, GNSSData& data)
    {
        switch (DataType(line))
        {
            case DATAType::GGA:
            {
                auto parts = line.split(",");
                parseGGA(parts, data);
                break;
            }
            case DATAType::GSV:
            {
                auto parts = line.split(",");
                parseGSV(parts, data);
                break;
            }
            default:
                break;
        }
    }
};