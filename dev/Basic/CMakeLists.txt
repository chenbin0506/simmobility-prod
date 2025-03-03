#Required version of cmake
cmake_minimum_required (VERSION 2.8.8)

#Set our module override policy:
IF (POLICY CMP0017)
  cmake_policy(SET CMP0017 NEW)
ENDIF (POLICY CMP0017)

#Fairly reasonable way of determining if we're on OSX
#If you have an "/Applications" directory on *nix... just why?
IF(EXISTS "/Applications/")
  SET(USING_OSX ON)
ENDIF(EXISTS "/Applications/")

#Force a decent compiler on OSX (gcc is still 4.2 on OSX, and it lacks functionality we need) 
#This MUST go before the project() definition.
IF(USING_OSX)
  #Gcc 4.X?
  IF(EXISTS "/opt/local/bin/g++-mp-4.6")
    SET(CMAKE_C_COMPILER    "/opt/local/bin/gcc-mp-4.6")
    SET(CMAKE_CXX_COMPILER  "/opt/local/bin/g++-mp-4.6")
  ENDIF(EXISTS "/opt/local/bin/g++-mp-4.6")
  
  #Clang? (built from SVN, will override gcc)
  #IF(EXISTS "/usr/local/bin/clang")
  #  SET(CMAKE_C_COMPILER    "/usr/local/bin/clang")
  #  SET(CMAKE_CXX_COMPILER  "/usr/local/bin/clang++")
  #ENDIF(EXISTS "/usr/local/bin/clang")
ENDIF(USING_OSX)

#Project name. Used to tag resources in cmake. 
project (SimMob)

#Turn on verbose output.
SET(CMAKE_VERBOSE_MAKEFILE ON)

#Ensure that all executables get placed in the top-level build directory.
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Also: version number. This will go into the generated header file.
set (Basic_VERSION_MAJOR 0)
set (Basic_VERSION_MINOR 1)

#SET(BUILD_SHARED_LIBRARIES ON)
#SET(CMAKE_EXE_LINKER_FLAGS "-static")

#Function that turns on Unity builds. From:
#   http://cheind.wordpress.com/2009/12/10/reducing-compilation-time-unity-builds/
#Modified: This function expects absolute file paths.
#Modified: Accepts an "exclude list" which it does not modify elements of. (Matches via "ends_with").
function(enable_unity_build UB_SUFFIX SOURCE_VARIABLE_NAME EXCLUDE_LIST)
  set(files ${${SOURCE_VARIABLE_NAME}})
  set(excludes ${${EXCLUDE_LIST}})

  #Generate a unique filename for the unity build translation unit
  set(unit_build_file ${CMAKE_CURRENT_BINARY_DIR}/master_${UB_SUFFIX}.cpp)

  #Force all Source Files to appear as header files.
  set_source_files_properties(${files} PROPERTIES HEADER_FILE_ONLY true)

  #Open the Unity Build file
  FILE(WRITE ${unit_build_file} "//Unity Build generated by CMake\n//Do NOT modify this file.\n")

  #Add include statement for each translation unit
  foreach(source_file ${files} )
    #Determine if this matches any item in the exclude list.
    SET(exclude OFF)
    foreach(exclude_file ${excludes})
      STRING(FIND ${source_file} ${exclude_file} FoundPos REVERSE)
      STRING(LENGTH ${source_file} ExpectPos)
      STRING(LENGTH ${exclude_file} SansPos)
      MATH(EXPR ExpectPos ${ExpectPos}-${SansPos})
      if (FoundPos EQUAL ExpectPos)
        SET(exclude ON)
        break()
      endif()
    endforeach()

    if (exclude) 
      set_source_files_properties(${source_file} PROPERTIES HEADER_FILE_ONLY false)
    else()
      FILE( APPEND ${unit_build_file} "#include \"${source_file}\"\n")
    endif()
  endforeach(source_file)

  #Add the Unity Build file to the list of source files.
  set(${SOURCE_VARIABLE_NAME} ${${SOURCE_VARIABLE_NAME}} ${unit_build_file} PARENT_SCOPE)  
endfunction(enable_unity_build)


