cmake_minimum_required (VERSION 2.8)
project (rovergauge)

set (ROVERGAUGE_VER_MAJOR 0)
set (ROVERGAUGE_VER_MINOR 1)
set (ROVERGAUGE_VER_PATCH 0)

set (CPACK_SOURCE_GENERATOR "TGZ" "ZIP")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${ROVERGAUGE_VER_MAJOR}.${ROVERGAUGE_VER_MINOR}.${ROVERGAUGE_VER_PATCH}-src")
set (CPACK_SOURCE_IGNORE_FILES
      ".*~$"
      ".git"
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
      "cmake_install.cmake"
      "install_manifest.txt"
      "README.TXT"
      "LICENSE.TXT"
      "settings.ini"
      "debug"
      "release"
      "object_script.*")

if (MINGW)
  message (STATUS "Found Windows/MinGW platform.")

  set (CMAKE_INSTALL_PREFIX "C:/comm14cux")
  set (EXE_FILE "release/rovergauge.exe")

  find_library (MINGW_LIBGCC libgcc_s_dw2-1)
  if (MINGW_LIBGCC)
    message (STATUS "Found MinGW's libgcc at ${MINGW_LIBGCC}")
  else ()
    message (SEND_ERROR "Could not find MinGW's libgcc_s_dw2-1.dll!")
  endif ()

  find_library (MINGW_M10 mingwm10)
  if (MINGW_M10)
    message (STATUS "Found MinGW's mingwm10 at ${MINGW_M10}")
  else ()
    message (SEND_ERROR "Could not find MinGW's mingwm10.dll!")
  endif ()

  find_library (QTCORE4_DLL QtCore4)
  if (QTCORE4_DLL)
    message (STATUS "Found QtCore4 at ${QTCORE4_DLL}")
  else ()
    message (SEND_ERROR "Could not find QtCore4.dll!")
  endif ()

  find_library (QTGUI4_DLL QtGui4)
  if (QTGUI4_DLL)
    message (STATUS "Found QtGui4 at ${QTGUI4_DLL}")
  else ()
    message (SEND_ERROR "Could not find QtGui4.dll!")
  endif ()

  find_library(COMM14CUX_DLL libcomm14cux)
  find_library(COMM14CUX_DLL libcomm14cux PATHS ${CMAKE_INSTALL_PREFIX})
  if (COMM14CUX_DLL)
    message (STATUS "Found libcomm14cux at ${COMM14CUX_DLL}")
  else ()
    message (STATUS "Warning: libcomm14cux not found; will not be included in package D")
  endif ()

  # convert Unix-style newline characters into Windows-style
  file (READ "README" README_CONTENTS)
  file (READ "LICENSE" LICENSE_CONTENTS)
  file (WRITE "README.TXT" ${README_CONTENTS})
  file (WRITE "LICENSE.TXT" ${LICENSE_CONTENTS})

  install (FILES ${EXE_FILE} DESTINATION ".")
  install (FILES "README.txt" "LICENSE.txt" DESTINATION "rovergauge_doc")

  install (FILES ${EXE_FILE}
	         ${MINGW_LIBGCC}
		 ${MINGW_M10}
		 ${QTCORE4_DLL}
		 ${QTGUI4_DLL}
		 ${COMM14CUX_DLL}
           DESTINATION ".")

  set (CPACK_GENERATOR "ZIP")
  set (CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
  set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${ROVERGAUGE_VER_MAJOR}.${ROVERGAUGE_VER_MINOR}.${ROVERGAUGE_VER_PATCH}-${CMAKE_SYSTEM_NAME}")
  include (CPack)

else()
  message (STATUS "Platform is not Windows/MinGW; trying Linux...")

  set (CMAKE_SKIP_RPATH TRUE)
  set (CMAKE_INSTALL_PREFIX "/usr")
  set (EXE_FILE "rovergauge")

  # set the installation destinations for the header files,
  # shared library binaries, and reference utility
  install (FILES ${EXE_FILE} DESTINATION "bin"
           PERMISSIONS
            OWNER_READ OWNER_EXECUTE OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)

  install (DIRECTORY DESTINATION "share/doc/rovergauge" DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_EXECUTE OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)

  install (FILES "README"
                 "LICENSE"
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
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Colin Bourassa <lucas.14cux@gmail.com>")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphical display for data read from 14CUX engine management system")
  set (CPACK_DEBIAN_PACKAGE_SECTION "Science")
  set (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1.3), libstdc++6 (>= 4.6.1), libcomm14cux (>= 0.2.0), libqtcore4 (>= 4.7.4), libqtgui4 (>= 4.7.4)")
  set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${ROVERGAUGE_VER_MAJOR}.${ROVERGAUGE_VER_MINOR}.${ROVERGAUGE_VER_PATCH}-${CMAKE_SYSTEM_NAME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
  set (CPACK_RESOURCE_FILE_LICENSE "LICENSE")
  set (CPACK_RESOURCE_FILE_README "README")

  include(CPack)

endif()

