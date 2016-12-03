TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

C_SOURCE_FILES += main.c
C_SOURCE_FILES += nrf_delay.c
C_SOURCE_FILES += retarget.c
C_SOURCE_FILES += simple_uart.c

OUTPUT_FILENAME := printf_gcc
SDK_PATH = ../../../

CFLAGS = -O0 -g
LDFLAGS = --specs=nano.specs -u _printf_float

#Uncomment correct line if you have s110 programmed on the chip.
DEVICE_VARIANT := xxaa
#DEVICE_VARIANT := xxab

#USE_SOFTDEVICE := s110
#USE_SOFTDEVICE := s210

include $(SDK_PATH)Source/templates/gcc/Makefile.common
