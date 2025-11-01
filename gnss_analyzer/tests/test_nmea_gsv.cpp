#include <QtTest>
#include "NMEAParser.hpp"
#include "NMEAException.hpp"

class TestNMEAParserGGA : public QObject {
    Q_OBJECT

private slots:
    void test_parseGSV_data()
    {
        
    }

    void test_parseGSV()
    {
        qWarning() << "TODO";
    }

};

QTEST_MAIN(TestNMEAParserGGA);
#include "test_nmea_gsv.moc"