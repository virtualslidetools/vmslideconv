#---------------------------------------------------------------------------
# What is this?
#---------------------------------------------------------------------------
# A Virtual Microscopy slide convertor that can convert to and from the
# the following formats:
#   a. TIFF/SVS file format to to Google Maps jpeg dataset format
#   b. Olympus/Bacus ini jpeg dataset convertor to Google Maps jpeg dataset
#   c. Olympus/Bacus ini jpeg dataset to TIFF/SVS file format

# The Olympus/Bacus ini jpeg dataset convertor is used for converting a 
# directory of .jpg files with two or more of their accompanying
# "FinalScan.ini", "FinalCond.ini", "SlideScan.ini", "SlideCond.ini" files 

# The Google Maps jpeg dataset can be used by OpenLayers, Leaflet, 
# or the Google Maps javascript API.

# The TIFF/SVS files are readable by Aperio ImageScope and
# the open source library libopenslide.

#---------------------------------------------------------------------------
# Instructions for compiling vmslideconv:
#---------------------------------------------------------------------------
# Skip to method 1 if you are using Visual Studio 2019 on Windows 10.
# Skip to method 2 if you are using Msys2 on Windows 10.
# Skip to method 3 if you are using RedHat/CentOS-Stream 8 or 9.
# Skip to method 4 if you are using Ubuntu 20.04.
# Skip to usage notes at the bottom of this document if you are interested
# in how to use the command line program.

# Every line in this file that starts with a # (pound symbol) is not meant
# to be typed into a shell prompt (unix or msys2) or command prompt 
# (windows/visual studio)

#-------------------------------------------------------------------------
# METHOD 1: Compiling vmslideconv with Visual Studio 2019 on Windows 10 
#-------------------------------------------------------------------------
# First install Visual Studio 2019 or later with git and cmake
# Then install vcpkg if you have not already

# Replace directory 'c:\src' in all the following lines with whatever 
# directory you use for your root source directory on Windows
mkdir c:\src
cd c:\src
c:
git clone https://github.com/Microsoft/vcpkg.git
set VCPKG_DEFAULT_TRIPLET=x64-windows
.\vcpkg\bootstrap-vcpkg.bat

# Get the vmslideconv source if you have not already
git clone https://github.com/virtualslidetools/vmslideconv

# Run cmake with the path to your vcpkg toolchain file
# You will need to change that path if you installed vcpkg 
# somewhere else
cmake -B vmslideconv-build -S vmslideconv -DCMAKE_TOOLCHAIN_FILE=c:\src\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
msbuild c:\src\vmslideconv-build\vmslideconv.sln

# --------------------------------------------------------------------------
# METHOD 2 - Compiling vmslideconv on Windows with msys2
# --------------------------------------------------------------------------
# Install MSYS2. Download the package from here: 
# http://www.msys2.org/

# 'pacman' is the command line package manager for MSYS2. 
# Update the MSYS2 system, then close your MSYS2 window:
pacman -Syu

# Reopen the MSYS2 window, and update all the other packages:
pacman -Syu

# Install gcc/g++ compilers:
pacman -S mingw64/mingw-w64-x86_64-gcc

# Install 'make' command:
pacman -S mingw64/mingw-w64-x86_64-make

# and:
pacman -S make

# Install pkg-config:
pacman -S msys/pkg-config

# Install libtiff:
pacman -S mingw64/mingw-w64-x86_64-libtiff

# Install opencv (this will also grab the libtiff and libjpeg-turbo 
dependacies):
pacman -S mingw64/mingw-w64-x86_64-opencv

# Install git if you plan on getting the code through git:
pacman -S git

# If you haven't already downloaded the source code for vmslideconv
# grab it here:
git clone https://github.com/virtualslidetools/vmslideconv.git

# Compile it by typing:
mkdir build
cmake ../vmslideconv
make

# Install the binary
make install

# You should have a executable vmslideconv.exe. When moving the
# executable to another system or not executing in a MSYS2 or Mingw shell 
# you will need to also supply the MSYS2 dlls for opencv, libjpeg-turbo, 
# and libtiff, and some other generic C/C++ runtime MSYS2 dlls.

#--------------------------------------------------------------------------
# METHOD 3: Compiling vmslideconv on Redhat 8/9 or CentOS-Stream 8/9
#            or similiar
#--------------------------------------------------------------------------
# Install git and development tools if you haven't done so already
sudo yum install git
sudo yum groupinstall 'Development Tools'

# For CentOS-Stream 9 you will neeed to use the below two lines to enable
# epel repo for openjpeg2-devel
dnf config-manager --set-enabled crb
dnf install epel-release epel-next-release

# Install libjpeg, libtiff, openjpeg2, and ncurses by typing:
sudo yum install libjpeg-turbo-devel libtiff-devel ncurses-devel openjpeg2-devel

# Install minizip-devel
sudo yum install minizip-devel

# -- BUILD OPENCV OR DOWNLOAD DEVELOPMENT RPM FOR IT --
# You will need opencv >= 3. You can either find a place to grab or build it using these directions:
sudo yum install cmake pkgconfig
sudo yum install libpng-devel jasper-devel openexr-devel libwebp-devel
sudo yum install libv4l-devel gstreamer1-plugins-base-devel
sudo yum install tbb-devel eigen3-devel

# Now grab the opencv source:
git clone https://github.com/opencv/opencv.git
cd opencv

# Now specify a stable version of opencv. 4.5.5 works fine:
git checkout 4.5.5
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D OPENCV_GENERATE_PKGCONFIG=ON ..
make
sudo make install

# For Redhat/CentOS 8, when you build opencv manually, it places it's 
# library files in /usr/local/lib64, and you will need /usr/local/lib64
# in your LD Path if it isn't already there. You can add this by adding 
# a file in /etc/ld.so.conf called usrlocallib64.conf:
sudo sh -c 'echo "/usr/local/lib64" > /etc/ld.so.conf.d/usrlocallib64.conf'

# Redhat/CentOS 8 you will need to run ldconfig:
sudo ldconfig

# Now go back to your vmslideconv directory:
cd ..

# -- GRAB THE VMSLIDECONV IF YOU HAVEN'T ALREADY AND COMPILE --
git clone https://github.com/virtualslidetools/vmslideconv.git
cd vmslideconv 
mkdir build
cmake ..
make

# -- INSTALL THE PROGRAM --
sudo make install

#--------------------------------------------------------------------------
# METHOD 4: Compiling vmslideconv on Ubuntu 20.04
#--------------------------------------------------------------------------
# Install git and development tools if you haven't done so already
sudo apt update
sudo apt install build-essential

# Install cmake and pkgconfig
sudo apt install pkg-config cmake

# Install libjpeg, libtiff, libopencv by typing:
sudo apt install libjpeg-turbo8-dev libtiff-dev libopencv-dev libopenjp2-7-dev

# Install minizip and ncurses-dev
sudo apt install libminizip-dev libncurses-dev

# -- GRAB VMSLIDECONV IF YOU HAVEN'T ALREADY AND COMPILE --
git clone https://github.com/virtualslidetools/vmslideconv.git
cd vmslideconv
mkdir build
cd build
cmake ..
make

# -- INSTALL THE PROGRAM --
sudo make install
