cmake_minimum_required(VERSION 3.0)

project(vmslideconv LANGUAGES C CXX )

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if (CMAKE_TOOLCHAIN_FILE)
  include(${CMAKE_TOOLCHAIN_FILE})
endif()

include(CheckStructHasMember)

if ("\"${CMAKE_CXX_COMPILER_ID}\"" MATCHES "MSVC")
  find_package(ZLIB REQUIRED)
  set(ZLIB::ZLIB)
 # message("Zlib include directory=${ZLIB_INCLUDE_DIR}")
  find_package(minizip 1 REQUIRED)
  find_path(MINIZIP_INCLUDE_DIRS NAMES zip.h PATH_SUFFIXES minizip include)
  message("Minizip include directory=${MINIZIP_INCLUDE_DIRS}")
  set(MINIZIP_LIBRARIES minizip::minizip)
  set(CMAKE_REQUIRED_INCLUDES "${MINIZIP_INCLUDE_DIRS};${ZLIB_INCLUDE_DIR}")
else()
  find_package(ZLIB)
  find_package(PkgConfig)
endif()

if (NOT MINIZIP_INCLUDE_DIRS AND PKG_CONFIG_FOUND)
  pkg_check_modules(MINIZIP minizip)
  if (NOT MINIZIP_LIBRARIES)
    pkg_check_modules(MINIZIP minizip2)
    add_definitions(-DOLY_USE_MZ_VER2=1)
    set(OLY_USE_MZ_VER2 "1")
  else()
    message("Minizip version=${MINIZIP_VERSION}")
    if ("${MINIZIP_VERSION}" MATCHES "^1")
      message("Minizip version 1 detected.")
    elseif ("${MINIZIP_VERSION}" MATCHES "^2")
      add_definitions(-DOLY_USE_MZ_VER2=1)
      set(OLY_USE_MZ_VER2 "1")
    elseif ("${MINIZIP_VERSION}" MATCHES "^3")
      add_definitions(-DOLY_USE_MZ_VER2=1)
      set(OLY_USE_MZ_VER2 "1")
      # glitch in minizip 3.0.2, no include directory set
      set(MINIZIP_INCLUDE_DIRS "/usr/include/minizip")
    endif()
  endif()
endif()

if (NOT MINIZIP_LIBRARIES)
	message(FATAL_ERROR "Minzip library version 1,2 or 3 not found!")
endif()

if (NOT MINIZIP_INCLUDE_DIRS)
	message(FATAL_ERROR "Minzip include dir version 1,2 or 3 not found!")
endif()


if (OLY_USE_MZ_VER2)
  set(OLY_MINIZIP_FILE "mz_compat.h")
else()
  set(OLY_MINIZIP_FILE "zip.h")
endif()
message("Minizip main include file=${OLY_MINIZIP_FILE}")

if (NOT MSVC)
  set(CMAKE_REQUIRED_INCLUDES "${MINIZIP_INCLUDE_DIRS}" "${ZLIB_INCLUDE_DIR}")
endif()

CHECK_STRUCT_HAS_MEMBER("zip_fileinfo" "dosDate" "${OLY_MINIZIP_FILE}" OLY_HAVE_DOSDATE LANGUAGE CXX)
CHECK_STRUCT_HAS_MEMBER("zip_fileinfo" "mz_dos_date" "${OLY_MINIZIP_FILE}" OLY_HAVE_MZ_DOS_DATE LANGUAGE CXX)
CHECK_STRUCT_HAS_MEMBER("zip_fileinfo" "dos_date" "${OLY_MINIZIP_FILE}" OLY_HAVE_DOS_DATE LANGUAGE CXX)
if (OLY_HAVE_MZ_DOS_DATE)
  add_definitions(-DOLY_HAVE_MZ_DOS_DATE=1)
endif()
if (OLY_HAVE_DOSDATE)
  add_definitions(-DOLY_HAVE_DOSDATE=1)
endif()
if (OLY_HAVE_DOS_DATE)
  add_definitions(-DOLY_HAVE_DOS_DATE=1)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif()

option(USE_MAGICK "Use ImageMagick instead of OpenCV for processing." OFF)
option(USE_OPENCV "Use OpenCV for image processing instead of ImageMagick." ON)

find_package(JPEG)
find_package(TIFF)
find_package(OpenJPEG)
find_package(Threads)
if (USE_MAGICK)
	find_package(ImageMagick COMPONENTS MagickCore MagickWand)
	set (USE_OPENCV OFF)
else()
	set (USE_OPENCV ON)
endif()

