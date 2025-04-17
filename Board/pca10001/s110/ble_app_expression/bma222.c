#include "bma222.h"
#include "twi_master.h"
#include "nrf_delay.h"


/*lint ++flb "Enter library region" */


static uint8_t m_device_address; //!< Device address in bits [7:1]
static int8_t m_accx;    //!<Accleration in X Axis is signed
static int8_t m_accy;    //!<Accleration in Y Axis is signed
static int8_t m_accz;    //!<Accleration in Z Axis is signed

// const uint8_t command_access_memory = 0x17; //!< Reads or writes to 256-byte EEPROM memory
// const uint8_t command_access_config = 0xAC; //!< Reads or writes configuration data to configuration register 
// const uint8_t command_read_temp = 0xAA; //!< Reads last converted temperature value from temperature register
// const uint8_t command_start_convert_temp = 0xEE; //!< Initiates temperature conversion.
// const uint8_t command_stop_convert_temp = 0x22; //!< Halts temperature conversion.

const uint8_t  ACCEL_BMA222_ACC_X_NEW    = 0x02;  //!< 0x02
const uint8_t  ACCEL_BMA222_ACC_X        = 0x03;  //!< 0x03
const uint8_t  ACCEL_BMA222_ACC_Y_NEW    = 0x04;  //!< 0x04
const uint8_t  ACCEL_BMA222_ACC_Y        = 0x05;  //!< 0x05
const uint8_t  ACCEL_BMA222_ACC_Z_NEW    = 0x06;  //!< 0x06
const uint8_t  ACCEL_BMA222_ACC_Z        = 0x07;  //!< 0x07
                                                  
const uint8_t  ACCEL_BMA222_INT_STAT     = 0x09;  //!< 0x09
const uint8_t  ACCEL_BMA222_INT_STAT_NEW = 0x0A;  //!< 0x0A
                                                  
const uint8_t  ACCEL_BMA222_INT_TAP      = 0x0B;  //!< 0x0B
const uint8_t  ACCEL_BMA222_INT_FLAT     = 0x0C;  //!< 0x0C
                                                  
const uint8_t  ACCEL_BMA222_RANGE_REG    = 0x0F;  //!< 0x0F
const uint8_t  ACCEL_BMA222_BW_REG       = 0x10;  //!< 0x10
const uint8_t  ACCEL_BMA222_SUSPEND_REG  = 0x11;  //!< 0x11
const uint8_t  ACCEL_BMA222_SFT_RST_REG  = 0x14;  //!< 0x14
                                                  
const uint8_t  ACCEL_BMA222_INT_SETTING1 = 0x16;  //!< 0x16
const uint8_t  ACCEL_BMA222_INT_SETTING2 = 0x17;  //!< 0x17
const uint8_t  ACCEL_BMA222_INT_MAPPING1 = 0x19;  //!< 0x19
const uint8_t  ACCEL_BMA222_INT_MAPPING2 = 0x1A;  //!< 0x1A
const uint8_t  ACCEL_BMA222_INT_MAPPING3 = 0x1B;  //!< 0x1B
                                                     
const uint8_t  ACCEL_BMA222_INT_DATA_SRC = 0x1E;  //!< 0x1E
const uint8_t  ACCEL_BMA222_INT_ELEC     = 0x20;  //!< 0x20

const uint8_t  ACCEL_BMA222_INT_RESET    = 0x21;  //!< 0x21

const uint8_t  ACCEL_BMA222_INT_LOWG_DURATION  = 0x22; //!< 0x22
const uint8_t  ACCEL_BMA222_INT_LOWG_THRESHOLD = 0x23; //!< 0x23

const uint8_t  ACCEL_BMA222_INT_HIG_DURATION   = 0x25; //!< 0x25
const uint8_t  ACCEL_BMA222_INT_HIG_THRESHOLD = 0x26; //!< 0x26


uint8_t read_buffer[64];

/**
 * Reads current range of the sensor.
 *
 * @return uint8_t Zero if communication with the sensor failed. 
 *         Proper settings for range are
 *         - 0x03 = +/- 2g
 *         - 0x05 = +/- 4g
 *         - 0x08 = +/- 8g
 *         - 0x0C = +/- 16g
 */
static uint8_t bma222_range_read(void)
{
  uint8_t config = 0;
    
  // Write: command protocol
  if (twi_master_transfer(m_device_address, (uint8_t*)&ACCEL_BMA222_RANGE_REG, 1, TWI_DONT_ISSUE_STOP))
  {
    if (twi_master_transfer(m_device_address | TWI_READ_BIT, &config, 1, TWI_ISSUE_STOP)) // Read: current configuration
    {
      // Read succeeded, configuration stored to variable "config"
    }
    else
    {
      // Read failed
      config = 0;
    }
  } 

  return config;
}

static bool bma222_write(uint8_t myRegister, uint8_t myData)
{
    uint8_t data_buffer[2];
	  bool writeresult = false;
  
    data_buffer[0] = myRegister;
    data_buffer[1] = myData;
  
    writeresult = twi_master_transfer(m_device_address, data_buffer, 2, TWI_ISSUE_STOP);
    
	  return writeresult;
}


