cmake_minimum_required (VERSION 3.18)
project (rovergauge)

set (ROVERGAUGE_VER_MAJOR 11)
set (ROVERGAUGE_VER_MINOR 0)
set (ROVERGAUGE_VER_PATCH 0)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${ROVERGAUGE_VER_MAJOR}.${ROVERGAUGE_VER_MINOR}.${ROVERGAUGE_VER_PATCH}-src")
set (CPACK_SOURCE_IGNORE_FILES
      ".*~$"
      ".git"
      ".gitignore"
      ".*swp$"
      ".*zip$"
      ".*gz$"
      ".*deb$"
      "Makefile.*"
      "moc_"
      "qrc_"
      "^CPack.*"
      "_CPack_Packages"
      "CPackConfig.cmake"
      "CPackSourceConfig.cmake"
      "CMakeFiles"
      "CMakeCache.txt"
      "CMakeLists.txt.user*"
      "cmake_install.cmake"
      "install_manifest.txt"
      "README.TXT"
      "LICENSE.TXT"
      "build*/"
      "settings.ini"
      "debug"
      "release"
      "object_script.*")

find_package (Qt6 COMPONENTS Core Widgets REQUIRED)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS} -s")
endif ()

include_directories ("${CMAKE_SOURCE_DIR}/src"
                     "${CMAKE_SOURCE_DIR}/src/qledindicator"
                     "${CMAKE_SOURCE_DIR}/src/analogwidgets"
                     "${CMAKE_CURRENT_BINARY_DIR}"
                     "${Qt6Widgets_INCLUDE_DIRS}")

qt6_wrap_ui (UI_SOURCE src/mainwindow.ui)
qt6_wrap_ui (UI_SOURCE src/optionsdialog.ui)
qt6_wrap_ui (UI_SOURCE src/batterybackeddisplay.ui)
qt6_wrap_ui (UI_SOURCE src/idleaircontroldialog.ui)
message (STATUS "Generated: ${UI_SOURCE}")
qt6_add_resources (RG_RESOURCE rovergauge_resources.qrc)
message (STATUS "Generated: ${RG_RESOURCE}")

add_definitions ("-DROVERGAUGE_VER_MAJOR=${ROVERGAUGE_VER_MAJOR}"
                 "-DROVERGAUGE_VER_MINOR=${ROVERGAUGE_VER_MINOR}"
                 "-DROVERGAUGE_VER_PATCH=${ROVERGAUGE_VER_PATCH}")

add_executable (rovergauge
    src/main.cpp
    src/simulatedecudata.cpp
    src/simulatedecudata.h
    src/cuxinterface.cpp
    src/cuxinterface.h
    src/helpviewer.cpp
    src/helpviewer.h
    src/idleaircontroldialog.cpp
    src/idleaircontroldialog.h
    src/logger.cpp
    src/logger.h
    src/serialdevenumerator.cpp
    src/serialdevenumerator.h
    src/fuelmapgrid.cpp
    src/fuelmapgrid.h
    src/fueltrimbar.cpp
    src/fueltrimbar.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/faultcodedialog.cpp
    src/faultcodedialog.h
    src/aboutbox.cpp
    src/aboutbox.h
    src/optionsdialog.cpp
    src/optionsdialog.h
    src/batterybackeddisplay.cpp
    src/batterybackeddisplay.h
    src/qledindicator/qledindicator.cpp
    src/qledindicator/qledindicator.h
    src/analogwidgets/led.cpp
    src/analogwidgets/led.h
    src/analogwidgets/functions.cpp
    src/analogwidgets/functions.h
    src/analogwidgets/widgetwithbackground.cpp
    src/analogwidgets/widgetwithbackground.h
    src/analogwidgets/manometer.cpp
    src/analogwidgets/manometer.h
    src/analogwidgets/abstractmeter.cpp
    src/analogwidgets/abstractmeter.h
    ${UI_SOURCE}
    ${RG_RESOURCE}
    rovergauge.rc)

