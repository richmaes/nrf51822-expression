// uicr.c
// Idea Rockets Consulting,  Copyright 2013
// nrf51822 user data driver

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "uicr.h"


void uicr_rdstr(unsigned char *dest,unsigned int eep_adr,unsigned char lofdstr)
{                               // reads a string from the spec addr   
	unsigned char k = 0 ;       // init char counter inside the string 
	while (k<lofdstr) {       // for (n-1) bytes  
	   dest[k] = nrf_nvmc_read_byte(eep_adr+k) ;      
	   k++   ;
	}
} 

uint32_t nrf_nvmc_read_longword(uint32_t address)
{
  uint32_t value;
  // Enable read.
  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
  while (NRF_NVMC->READY == NVMC_READY_READY_Busy){
  }

  value = *(uint32_t*)address;
  while (NRF_NVMC->READY == NVMC_READY_READY_Busy){
  }

  return value;
  
}

uint8_t nrf_nvmc_read_byte(uint32_t address)
{
   uint8_t value;
   uint32_t value_32;
   uint32_t jAddress;
   // Calculate long word justified address
   jAddress = address & 0xFFFFFFFC;

   // Enable read.

   // Enable read.
  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
  while (NRF_NVMC->READY == NVMC_READY_READY_Busy){
  }
  // retrieve 32 bit value with justfied address
  value_32 = *(uint32_t*)jAddress;

  // byte select and mask the chunk we want
  value = (value_32 >> ((address % 4) * 8)) & 0x000000FF;
  
  while (NRF_NVMC->READY == NVMC_READY_READY_Busy){
  }
  
  return value;
}
