// uicr.c
// Idea Rockets Consulting,  Copyright 2013
// nrf51822 user data driver

void uicr_rdstr(unsigned char *dest,unsigned int eep_adr,unsigned char lofdstr);
uint32_t nrf_nvmc_read_longword(uint32_t address);
uint8_t nrf_nvmc_read_byte(uint32_t address);