#Helpful date macro, from:
#  http://www.cmake.org/pipermail/cmake/2009-February/027014.html
MACRO (TODAY RESULT)
    IF (UNIX OR MINGW)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${RESULT}) 
        string(REGEX REPLACE "(..)/(..)/(....).*" "\\3\\2\\1" ${RESULT} ${${RESULT}})
    ELSEIF(WIN32)
        EXECUTE_PROCESS(COMMAND "date" "/T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/(....).*" "\\3\\2\\1" ${RESULT} ${${RESULT}})
    ELSE ()
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${RESULT} 000000)
    ENDIF ()
ENDMACRO (TODAY)


#Option: build tests. Use the cmake gui to change this on a per-user basis.
option(BUILD_TESTS "Build unit tests." OFF)

#Option: build tests for long term model. Use the cmake gui to change this on a per-user basis.
option(BUILD_TESTS_LONG "Build unit tests." OFF)

#Option: build short term. Use the cmake gui to change this on a per-user basis.
option(BUILD_SHORT "Build short-term simulator." ON)

#Option: build medium term. Use the cmake gui to change this on a per-user basis.
option(BUILD_MEDIUM "Build medium-term simulator." ON)

#Option: build long term. Use the cmake gui to change this on a per-user basis.
option(BUILD_LONG "Build long-term simulator." ON)

#Option: build short/medium/long as libraries. Use the cmake gui to change this on a per-user basis.
option(BUILD_LIBS "If true, builds the short, medium, and long-term code as libraries (to be installed in sim_mob_short,sim_mob_mid,sim_mob_long." OFF)

#Option: Static building SimMobility. Currently static building doesn't work with Ubuntu 16.04
option(BUILD_STATIC "If true, SimMobility is built in static mode." ON)

#Option: Run SimMobility to only produce an input XML file for itself
option(SIMMOB_XML_WRITER "Sim Mobility will print the network as an XML file." OFF)

#Option: USE XML file as SimMobility input. Note that this uses the partial xml reader by default;
#        It is not possible to turn on the old XML readers any more (just set this flag in your CMakeCache file).
#option(SIMMOB_XML_READER "Load from XML, not the database" OFF)

#Option: Turn on Unity (full-file) builds.
option(SIMMOB_FULL_FAST_BUILD_DEBUG "Turn on full/fast builds in debug mode. This will greatly speed up compilation, but will force a full rebuild each time." OFF)
option(SIMMOB_FULL_FAST_BUILD_RELEASE "Turn on full/fast builds in release mode. This will greatly speed up compilation, but will force a full rebuild each time." OFF)

#String "option"; allows easier mangling of XML_READER until we get the new Config syntax working.
#set(SIMMOB_XML_IN_FILE "" CACHE STRING "If non-empty, the XML_READER will attempt to read this instead of whatever is listed in simpleconf.cpp")

#Option: strict error handling. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_STRICT_AGENT_ERRORS  "All unexpected and potentially dangerous behavior causes Agents to throw exceptions. If OFF, Agents will generally just remove themselves from the simulation in case of error." OFF)

#Option: disable output. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_DISABLE_OUTPUT  "Disable all mutex-locked output and all additional non-trivial output. Avoid all use of mutexes w.r.t. output." OFF)

#Option: disable MPI. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_DISABLE_MPI  "Disable all included files, libraries, and code segments that require MPI." ON)

#Option: compile with c++0x. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_LATEST_STANDARD  "Attempt to compile Sim Mobility with the latest version of C++ (2011). Untested." ON)

#Option: profiling flags. Use the cmake gui to change this on a per-user basis.
option(SIMMOB_PROFILE_ON "Turn on profiling. If this flag is off, all SIMMOB_PROFILE_* options will be treated as effectively off." OFF)
option(SIMMOB_PROFILE_AGENT_UPDATES "Log all Agent update ticks per thread and start time. Useful for debugging parallelization errors, but very slow." OFF)
option(SIMMOB_PROFILE_WORKER_UPDATES "Log all Worker update ticks, including start and end time. This can be used to measure the real-world cost of threading, but is only likely accurate if frame ticks are relatively large (~100ms). Also very slow." OFF)
option(SIMMOB_PROFILE_AURAMGR "Log the time taken to update the Aura Manager's spatial index. Usually combined with PROFILE_WORKER_UPDATES. Can be slow." OFF)
option(SIMMOB_PROFILE_COMMSIM "Log the various stages of the Broker's update phase, including network communication and waiting on Agents. Can be slow." OFF)

