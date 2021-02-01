#include <string.h>
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define NRF_LOG_FLOAT_MARKER "%s%d.%02d"
#define SPI_INSTANCE  0   

                                            /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

//Self-defined Variables
static uint8_t ADXL_Init_Addr[3] = {0x2D,0x31,0x38};
static uint8_t ADXL_Init_Data[3] = {0x08,0x0B,0x80};
static uint8_t ADXL_Data_Addr[6] = {0x33,0x32,0x35,0x34,0x37,0x36};
static int8_t  ADXL_Data[7] = {0};

static float x_value = 0.0;
static float y_value = 0.0;
static float z_value = 0.0; 


//Self-defined Functions
static void ADXL_Init();
static void ADXL_Read(uint8_t addr);

static void SPI_read(uint8_t addr, uint8_t *data, uint8_t length)
{
    ret_code_t err_code = 1;
    uint8_t tx_data;
    uint8_t rx_data;
    tx_data = 0x80 | addr;
    NRF_LOG_INFO("tx_data = 0x%x",tx_data);

    err_code = nrf_drv_spi_transfer(&spi, &tx_data, 1, &rx_data, 2);
    APP_ERROR_CHECK(err_code);
    
    *data = rx_data;
    NRF_LOG_INFO("rx_data = 0x%x   %d",*data,*data);

}

static void SPI_read_ADXL(uint8_t addr,uint8_t* rx_data,uint8_t bytes)
{
    ret_code_t err_code = 1;  
    uint8_t tx_data;

    if(bytes > 1)
        tx_data = 0x80|addr|0x40;
    else
        tx_data = 0x80|addr;

      err_code = nrf_drv_spi_transfer(&spi,&tx_data,7,rx_data,7);
      APP_ERROR_CHECK(err_code);
      NRF_LOG_INFO("\n");

    for(int i = 1 ;i < 7 ; i++)
      NRF_LOG_INFO("Data[%d] %d",i,rx_data[i]);
   
    

}
    
    

void SPI_write(uint8_t addr, uint8_t *data, uint8_t length)
{
    ret_code_t err_code = 1;
    uint8_t tx_data[length+1];

    tx_data[0] = addr;
    memcpy(tx_data+1,data,length);

    err_code = nrf_drv_spi_transfer(&spi ,tx_data, sizeof(tx_data), NULL, 0);
    NRF_LOG_INFO("err1 = %d",err_code);
    APP_ERROR_CHECK(err_code);

}

static void ADXL_Init(){
    
    NRF_LOG_INFO("ADXL_Init");
    for (uint8_t i = 0; i < 3; i++)
      SPI_write(ADXL_Init_Addr[i], &ADXL_Init_Data[i], 1);
    NRF_LOG_INFO("\n");
}



static void ADXL_Read_For_Test(uint8_t addr){
    uint8_t addr_for_write = addr;
    uint8_t addr_for_read  = addr|0x80;
    uint8_t data;
  
    SPI_read(addr, &data , 1);
    NRF_LOG_INFO("rx = %x",data);

}

//@brief SPI user event handler.
//@param event£±

int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin     = SPIM0_SS_PIN;
    spi_config.miso_pin   = SPIM0_MISO_PIN;
    spi_config.mosi_pin   = SPIM0_MOSI_PIN;
    spi_config.sck_pin    = SPIM0_SCK_PIN;
    spi_config.frequency  = NRF_SPI_FREQ_2M;
    spi_config.mode       = NRF_SPI_MODE_3;
    
    nrf_gpio_cfg_output(SPIM0_SS_PIN);
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));

    NRF_LOG_INFO("SPI example started.");
    ADXL_Init();
 
    while(1){
      //for test
      //ADXL_Read_For_Test(0x00);
      uint8_t rx_buffer[7] ;
      SPI_read_ADXL(0x32,rx_buffer,7); 

      x_value = (float)((int16_t)(rx_buffer[2]<<8)|rx_buffer[1])/256;
      y_value = (float)((int16_t)(rx_buffer[4]<<8)|rx_buffer[3])/256;
      z_value = (float)((int16_t)(rx_buffer[6]<<8)|rx_buffer[5])/256;


      NRF_LOG_INFO("x_value = " NRF_LOG_FLOAT_MARKER "\r", NRF_LOG_FLOAT(x_value));
      NRF_LOG_INFO("y_value = " NRF_LOG_FLOAT_MARKER "\r", NRF_LOG_FLOAT(y_value));
      NRF_LOG_INFO("z_value = " NRF_LOG_FLOAT_MARKER "\r", NRF_LOG_FLOAT(z_value));

      NRF_LOG_FLUSH();   
      nrf_delay_ms(1000);

    }

   
}