message (STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

if (MINGW)
  message (STATUS "Found Windows/MinGW platform.")

  # prevent the final executable from launching a terminal window in parallel with the main GUI
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")

  # -fPIC appears to be redundant when building win32 binaries, so disable that flag
  string (REGEX REPLACE "-fPIC" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

  # When we call find_library() below, we're only looking for DLLs, so set the suffix list appropriately
  set (CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
  message (STATUS "Limiting find_library() suffixes to ${CMAKE_FIND_LIBRARY_SUFFIXES}")

  find_library (LIBGCC NAMES libgcc_s_sjlj-1 libgcc_s_seh-1 libgcc_s_dw2-1)
  if (LIBGCC)
    message (STATUS "Found libgcc at ${LIBGCC}")
  else ()
    message (SEND_ERROR "Could not find libgcc!")
  endif ()

  find_library (LIBSTDCPP libstdc++-6)
  if (LIBSTDCPP)
    message (STATUS "Found libstdc++ at ${LIBSTDCPP}")
  else ()
    message (SEND_ERROR "Could not find libstdc++!")
  endif ()

  find_library (LIBWINPTHREAD libwinpthread-1)
  if (LIBWINPTHREAD)
    message (STATUS "Found libwinpthread at ${LIBWINPTHREAD}")
  else ()
    message (SEND_ERROR "Could not find libwinpthread!")
  endif ()

  find_library (LIBZSTD libzstd)
  if (LIBZSTD)
    message (STATUS "libzstd location is ${LIBZSTD}")
  else ()
    message (SEND_ERROR "Could not find libzstd!")
  endif ()

  get_target_property (QT6CORE_LIB Qt6::Core LOCATION)
  if (QT6CORE_LIB)
    message (STATUS "Qt6::Core location is ${QT6CORE_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt6Core library!")
  endif ()

  get_target_property (QT6WIDGETS_LIB Qt6::Widgets LOCATION)
  if (QT6WIDGETS_LIB)
    message (STATUS "Qt6::Widgets location is ${QT6WIDGETS_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt6Widgets library!")
  endif ()

  get_target_property (QT6GUI_LIB Qt6::Gui LOCATION)
  if (QT6GUI_LIB)
    message (STATUS "Qt6::Gui location is ${QT6GUI_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt6Gui library!")
  endif ()

  get_target_property (QT6WINDOWS_LIB Qt6::QWindowsIntegrationPlugin LOCATION)
  if (QT6WINDOWS_LIB)
    message (STATUS "Qt6::QWindows location is ${QT6WINDOWS_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt6 Windows GUI plugin!")
  endif ()

  get_target_property (QT6WINDOWSVISTA_LIB Qt6::QWindowsVistaStylePlugin LOCATION)
  if (QT6WINDOWSVISTA_LIB)
    message (STATUS "Qt6::QWindowsVistaStyle location is ${QT6WINDOWSVISTA_LIB}")
  else ()
    message (WARNING "Could not find Qt6 Windows Vista style GUI plugin!")
  endif ()

  find_library (COMM14CUX_DLL libcomm14cux)
  if (COMM14CUX_DLL)
    message (STATUS "Found libcomm14cux at ${COMM14CUX_DLL}")
  else ()
    message (SEND_ERROR "Could not find libcomm14cux!")
  endif ()

  target_link_libraries (rovergauge ${COMM14CUX_DLL} Qt6::Widgets)

  # convert Unix-style newline characters into Windows-style
  configure_file ("${CMAKE_SOURCE_DIR}/README.md" "${CMAKE_BINARY_DIR}/README.TXT" NEWLINE_STYLE WIN32)
  configure_file ("${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/LICENSE.TXT" NEWLINE_STYLE WIN32)

  install (FILES "${CMAKE_BINARY_DIR}/rovergauge.exe"
                  ${LIBGCC}
                  ${LIBSTDCPP}
                  ${LIBWINPTHREAD}
                  ${QT6CORE_LIB}
                  ${QT6WIDGETS_LIB}
                  ${QT6GUI_LIB}
                  ${COMM14CUX_DLL}
                  ${LIBZSTD}
           DESTINATION ".")
  install (FILES "${CMAKE_BINARY_DIR}/README.TXT" "${CMAKE_BINARY_DIR}/LICENSE.TXT" DESTINATION "doc")
  install (FILES ${QT6WINDOWS_LIB} DESTINATION "platforms")
  if (QT6WINDOWSVISTA_LIB)
    install (FILES ${QT6WINDOWSVISTA_LIB} DESTINATION "styles")
  endif ()

  set (CPACK_GENERATOR "ZIP")
  set (CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
  set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${ROVERGAUGE_VER_MAJOR}.${ROVERGAUGE_VER_MINOR}.${ROVERGAUGE_VER_PATCH}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
  include (CPack)

else()
  message (STATUS "Defaulting to Linux build environment.")

  target_link_libraries (rovergauge comm14cux Qt6::Widgets)

  set (CMAKE_SKIP_RPATH TRUE)
  set (CMAKE_INSTALL_PREFIX "/usr")

  # set the installation destinations for the header files,
  # shared library binaries, and reference utility
  install (FILES "${CMAKE_CURRENT_BINARY_DIR}/rovergauge"
           DESTINATION "bin"
           PERMISSIONS
            OWNER_READ OWNER_EXECUTE OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)

  install (DIRECTORY DESTINATION "share/doc/rovergauge" DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_EXECUTE OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)

  install (FILES "${CMAKE_SOURCE_DIR}/README.md"
                 "${CMAKE_SOURCE_DIR}/LICENSE"
           DESTINATION "share/doc/rovergauge"
           PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

  # set the architecture flag that will be used by the Debian package builder
  if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
  elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
    set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
  endif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")

  set (CPACK_GENERATOR "DEB")
  set (CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set (CPACK_PACKAGE_VERSION_MAJOR ${ROVERGAUGE_VER_MAJOR})
  set (CPACK_PACKAGE_VERSION_MINOR ${ROVERGAUGE_VER_MINOR})
  set (CPACK_PACKAGE_VERSION_PATCH ${ROVERGAUGE_VER_PATCH})
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Colin Bourassa <colin.bourassa@gmail.com>")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphical display for data read from 14CUX engine management system")
  set (CPACK_DEBIAN_PACKAGE_SECTION "Science")
  set (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.13), libstdc++6 (>= 4.6.3), libcomm14cux (>= 2.1.0), libqt6core6 (>= 6.2.2), libqt6gui6 (>= 6.2.2), libqt6widgets6 (>= 6.2.2)")
  set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${ROVERGAUGE_VER_MAJOR}.${ROVERGAUGE_VER_MINOR}.${ROVERGAUGE_VER_PATCH}-${CMAKE_SYSTEM_NAME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
  set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
  set (CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

  include(CPack)

endif()