#Option: interactive mode flag (for the GUI)
option(SIMMOB_INTERACTIVE_MODE "Force Sim Mobility to synchronize interactively with the GUI or console." OFF)


##################################################
## CPack Configuration
##################################################

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sim Mobility simulation environment.")
SET(CPACK_PACKAGE_NAME "simmobility")
SET(CPACK_PACKAGE_VENDOR "Singapore-MIT Alliance for Research and Technology (SMART)")
SET(CPACK_GENERATOR "DEB")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")

#Regarding the package version, we use the format "X.Y.Z.B", where "X", "Y", and "Z" follow
# traditional library numbering policy and "B" is the "build version" (today's date).
#Note that "B" is omitted for "Release" builds, which has the nice effect that, most of the 
# time, you should never have to modify anything and your *deb installs will work just fine for
# developers and users alike. 
#Reference: http://programmers.stackexchange.com/a/25015
TODAY(CPACK_PACKAGE_VERSION_NIGHTLY) #This will automatically handle date numbering for you.
SET(CPACK_PACKAGE_VERSION_MAJOR "0") #Never increase this.
SET(CPACK_PACKAGE_VERSION_MINOR "0") #Never increase this.
SET(CPACK_PACKAGE_VERSION_PATCH "1") #Increase this only when you plan to upload the generated *.deb to our Apt repository.

#Base String:
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

#+Build
IF (NOT CMAKE_BUILD_TYPE MATCHES "Release") #Match the empty build type too.
  SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}.${CPACK_PACKAGE_VERSION_NIGHTLY}")
ENDIF()

#More stuff
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
#SET(CPACK_STRIP_FILES "bin/SimMobility_Short")  #Note: this will fail on Windows (don't use CPack on Windows!)
#SET(CPACK_SOURCE_STRIP_FILES "")                #Note: this will fail on Windows (don't use CPack on Windows!)
#SET(CPACK_PACKAGE_EXECUTABLES "SimMobility_Short" "SimMobility short-term")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")  #TODO: We should use "dpkg --print-architecture"
SET(CPACK_PACKAGE_CONTACT "Seth N. Hetu <seth@smart.mit.edu>")

#This is where it starts to get complicated
SET(COMMA ",")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS
           "libboost-all-dev (>= 1.40)"
  ${COMMA} "libsoci-core-gcc-dev (>= 3.0.0)"
  ${COMMA} "libsoci-postgresql-gcc (>= 3.0.0)"
  ${COMMA} "libxerces-c-dev (>= 3.1.0)"
  ${COMMA} "xsdcxx (>= 3.3.0)"
  ${COMMA} "libpq-dev (>= 8.4.17)"
  ${COMMA} "libjsoncpp-dev (>= 0.6.0~rc2)"
  ${COMMA} "cmake"  #We check the version earlier; leave this line last.
)

IF (${BUILD_SHORT} MATCHES "ON" OR ${BUILD_MEDIUM} MATCHES "ON")
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS
		"libglpk-dev (>= 4.43-1)"
	)
ENDIF (${BUILD_SHORT} MATCHES "ON" OR ${BUILD_MEDIUM} MATCHES "ON")

#Make sure to flatten the DEPENDS array before using it.
string(REPLACE ";" "" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})

#Now bring everything together and generate the packing information.
INCLUDE(CPack)

##################################################
## End CPack Configuration
##################################################



#Special case flag: required on OSX due to a (somewhat) glitchy libstdc++
#IF(USING_OSX)
#  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -D_GLIBCXX_FULLY_DYNAMIC_STRING=1")
#ENDIF(USING_OSX)

