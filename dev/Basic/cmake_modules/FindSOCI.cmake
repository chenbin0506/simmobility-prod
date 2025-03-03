## Find SOCI
##
## Copyright (c) 2010 Jamie Jones <jamie_jones_au@yahoo.com.au>
## This file is licensed under the GNU GPLv2 or any later versions,
##
## This module defines
## SOCI_LIBRARIES
## SOCI_FOUND, if false, do not try to link to SOCI
## SOCI_INCLUDE_DIRS, where to find the headers
##

if (SOCI_INCLUDE_DIRS AND SOCI_LIBRARIES)
  #Do nothing; the cached values of these already exist

else ()

  FIND_PATH(SOCI_INCLUDE_DIRS soci.h
      ${SOCI_DIR}/include
      ${SOCI_DIR}/include/soci
      $ENV{SOCI_DIR}/include
      $ENV{SOCI_DIR}/include/soci
      $ENV{SOCI_DIR}
      C:/MinGW/include
      C:/MinGW/include/soci
      C:/MinGW32/include
      C:/MinGW32/include/soci
      ~/Library/Frameworks
      /home/xuyan/xuyan_SimMobility/soci/include
      /home/xuyan/xuyan_SimMobility/soci/include/soci
      /Library/Frameworks
      /usr/local/include
      /usr/local/include/soci
      /usr/include
      /usr/include/soci
      /sw/include # Fink
      /opt/local/include # DarwinPorts
      /opt/local/include/soci # MacPorts location
      /opt/csw/include # Blast wave
      /opt/include
      /afs/csail.mit.edu/u/x/xuyan/lib/soci/soci-3.2.1/include
      /afs/csail.mit.edu/u/x/xuyan/lib/soci/soci-3.2.1/include/soci
  )

  FIND_LIBRARY(SOCI_LIBRARIES
      NAMES soci_core  soci_core-gcc-3_0  soci_core_3_0  soci_core-gcc-3_1  soci_core_3_1 soci_core_3_2
      PATHS
      C:/MinGW/lib
      C:/MinGW32/lib
      ${SOCI_DIR}/lib
      $ENV{SOCI_DIR}/lib
      $ENV{SOCI_DIR}
      ~/Library/Frameworks
      /home/xuyan/xuyan_SimMobility/soci/lib64
      /Library/Frameworks
      /usr/local/lib
      /usr/local/lib64
      /usr/lib
      /sw/lib
      /opt/local/lib
      /opt/csw/lib
      /opt/lib
      /afs/csail.mit.edu/u/x/xuyan/lib/soci/soci-3.2.1/lib64
  )
  

  IF(SOCI_LIBRARIES)
      IF (NOT SOCI_FIND_QUIETLY)
      MESSAGE(STATUS "Found the SOCI library at ${SOCI_LIBRARIES}")
      MESSAGE(STATUS "Found the SOCI headers at ${SOCI_INCLUDE_DIRS}")
      ENDIF (NOT SOCI_FIND_QUIETLY)
  ENDIF(SOCI_LIBRARIES)

  IF(NOT SOCI_LIBRARIES)
      IF (SOCI_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "SOCI Not Found.")
      ENDIF (SOCI_FIND_REQUIRED)
  ENDIF(NOT SOCI_LIBRARIES)

endif (SOCI_INCLUDE_DIRS AND SOCI_LIBRARIES)

