cmake_minimum_required(VERSION 2.6)
project(SERIAL C)

include_directories(
  ${PROJECT_SOURCE_DIR}/src
)

add_library(serial
  src/serial/Device.c
  src/serial/Base64.c
)
target_link_libraries(serial)

add_executable(sample
  src/sample/main.c
)
target_link_libraries(sample serial)

add_executable(base64
  src/tests/base64/main.c
)
target_link_libraries(base64 serial)