bool bma222_init(uint8_t device_address)
{
  bool transfer_succeeded = true;
    uint8_t range;
	uint8_t result = 0;
  
  m_device_address = (uint8_t)(device_address << 1); //BMA222_BASE_ADDRESS + (uint8_t)(device_address << 1);
  range = bma222_range_read();  
  // range = 1;

  if (range != 0)
  {
    // Configure BMA for 2g mode if not done so already.
    // if (!(range == BMA222_RANGE_REG_2G_MODE))
    // {
   //  uint8_t data_buffer[3];
   // 
   //  data_buffer[0] = BMA222_BASE_ADDRESS;
   //  data_buffer[1] = ACCEL_BMA222_RANGE_REG;
   //  data_buffer[2] = BMA222_RANGE_REG_2G_MODE;
   // 
   //  transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);

    bma222_write(ACCEL_BMA222_RANGE_REG,BMA222_RANGE_REG_4G_MODE);

    // Populate interrupt with push pull and acitve high interrupts by default
    // data_buffer[1] = ACCEL_BMA222_INT_ELEC;
    // data_buffer[2] = 0x00;
    // 
    // transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
    bma222_write(ACCEL_BMA222_INT_ELEC,0x05);


    // Enable High G and Low G interrupts to wake device
    //data_buffer[1] = ACCEL_BMA222_INT_SETTING2;
    //data_buffer[2] = BMA222_INT_SETTING2_HIGHG_Z_EN | BMA222_INT_SETTING2_HIGHG_Y_EN | BMA222_INT_SETTING2_HIGHG_X_EN | BMA222_INT_SETTING2_LOW_EN;
	//
    //transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
	bma222_write(ACCEL_BMA222_INT_SETTING1,(BMA222_INT_SETTING1_S_TAP_EN ));
	bma222_write(ACCEL_BMA222_INT_SETTING2,BMA222_INT_SETTING2_HIGHG_Z_EN | BMA222_INT_SETTING2_HIGHG_Y_EN | BMA222_INT_SETTING2_HIGHG_X_EN | BMA222_INT_SETTING2_LOW_EN);

    // Assign HIGH G interrupts to interrupt 1
    // data_buffer[1] = ACCEL_BMA222_INT_MAPPING1;
    // data_buffer[2] = BMA222_INT_MAPPING_INT_HIGH | BMA222_INT_MAPPING_INT_D_TAP | BMA222_INT_MAPPING_INT_S_TAP;
	// 
	// 
	bma222_read_reg(ACCEL_BMA222_INT_MAPPING1, &result);

    bma222_write(ACCEL_BMA222_INT_MAPPING1,BMA222_INT_MAPPING_INT_HIGH  | BMA222_INT_MAPPING_INT_S_TAP);
   //transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
		
	bma222_read_reg(ACCEL_BMA222_INT_MAPPING1, &result);

    bma222_read_all_reg();

    // Assign LOW G interrupt to interrupt 2
    //data_buffer[1] = ACCEL_BMA222_INT_MAPPING3;
    //data_buffer[2] = BMA222_INT_MAPPING_INT_LOW;
	//
    //transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
    bma222_write(ACCEL_BMA222_INT_MAPPING3,BMA222_INT_MAPPING_INT_LOW);

    // Configure interrupts for temporary 8 seconds 
    //data_buffer[1] = ACCEL_BMA222_INT_RESET;
    //data_buffer[2] = BMA222_INT_RESET_250MS;
	//
    //transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
    bma222_write(ACCEL_BMA222_INT_RESET,0x80|BMA222_INT_RESET_250MS);

    
    // Configure low G duration
    #define LOW_G_INT_DURATION_MS 450 //!< 450MS duration 
    //data_buffer[1] = ACCEL_BMA222_INT_LOWG_DURATION;
    //data_buffer[2] = (LOW_G_INT_DURATION_MS/2)-1; //!< Convert to a delay value 
	//
    //transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
	bma222_write(ACCEL_BMA222_INT_LOWG_DURATION,(LOW_G_INT_DURATION_MS/2)-1);

    // Configure low G threshold
    #define LOW_G_INT_THRESHOLD_MG 375
    #define LOW_G_INT_THRESHOLD_CONVERSION (LOW_G_INT_THRESHOLD_MG/7.81)
    // data_buffer[1] = ACCEL_BMA222_INT_LOWG_THRESHOLD;
    // data_buffer[2] = LOW_G_INT_THRESHOLD_CONVERSION; //!< Calculated LOW G Threshold 
	// 
    // transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
    bma222_write(ACCEL_BMA222_INT_LOWG_THRESHOLD,LOW_G_INT_THRESHOLD_CONVERSION);


    // Configure high G duration
    #define HI_G_INT_DURATION_MS 32 //!< 32MS duration 
    // data_buffer[1] = ACCEL_BMA222_INT_HIG_DURATION;
    // data_buffer[2] = (HI_G_INT_DURATION_MS/2)-1; //!< Convert to a delay value 
	// 
    // transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
    bma222_write(ACCEL_BMA222_INT_HIG_DURATION,(HI_G_INT_DURATION_MS/2)-1);

    // Configure low G threshold
    #define HI_G_INT_THRESHOLD_MG 1300 //!< 1900 mG threshold
    // #define HI_G_INT_THRESHOLD_MG_CONVERSION (HI_G_INT_THRESHOLD_MG/7.81)   // In 2G Mode
	#define HI_G_INT_THRESHOLD_MG_CONVERSION (HI_G_INT_THRESHOLD_MG/15.63)  // In 4G Mode
    //data_buffer[1] = ACCEL_BMA222_INT_HIG_THRESHOLD;
    //data_buffer[2] = HI_G_INT_THRESHOLD_MG_CONVERSION; //!< Calculated LOW G Threshold 
	//
    //transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);
	bma222_write(ACCEL_BMA222_INT_HIG_THRESHOLD,HI_G_INT_THRESHOLD_MG_CONVERSION);
    
	// Place device into low power mode with 50ms sleep duration.
	bma222_write(ACCEL_BMA222_SUSPEND_REG,BMA222_SUSPEND_SLEEP_100MS | BMA222_SUSPEND_LOWPOWER_ON);
	
  
    // Select low bandwidth to conserve power
	bma222_write(ACCEL_BMA222_BW_REG,BMA222_7_81HZ);
	

  }

  else
  {
    transfer_succeeded = false;
  }


  return transfer_succeeded;
}

