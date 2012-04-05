#!/bin/sh

# http://www.codesourcery.com/sgpp/lite/arm/portal/release1802
TOOLCHAIN_DIR=/opt/arm-2011.03

# http://www.st.com/internet/com/SOFTWARE_RESOURCES/SW_COMPONENT/FIRMWARE/stm32f10x_stdperiph_lib.zip
STM32_LIB_DIR=/opt/STM32F10x_StdPeriph_Lib_V3.5.0

BUILD_DIR=/tmp/stm32

# prepare build dir
rm -fr $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 1. checkout stm32-cmake
git clone git://github.com/ObKo/stm32-cmake.git
cd stm32-cmake/cmsis

P=""
param_add() { 
    P="$P -D${1}=${2}" 
}

param_add CMAKE_TOOLCHAIN_FILE      ../gcc_stm32.cmake
param_add CMAKE_BUILD_TYPE          Release
param_add STM32_StdPeriphLib_DIR    $STM32_LIB_DIR
param_add TOOLCHAIN_PREFIX          $TOOLCHAIN_DIR
param_add CMAKE_INSTALL_PREFIX      $TOOLCHAIN_DIR/arm-none-eabi/

# 2. creates makefile and build
cmake $P \
    && make \
    && make install

# 3. copy cmake files
echo "copy FindCMSIS.cmake to cmake modules dir"
sudo cp ../cmake/Modules/FindCMSIS.cmake /usr/share/cmake-2.8/Modules    
sudo cp ../gcc_stm32.cmake $TOOLCHAIN_DIR

