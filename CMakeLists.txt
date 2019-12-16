cmake_minimum_required(VERSION 2.6)
project(SERIAL C)

include_directories(
  ${PROJECT_SOURCE_DIR}/src
)

add_library(serial
  src/serial/Device.c
  src/serial/Base64.c
  src/serial/Stream.c
  src/serial/sha1.c
  src/serial/Hash.c
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

add_executable(stream
  src/tests/stream/main.c
)
target_link_libraries(stream serial)
