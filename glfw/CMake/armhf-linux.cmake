# Define the environment for cross-compiling with 32-bit arm linux
SET(CMAKE_SYSTEM_NAME    Linux) # Target system name
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_C_COMPILER     "arm-linux-gnueabihf-gcc")
SET(CMAKE_CXX_COMPILER   "arm-linux-gnueabihf-g++")
# SET(CMAKE_RC_COMPILER    "i686-w64-mingw32-windres")
SET(CMAKE_RANLIB         "arm-linux-gnueabihf-ranlib")

# Configure the behaviour of the find commands
SET(CMAKE_FIND_ROOT_PATH "/usr/arm-linux-gnueabihf")
#SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
