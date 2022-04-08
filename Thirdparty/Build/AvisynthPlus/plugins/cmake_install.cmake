# Install script for directory: H:/Kainote/Thirdparty/AviSynthPlus/plugins

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/AviSynth+")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("H:/Kainote/Thirdparty/Build/AvisynthPlus/plugins/DirectShowSource/cmake_install.cmake")
  include("H:/Kainote/Thirdparty/Build/AvisynthPlus/plugins/VDubFilter/cmake_install.cmake")
  include("H:/Kainote/Thirdparty/Build/AvisynthPlus/plugins/ImageSeq/cmake_install.cmake")
  include("H:/Kainote/Thirdparty/Build/AvisynthPlus/plugins/TimeStretch/cmake_install.cmake")
  include("H:/Kainote/Thirdparty/Build/AvisynthPlus/plugins/Shibatch/cmake_install.cmake")
  include("H:/Kainote/Thirdparty/Build/AvisynthPlus/plugins/ConvertStacked/cmake_install.cmake")

endif()

