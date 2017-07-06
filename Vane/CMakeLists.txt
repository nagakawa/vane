PROJECT(Vane)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(../dependencies.cmake)

SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET(Vane_executable_SRCS
  ${PROJECT_SOURCE_DIR}/main.cpp
  ${PROJECT_SOURCE_DIR}/commands.cpp
  ${PROJECT_SOURCE_DIR}/ipa.cpp
  ${PROJECT_SOURCE_DIR}/baseconv.cpp
  ${PROJECT_SOURCE_DIR}/utf8.cpp
  ${PROJECT_SOURCE_DIR}/filter.cpp
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_INCLUDE_DIR})
# actual target:
ADD_EXECUTABLE(Vane ${Vane_executable_SRCS})

TARGET_LINK_LIBRARIES(Vane
  ${CMAKE_SOURCE_DIR}/build/libdiscordpp.so
  #${CMAKE_SOURCE_DIR}/build/libcurlpp.so
  #${ZLIB_LIBRARIES}
  #${LIBPTHREAD_LIBRARIES}
  ${Boost_LIBRARIES}
  curlpp curl pthread ssl crypto
  #${SQLITE_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  # ${CURL_LIBRARIES}
)

#ADD_CUSTOM_COMMAND(
#  TARGET x801
#  POST_BUILD
#  COMMAND cp -r ../intrinsic-assets ../build
#)

# add install target:
INSTALL(TARGETS Vane DESTINATION bin)
