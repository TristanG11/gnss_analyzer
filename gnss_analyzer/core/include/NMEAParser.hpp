#include "GNSSDataModel.hpp"

namespace NMEAParser {
    double convertToDecimalDegrees(const QString &value, const QString &direction);
    DATAType DataType(const QString &line);
    void parseGGA(const QStringList &tokens, GNSSData &data);
    void parseGSV(const QStringList &tokens, GNSSData &data);
    void parseLine(const QString &line, GNSSData& data);
};