if (USE_OPENCV)
	find_package(OpenCV)
endif()

if (JPEG_FOUND)
  set (LIBJPEG_LIBRARIES ${JPEG_LIBRARY})
  set (LIBJPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
elseif (PKG_CONFIG_FOUND)
  pkg_check_modules(LIBJPEG REQUIRED libjpeg)
else()
  message(FATAL_ERROR "Jpeg library not found!")
endif()

if (TIFF_FOUND)
  set (LIBTIFF_LIBRARIES ${TIFF_LIBRARY})
  set (LIBTIFF_INCLUDE_DIRS ${TIFF_INCLUDE_DIR})
elseif (PKG_CONFIG_FOUND)
  pkg_check_modules(LIBTIFF REQUIRED libtiff-4)
else()
  message(FATAL_ERROR "LibTiff not found!")
endif()

if (OPENJPEG_FOUND)
  set (LIBOPENJPEG_LIBRARIES ${OPENJPEG_LIBRARIES})
  set (LIBOPENJPEG_INCLUDE_DIRS ${OPENJPEG_INCLUDE_DIRS})
elseif (PKG_CONFIG_FOUND)
  pkg_check_modules(LIBOPENJPEG REQUIRED libopenjpeg)
else()
  message(FATAL_ERROR "OpenJPEG library not found!")
endif()

if (${USE_MAGICK} STREQUAL "OFF")
  add_definitions(-DUSE_OPENCV=1)
  if (OpenCV_FOUND)
 	  set (OPENCV_LIBRARIES ${OpenCV_LIBS})
    set (OPENCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})
  elseif (PKG_CONFIG_FOUND)
    pkg_check_modules(OPENCV REQUIRED opencv4)
  else()
    message(FATAL_ERROR "OpenCV Libraries not found!")
  endif()
else()
  add_definitions(-DUSE_MAGICK=1)
  add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=16)
  add_definitions(-DMAGICKCORE_HDRI_ENABLE=1)
  add_definitions(-DMAGICK_MAP_LIMIT="256MB")
  add_definitions(-DMAGICK_MEMORY_LIMIT="256MB")
  add_definitions(-DMAGICK_DISK_LIMIT="16GB")
  add_definitions(-DMAGICK_AREA_LIMIT="50MPP")
  if (ImageMagick_MagickWand_FOUND AND ImageMagick_MagickCore_FOUND)
    set (MAGICKWAND_LIBRARIES ${ImageMagick_MagickWand_LIBRARY})
    set (MAGICKWAND_INCLUDE_DIRS ${ImageMagick_MagickWand_INCLUDE_DIR})
    set (MAGICKCORE_LIBRARIES ${ImageMagick_MagickCore_LIBRARY})
    set (MAGICKCORE_INCLUDE_DIRS ${ImageMagick_MagickCore_INCLUDE_DIR})
  elseif (PKG_CONFIG_FOUND)
    pkg_check_modules(MAGICKWAND REQUIRED MagickWand)
    pkg_check_modules(MAGICKCORE REQUIRED MagickCore)
  else()
    message(FATAL_ERROR "ImageMagick Wand and Core Libraries not found!")
  endif()
endif()

message(STATUS "OpenCV Include directories:")
message(STATUS ${OPENCV_INCLUDE_DIRS})
message(STATUS ${OPENCV_INCLUDE_DIR})

set(ENABLE_EXPORTS, true)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED YES)

set( MAIN_SRC
  src/safebmp.cc
  src/safejpg.cc
  src/vmslideconv.cc
  src/jpgcachesupport.cc
  src/tilecachesupport.cc
  src/imagesupport.cc
  src/jpgsupport.cc
  src/tiffsupport.cc
  src/jp2kdecode.cc
  src/zipsupport.cc
  src/virtualslide.cc
  src/composite.cc
  src/composite-read.cc
  src/aperio.cc
  src/drawing.cc
  src/blend.cc
  src/ycbcr.cc
)

set( MAIN_HEADERS
  include/vmslideconv.h
  include/virtualslide.h
  include/composite.h
  include/aperio.h
  include/imagesupport.h
  include/jpgsupport.h
  include/jpgcachesupport.h
  include/tiffsupport.h
  include/tilecachesupport.h
  include/zipsupport.h
  include/blendbkgd.h
  include/safebmp.h
)

set( INFO_SRC
  src/imagesupport.cc
  src/tiffsupport.cc
  src/tilecachesupport.cc
  src/svsinfo.cc
  src/safebmp.cc
  src/jp2kdecode.cc
  src/ycbcr.cc 
)

