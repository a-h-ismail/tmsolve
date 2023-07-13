cmake_minimum_required(VERSION 3.4.0)

project(tmsolve)

# Find source files
file(GLOB SOURCES *.c libtmsolve/src/*.c)

# Include header files
include_directories(libtmsolve/include)

add_compile_options(-O2)

add_compile_definitions(LOCAL_BUILD USE_READLINE)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link to standard math library and readline
target_link_libraries(${PROJECT_NAME} m readline)

# Install binary
install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})