#TODO: We use fPIC for everything in Debug mode, to avoid building twice. 
#      This (may) make Sim Mobility slower in Debug mode. In release mode,
#      we build two versions of the code: non-PIC for the executable, and 
#      PIC for the library. Of course, this slows the build process down.
IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  IF (NOT CMAKE_BUILD_TYPE MATCHES "Release")   #Match "empty" build_type too
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  ENDIF()
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

#Force gcc to output single line errors. 
# This makes it easier for Eclipse to parse and understand each error.
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#Generate a header file to contain configuration settings. This is useful
#  for, e.g., cmake variable which can then be set via the gui. 
configure_file (
  "${PROJECT_SOURCE_DIR}/shared/GenConfig.h.in"
  "${PROJECT_SOURCE_DIR}/shared/GenConfig.h"
)

#Add all config files in "shared/conf/settings"
FILE(GLOB SettingsFiles "${PROJECT_SOURCE_DIR}/shared/conf/settings/*.h.in")
FOREACH(InFile ${SettingsFiles})
  STRING(REGEX REPLACE "(.*[.]h)[.]in" "\\1" OutFile "${InFile}")
  configure_file ("${InFile}" "${OutFile}")
ENDFOREACH()

IF(${BUILD_STATIC} MATCHES "ON")
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
ENDIF(${BUILD_STATIC} MATCHES "ON")

#Cflags for C++0x support via gcc
IF (SIMMOB_LATEST_STANDARD)
  IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -static-libstdc++ -static-libgcc")
  ELSE() 
    #Just hope for the best
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libstdc++ -static-libgcc")
  ENDIF()
ELSE (SIMMOB_LATEST_STANDARD)
  #Avoid "C++0x compatibility" warnings.
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-compat -static-libstdc++ -static-libgcc")

  #Needed for profiling Agent updates (timing)
  #IF (${SIMMOB_AGENT_UPDATE_PROFILE} MATCHES "ON")
  #  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_XOPEN_SOURCE=700")
  #ENDIF (${SIMMOB_AGENT_UPDATE_PROFILE} MATCHES "ON")
ENDIF(SIMMOB_LATEST_STANDARD)


#Threading fix on GCC-4.7
IF(CMAKE_COMPILER_IS_GNUCXX)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  IF (GCC_VERSION VERSION_EQUAL 4.7)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_HAVE_GTHR_DEFAULT")
  ENDIF (GCC_VERSION VERSION_EQUAL 4.7)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)


#Enable threading on Clang
#NOTE: This is only a problem if GCC 4.7 AND clang are installed. 
#      If you get errors about "DISABLE_THREADS" being set for boost, you can either enable the next
#      few lines, or compile against gcc-4.6's standard library. 
#IF(CMAKE_C_COMPILER MATCHES "/clang$" AND CMAKE_CXX_COMPILER MATCHES "/clang[+][+]$")
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_GLIBCXX_HAVE_GTHR_DEFAULT")
#ENDIF()


#Testing PGO: Have to pass these flags in from the command line.
#NOTE: PGO currently doesn't work right.
#SET(TestPgoDirectory "${CMAKE_CURRENT_SOURCE_DIR}/pgo")
#IF (SIMMOBTEST_GENERATE_PROFILE)
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-generate=${TestPgoDirectory}")
#  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-generate=${TestPgoDirectory}")
#  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-generate=${TestPgoDirectory}")
#ENDIF(SIMMOBTEST_GENERATE_PROFILE)
#IF (SIMMOBTEST_USE_PROFILE)
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use=${TestPgoDirectory}")
#  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-use=${TestPgoDirectory}")
#  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-use=${TestPgoDirectory}")
#ENDIF (SIMMOBTEST_USE_PROFILE)


#Avoid deprecation warnings in boost by declaring that this platform has no hash_set
# implementation (note: doesn't affect unordered_map; fixes a warning with gcc-4.5 and boost 1.42)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_NO_HASH")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#Additional module find scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

#MinGW helpers
include_directories("/mingw/include")

#List of libraries to link
SET(LibraryList "")



