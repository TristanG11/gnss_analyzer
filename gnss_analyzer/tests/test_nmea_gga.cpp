#include <QtTest>
#include "NMEAParser.hpp"
#include "NMEAException.hpp"

class TestNMEAParserGGA : public QObject {
    Q_OBJECT

private slots:

    // --- Test Data ---
    void test_parseGGA_data() 
    {
        QTest::addColumn<QString>("nmeaLine");
        QTest::addColumn<double>("expectedLat");
        QTest::addColumn<double>("expectedLon");
        QTest::addColumn<int>("expectedSat");
        QTest::addColumn<QString>("expectedFix");
        QTest::addColumn<double>("expectedAlt");
        QTest::addColumn<double>("expectedHdop");
        QTest::addColumn<bool>("expectException");

        QTest::newRow("basic_GPS_fix") 
            << "$GPGGA,123519,4807.038,N,11131.000,E,1,08,0.9,545.4,M,,*47"
            << 48.1173 << 111.517 << 8 << "GPS Fix" << 545.4 << 0.9 << false;

        QTest::newRow("different_pos") 
            << "$GPGGA,102030,5123.456,N,00012.345,E,1,10,1.2,120.0,M,,*5C"
            << 51.391 << 0.20575 << 10 << "GPS Fix" << 120.0 << 1.2 << false;;

        QTest::newRow("no_fix")
            << "$GPGGA,094500,,,,,0,00,99.9,,,,,,*48"
            << -qInf() << -qInf() << 0 << "No fix" << 0.0 << 99.9 << true;
    }


    void test_parseGGA() {
        QFETCH(QString, nmeaLine);
        QFETCH(double, expectedLat);
        QFETCH(double, expectedLon);
        QFETCH(int, expectedSat);
        QFETCH(QString, expectedFix);
        QFETCH(double, expectedAlt);
        QFETCH(double, expectedHdop);
        QFETCH(bool, expectException);

        GNSSData data;
        bool exceptionCaught = false;
        QString errorMsg;

        // --- Try parsing ---
        try {
            auto parts = nmeaLine.split(",");
            NMEAParser::parseGGA(parts, data);
        } catch (const InvalidDataError &e) {
            exceptionCaught = true;
            errorMsg = e.what();
            qDebug() << "Caught exception:" << errorMsg;
        } catch (const ParsingError &e) {
            exceptionCaught = true;
            errorMsg = e.what();
            qDebug() << "Caught exception:" << errorMsg;
        } catch (const std::exception &e) {
            exceptionCaught = true;
            errorMsg = e.what();
            qDebug() << "Caught std::exception:" << errorMsg;
        }

        if (expectException) {
            QVERIFY2(exceptionCaught,
                     qPrintable(QString("Expected exception but none caught for line: %1").arg(nmeaLine)));
            QVERIFY(errorMsg.contains("InvalidData") || errorMsg.contains("Empty latitude"));
            return; // Stop here, no need to verify data fields
        } else {
            QVERIFY2(!exceptionCaught,
                     qPrintable(QString("Unexpected exception: %1").arg(errorMsg)));
        }

        // --- Timestamp ---
        QVERIFY(data.timestamp.isValid());

        // --- Position ---
        if (qIsInf(expectedLat))
            QVERIFY(qIsInf(data.latitude));
        else
            QVERIFY(qAbs(data.latitude - expectedLat) < 0.002);

        if (qIsInf(expectedLon))
            QVERIFY(qIsInf(data.longitude));
        else
            QVERIFY(qAbs(data.longitude - expectedLon) < 0.002);

        // --- Fix quality / type ---
        QCOMPARE(data.fixType, expectedFix);

        // --- Satellite count ---
        QCOMPARE(data.satellites, expectedSat);

        // --- HDOP ---
        QVERIFY(qAbs(data.hdop - expectedHdop) < 0.001);

        // --- Altitude ---
        QVERIFY(qAbs(data.altitude - expectedAlt) < 0.001);
    }
};

QTEST_MAIN(TestNMEAParserGGA)
#include "test_nmea_gga.moc"
