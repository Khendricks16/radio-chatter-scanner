cmake_minimum_required(VERSION 3.23)

project(rcs)


# Main program executable
add_executable(rcs)


# Written components that rcs depend upon
target_sources(rcs
    PRIVATE
    src/main.c
    src/radio.c
    src/input.c
    src/db.c
)

# ADD Sqlite library
find_package(SQLite3)
target_link_libraries(project_target PRIVATE SQLite::SQLite3)

# ADD rtlsdr library
find_library(rtlsdr)
#find_path(RTLSDR_INCLUDE_DIR rtl-sdr.h REQUIRED)

#target_include_directories(rcs PRIVATE ${RTLSDR_INCLUDE_DIR})
#target_link_libraries(rcs rtlsdr ${RTLSDR_LIB})

