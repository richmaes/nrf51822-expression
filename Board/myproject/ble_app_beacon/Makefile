TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

C_SOURCE_FILES += main.c
C_SOURCE_FILES += softdevice_handler.c
C_SOURCE_FILES += ble_advdata.c
C_SOURCE_FILES += nrf_delay.c

OUTPUT_FILENAME := ble_app_beacon_gcc
SDK_PATH = ../../../


#Uncomment correct line if you have s110 programmed on the chip.
DEVICE_VARIANT := xxaa
#DEVICE_VARIANT := xxab

USE_SOFTDEVICE := S110
#USE_SOFTDEVICE := s210

include $(SDK_PATH)Source/templates/gcc/Makefile.common
