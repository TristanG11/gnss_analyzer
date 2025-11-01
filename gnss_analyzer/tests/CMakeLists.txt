cmake_minimum_required(VERSION 3.16)


add_executable(GNSSAnalyzerTests
    test_nmea_gga.cpp
)

# Liens nécessaires
target_link_libraries(GNSSAnalyzerTests
    PRIVATE
    gnsscore
    Qt5::Core
    Qt5::Test
)


add_test(NAME GNSSAnalyzerTests COMMAND GNSSAnalyzerTests)
