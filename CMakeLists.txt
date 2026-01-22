cmake_minimum_required(VERSION 3.27)

project(tmsolve)

# Find source files
file(GLOB SOURCES *.c libtmsolve/src/*.c)

# Include header files
include_directories(libtmsolve/include)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

if(LINUX)
    # Linux configuration uses readline
    add_compile_definitions(LOCAL_BUILD USE_READLINE)
    target_link_libraries(${PROJECT_NAME} m readline)
    install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)
elseif(MSYS)
    add_compile_definitions(LOCAL_BUILD USE_READLINE)
    # Bundle readline, ncurses and winpthread in the executable to keep it portable
    # ncurses is an indirect dependency of tmsolve (it is a dependency of readline)
    target_link_libraries(${PROJECT_NAME} -static readline ncurses winpthread)
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