#Find boost
set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48" "1.48.0" "1.49" "1.49.0" "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0" "1.53" "1.53.0" "1.54" "1.54.0" "1.55.0" "1.56.0" "1.57.0" "1.58.0" "1.59.0")
find_package(Boost COMPONENTS system thread filesystem serialization date_time chrono random regex REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
IF (${BUILD_STATIC} MATCHES "ON")
   LIST(APPEND LibraryList -Wl,-Bstatic)
ENDIF(${BUILD_STATIC} MATCHES "ON")

LIST(APPEND LibraryList -lboost_system -lboost_thread -lboost_filesystem -lboost_serialization -lboost_date_time -lboost_chrono -lboost_random -lboost_regex  -lsoci_core -lsoci_postgresql -lm -lc -lgcc -lc )


#Find TinyXML (optional; not used any more)
find_package(TinyXML)
if (TINYXML_FOUND)
  include_directories(${TINYXML_INCLUDE_DIR})
  LIST(APPEND LibraryList ${TINYXML_LIBRARIES})
endif()

#Find Xsd (headers only)
find_package(Xsd REQUIRED)
include_directories(${XSD_INCLUDE_DIR})


#Find JsonCpp
find_package(JsonCpp REQUIRED)
include_directories(${JSONCPP_INCLUDE_DIR})
LIST(APPEND LibraryList ${JSONCPP_LIBRARY})

#Find PROJ4
find_package(PROJ4 REQUIRED)
include_directories(${PROJ4_INCLUDE_DIR})
LIST(APPEND LibraryList ${PROJ4_LIBRARY})

#Find PQ
find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})
LIST(APPEND LibraryList ${PostgreSQL_LIBRARIES})

#Find SOCI
find_package(SOCI REQUIRED)
include_directories(${SOCI_INCLUDE_DIRS})
LIST(APPEND LibraryList ${SOCI_LIBRARIES})

#Find SOCI-PostgreSQL
find_package(SOCIPostgreSQL REQUIRED)
include_directories(${SOCIPOSTGRESQL_INCLUDE_DIRS})
LIST(APPEND LibraryList ${SOCIPOSTGRESQL_LIBRARIES})

#Find Lua
find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIRS})
LIST(APPEND LibraryList ${LUA_LIBRARIES})

#we can only link XercesC dynamically right now. Hence the line below. 
#I'm hoping we can do a full static link in the future. -chetan 16 May 2017
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so")
#Find XercesC
find_package(XercesC REQUIRED)
include_directories(${XERCESC_INCLUDE_DIR})
LIST(APPEND LibraryList ${XERCESC_LIBRARIES})

IF (${BUILD_STATIC} MATCHES "ON")
   SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
ENDIF(${BUILD_STATIC} MATCHES "ON")


LIST(APPEND LibraryList -lcrypto -lssl -lpq -ldl )

IF (${BUILD_SHORT} MATCHES "ON" OR ${BUILD_MEDIUM} MATCHES "ON")
    #Find GLPK
	FIND_LIBRARY( GLPK_LIB glpk PATHS /usr/lib $ENV{GLPK_DIR}/lib)
	SET(GLPK_LIBRARIES ${GLPK_LIB} )

	FIND_PATH(GLPK_INCLUDE_DIR
	  glpk.h
	  PATHS /usr/include/ /usr/include/glpk $ENV{GLPK_DIR}/include
	  DOC "Directory where GLPK header files are stored" )
	include_directories(${GLPK_INCLUDE_DIR})
	LIST(APPEND LibraryList -lglpk)
ENDIF (${BUILD_SHORT} MATCHES "ON" OR ${BUILD_MEDIUM} MATCHES "ON")