set ( INFO_HEADERS
  include/tiffsupport.h
  include/imagesupport.h
  include/jpgsupport.h
  include/safebmp.h
)

if (WIN32 OR WIN64 OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list(APPEND MAIN_HEADERS include/console-mswin.h)
  list(APPEND MAIN_HEADERS include/getopt-mswin.h)
  list(APPEND MAIN_SRC src/console-mswin.cc)
  if (USE_MAGICK)
    list(APPEND MAIN_SRC src/initmagick.cc)
  endif()
else()
  list(APPEND MAIN_HEADERS include/console-unix.h)
  list(APPEND MAIN_SRC src/console-unix.cc)
endif()

include(CheckSymbolExists)
check_symbol_exists(strerror   string.h HAVE_STRERROR)
check_symbol_exists(strerror_r string.h HAVE_STRERROR_R)
check_symbol_exists(strerror_s string.h HAVE_STRERROR_S)

check_symbol_exists(localtime   time.h HAVE_LOCALTIME)
check_symbol_exists(localtime_r time.h HAVE_LOCALTIME_R)
check_symbol_exists(localtime_s time.h HAVE_LOCALTIME_S)

check_symbol_exists(strncpy_s string.h HAVE_STRNCPY_S)
check_symbol_exists(_putenv stdlib.h HAVE__PUTENV)

check_symbol_exists(fopen_s stdio.h HAVE_FOPEN_S)

if (HAVE_STRERROR)
  add_definitions(-DHAVE_STRERROR)
endif()

if (HAVE_STRERROR_R)
  message(STATUS "Performing ${CMAKE_PROJECT_NAME} Test HAVE_GLIBC_STRERROR_R")
  try_compile(HAVE_GLIBC_STRERROR_R
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/compilertests.cc
    OUTPUT_VARIABLE HAVE_GLIBC_STRERROR_R)

  message(STATUS "Performing ${CMAKE_PROJECT_NAME} Test HAVE_POSIX_STRERROR_R")
  try_compile(HAVE_POSIX_STRERROR_R
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/compilertests.cc
    OUTPUT_VARIABLE HAVE_POSIX_STRERROR_R)

  if (HAVE_GLIBC_STRERROR_R)
    add_definitions(-DHAVE_GLIBC_STRERROR_R)
  elseif (HAVE_POSIX_STRERROR_R)
    add_definitions(-DHAVE_POSIX_STRERROR_R)
  endif()
endif()

if (HAVE_STRERROR_S)
  add_definitions(-DHAVE_STRERROR_S)
endif()

if (HAVE_LOCALTIME)
  add_definitions(-DHAVE_LOCALTIME)
endif()

if (HAVE_LOCALTIME_R)
  add_definitions(-DHAVE_LOCALTIME_R)
endif()

if (HAVE_LOCALTIME_S)
  add_definitions(-DHAVE_LOCALTIME_S)
endif()

if (HAVE_STRNCPY_S)
  add_definitions(-DHAVE_STRNCPY_S)
endif()

if (HAVE__PUTENV)
  add_definitions(-DHAVE__PUTENV)
endif()

if (HAVE_FOPEN_S)
  add_definitions(-DHAVE_FOPEN_S)
endif()

include_directories(${MAGICKCORE_INCLUDE_DIRS} ${MAGICKWAND_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} ${LIBJPEG_INCLUDE_DIRS} ${LIBTIFF_INCLUDE_DIRS} ${LIBOPENJPEG_INCLUDE_DIRS} ${MINIZIP_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable( vmslideconv ${MAIN_SRC} ${MAIN_HEADERS} )
target_link_libraries( vmslideconv ${MAGICKWAND_LIBRARIES} ${MAGICKCORE_LIBRARIES} ${OPENCV_LIBRARIES} ${LIBTIFF_LIBRARIES} ${LIBJPEG_LIBRARIES} ${LIBOPENJPEG_LIBRARIES} ${MINIZIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

if (MSVC)
  target_compile_options(vmslideconv PRIVATE /W4 /WX)  
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -pg -O0 -Wall -Wextra -Wpedantic")
  set(CMAKE_CXX_FLAGS_RELEASE "-g -O3 -Wall -Wextra -Wpedantic")
endif()

add_executable( svsinfo ${INFO_SRC} )
target_link_libraries( svsinfo ${LIBTIFF_LIBRARIES} ${LIBOPENJPEG_LIBRARIES} ${OPENCV_LIBRARIES})

install(TARGETS vmslideconv DESTINATION bin)
install(TARGETS svsinfo DESTINATION bin)
