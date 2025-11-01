# core/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Create a library for the core logic

add_library(gnsscore
    src/GNSSDataModel.cpp
    src/NMEAParser.cpp
)

target_include_directories(gnsscore PUBLIC include)
target_link_libraries(gnsscore PUBLIC Qt5::Core)