#Find CppUnit and QxCppUnit if we are building unit tests.
SET(UnitTestLibs "")
IF (${BUILD_TESTS} MATCHES "ON" OR ${BUILD_TESTS_LONG} MATCHES "ON")
  #Find CPP Unit
  find_package(CppUnit REQUIRED)
  include_directories(${CPPUNIT_INCLUDE_DIR})
  LIST(APPEND UnitTestLibs ${CPPUNIT_LIBRARY})

  #Find QxCppUnit and Qt, both of which are optional.
  find_package(QxCppUnit)
  FIND_PACKAGE(Qt4 4.0.0 COMPONENTS QtCore QtGui)

  #For our Windows/OSX users (where QxRunner is not-so-supported), allow the 
  #  tests to be build with pure CppUnit if either QxCppUnit or Qt4 are not found.
  IF (QXCPPUNIT_FOUND AND QT4_FOUND)
    SET(SIMMOB_USE_TEST_GUI ON)
  ELSE ()
    MESSAGE("Missing Qt4 or QxCppUnit: Unit Tests will be built in console mode.")
    SET(SIMMOB_USE_TEST_GUI OFF)
  ENDIF (QXCPPUNIT_FOUND AND QT4_FOUND)

  #Include/Library definitions for the GUI
  IF (SIMMOB_USE_TEST_GUI)
    #Include QxCppUnit
    include_directories(${QXCPPUNIT_INCLUDE_DIR})
    LIST(APPEND UnitTestLibs ${QXCPPUNIT_LIBRARIES})

    #Qt is picky about how it's #included
    INCLUDE(${QT_USE_FILE})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
    LIST(APPEND UnitTestLibs ${QT_LIBRARIES})
  ENDIF (SIMMOB_USE_TEST_GUI)
ENDIF ()