void bma222_setDeviceAddress(uint8_t device_address)
{
   m_device_address = (uint8_t)device_address << 1 ;// BMA222_BASE_ADDRESS + (uint8_t)(device_address << 1);
}

bool bma222_reset(void)
{ 
   bool result = false;
   result = bma222_write(ACCEL_BMA222_SFT_RST_REG, 0xB6);
	 nrf_delay_us(5000);
   return result;
}

//bool bma222_write(uint8_t device_address, uint8_t register_address, uint8_t register_data)
//{
//   bool transfer_succeeded = true;
//   //uint8_t range;
//   uint8_t data_buffer[3];
//  
//   m_device_address = BMA222_BASE_ADDRESS + (uint8_t)(device_address << 1);
//  
//   // Configure BMA for 2g mode if not done so already.
//     
//   data_buffer[0] = BMA222_BASE_ADDRESS;
//   data_buffer[1] = register_address;
//   data_buffer[2] = register_data;
//  
//   transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 3, TWI_ISSUE_START, TWI_ISSUE_STOP);  
//
//   return transfer_succeeded;
//}

bool bma222_read_reg(uint8_t reg_address, uint8_t *dataout)
{
    bool transfer_succeeded = false;

    // Write: Begin read acceleration command
    if (twi_master_transfer(m_device_address, (uint8_t*)&reg_address, 1,  TWI_DONT_ISSUE_STOP))
    {
      uint8_t data_buffer[1];
   
      // Read: 1 byte to data_buffer.  Only some are valid accelerations
      if (twi_master_transfer(m_device_address | TWI_READ_BIT, data_buffer, 1, TWI_ISSUE_STOP)) 
      {
        *dataout = (uint8_t)data_buffer[0];
                transfer_succeeded = true;
      }
    }

  return transfer_succeeded;
}


bool bma222_read_all_reg()
{
	  bool transfer_succeeded = true;
	  uint8_t i = 0;
	  //m_device_address = (uint8_t)(device_address << 1);
    
    for (i=0;i < 64; i = i + 1) {
        if (!bma222_read_reg(i,&read_buffer[i]))
		    {
            transfer_succeeded = false;
			      //i = 66;
		    }
    }

	return transfer_succeeded;
}

bool bma222_acc_read(void)
{
  bool transfer_succeeded = false;

  // Write: Begin read acceleration command
  if (twi_master_transfer(m_device_address, (uint8_t*)&ACCEL_BMA222_ACC_X_NEW, 1, TWI_DONT_ISSUE_STOP))
  {
    uint8_t data_buffer[6];

    // Read: 6 acceleration bytes to data_buffer.  Only some are valid accelerations
    if (twi_master_transfer(m_device_address | TWI_READ_BIT, data_buffer, 6,  TWI_ISSUE_STOP)) 
    {
      m_accx = (int8_t)data_buffer[1];
      m_accy = (int8_t)data_buffer[3];
      m_accz = (int8_t)data_buffer[5];
      
      transfer_succeeded = true;
    }
  }

  return transfer_succeeded;
}

int8_t bma222_getX(void)
{
  return m_accx;
}

int8_t bma222_getY(void)
{
  return m_accy;
}

int8_t bma222_getZ(void)
{
  return m_accz;
}




/*lint --flb "Leave library region" */ 