#Optionally find: MPI
IF (${SIMMOB_DISABLE_MPI} MATCHES "OFF")
  #Find boost-mpi
  find_package(Boost COMPONENTS mpi REQUIRED)
  LIST(APPEND LibraryList ${Boost_LIBRARIES})
  
  #Find any MPI implementation
  set(MPI_COMPILER ${MPI_ROOT}/bin/mpicxx)
  find_package(MPI REQUIRED)
  include_directories(${MPI_CXX_INCLUDE_PATH})
  LIST(APPEND LibraryList ${MPI_CXX_LIBRARIES})
  
  #More mpi-related flags
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}")
  
  #Use the mpi compiler.
  SET(CMAKE_C_COMPILER ${MPI_C_COMPILER})
  SET(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
  MESSAGE("INFO: Overriding c++ compiler: ${CMAKE_CXX_COMPILER}")
ENDIF(${SIMMOB_DISABLE_MPI} MATCHES "OFF")

#Real-time library needed for "clock_gettime"
IF (${SIMMOB_PROFILE_ON} MATCHES "ON")
  LIST(APPEND LibraryList "rt")
ENDIF (${SIMMOB_PROFILE_ON} MATCHES "ON")

#Find the Threading library (pthread)
find_package(Threads REQUIRED)
LIST(APPEND LibraryList ${CMAKE_THREAD_LIBS_INIT})

# Link to Splinter Library
#find_package(SPLINTER REQUIRED)
#include_directories(${SPLINTER_INCLUDE_DIR})
#LIST(APPEND LibraryList ${SPLINTER_LIBRARY})
##LIST(APPEND LibraryList /usr/local/lib/libsplinter-3-0.so)

# Link to TripEnergy Library
# TODO: Write Find module for optional TripEnergy library or configure path from XML (Jimi - Mar12)
#LIST(APPEND LibraryList ${CMAKE_SOURCE_DIR}/medium/models/tripEnergySO/libTripEnergySO.a)
#TODO: Put this in private/models and create find module accordingly
#find_package(TripEnergySO)
#if(TRIPENERGYSO_FOUND)
#  include_directories(${TRIPENERGYSO_INCLUDE_DIR})
#  LIST(APPEND LibraryList ${TRIPENERGYSO_LIBRARY})
#endif(TRIPENERGYSO_FOUND)
#LIST(APPEND LibraryList ${CMAKE_SOURCE_DIR}/medium/models/tripEnergySO/libTripEnergySO.a)

#Remove duplicates, just in case
#TODO: We should probably build up a list of IncludeDirs and LibraryDirs too (duplicates removed)
#      Not a real priority right now, but should allow better porting later on.
list(REMOVE_DUPLICATES LibraryList)

#Find all of our Shared source files
FILE(GLOB_RECURSE SharedCode_CPP shared/*.cpp)
FILE(GLOB_RECURSE SharedCode_HPP shared/*.hpp)


#Remove tests
FILE(GLOB_RECURSE SharedCode_TEST "shared/unit-tests/*.cpp" "shared/unit-tests/*.c")
LIST(REMOVE_ITEM SharedCode_CPP ${SharedCode_TEST})

#Remove geospatial/xmlreader
FILE(GLOB_RECURSE SharedCode_geo_xmlLoader "shared/geospatial/xmlLoader/*.cpp")
#LIST(REMOVE_ITEM SharedCode_CPP ${SharedCode_geo_xmlLoader})
#Remove shared/conf/xmlreader
FILE(GLOB_RECURSE SharedCode_conf_xmlLoader "shared/conf/xmlLoader/*.cpp")
#LIST(REMOVE_ITEM SharedCode_CPP ${SharedCode_conf_xmlLoader})

#All code has access to "shared/"
include_directories("${PROJECT_SOURCE_DIR}/shared")


#Manually managed exclude-list for Unity builds.
SET(UnityExclude "")
LIST(APPEND UnityExclude "shared/geospatial/aimsun/Loader.cpp")
LIST(APPEND UnityExclude "shared/geospatial/aimsun/CrossingLoader.cpp")
LIST(APPEND UnityExclude "shared/geospatial/aimsun/LaneLoader.cpp")
LIST(APPEND UnityExclude "shared/util/internal/xml_writer.cpp")
LIST(APPEND UnityExclude "shared/util/internal/namer.cpp")


#Determine if we are using Unity (full/fast) builds or not.
SET(UNITY_BUILD OFF)
IF (CMAKE_BUILD_TYPE MATCHES "Release" AND SIMMOB_FULL_FAST_BUILD_RELEASE) 
  SET(UNITY_BUILD ON)
ELSEIF(SIMMOB_FULL_FAST_BUILD_DEBUG)
  SET(UNITY_BUILD ON)
ENDIF()

#Mangle the include file list if we are using Full/Fast builds.
IF (UNITY_BUILD)
  enable_unity_build(SimMob_Shared SharedCode_CPP UnityExclude)
ENDIF()

#Build a shared object file for short-term, mid-term, long-term, and tests to use
add_library(SimMob_Shared OBJECT ${SharedCode_CPP})

#Build an *actual* shared object for the geospatial code & XML loader.
#TODO: For now we just include everything in the shared lib. Later we can slim it down.
IF (CMAKE_BUILD_TYPE MATCHES "Release") 
  #We need to re-build Release mode, since PIC is (likely) off.
  add_library(simmob_geoxml SHARED  ${SharedCode_CPP})
ELSE()
  #The debug (or other) code already uses PIC
  add_library(simmob_geoxml SHARED $<TARGET_OBJECTS:SimMob_Shared>)
ENDIF()

#Install targets for our shared library (header files)
#TODO: We currently install *all* header files in Shared.
#FILE(GLOB_RECURSE GeoHeaderFiles "${CMAKE_CURRENT_SOURCE_DIR}/shared/*.hpp")
#INSTALL(FILES ${GeoHeaderFiles} DESTINATION include/sim_mob)
install(DIRECTORY shared/ DESTINATION include/sim_mob FILES_MATCHING PATTERN "*.hpp" PATTERN "conf/settings/*.h")

#Install the library too.
INSTALL(TARGETS simmob_geoxml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

#Build the Short term?
IF (${BUILD_SHORT} MATCHES "ON")
	add_subdirectory(short)
ENDIF (${BUILD_SHORT} MATCHES "ON")

#Build the Medium term?
IF (${BUILD_MEDIUM} MATCHES "ON")
	add_subdirectory(medium)
ENDIF (${BUILD_MEDIUM} MATCHES "ON")

#Build the Long term?
IF (${BUILD_LONG} MATCHES "ON" OR ${BUILD_TESTS_LONG} MATCHES "ON")
	add_subdirectory(long)
ENDIF ()

#Build tests? 
IF (${BUILD_TESTS} MATCHES "ON")
	add_subdirectory(shared/unit-tests)
ENDIF (${BUILD_TESTS} MATCHES "ON")


# Based on http://majewsky.wordpress.com/2010/08/14/tip-of-the-day-cmake-and-doxygen/
# Add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    #configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doxy/SimMobility.doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

