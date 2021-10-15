#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/gpio.h>

#define ADAPTER_NAME     "MyDevice_I2C_ADAPTER"


#define SCL_GPIO  PIN_SCL    //GPIO act as SCL
#define SDA_GPIO  PIN_SDA    //GPIO act as SDA

#define I2C_DELAY usleep_range(5, 10) 

/*
** Function to read the SCL GPIO
*/
static bool MyDevice_I2C_Read_SCL(void)
{
  gpio_direction_input(SCL_GPIO);
  return gpio_get_value(SCL_GPIO);
}

/*
** Function to read the SDA GPIO
*/
static bool MyDevice_I2C_Read_SDA(void)
{
  gpio_direction_input(SDA_GPIO);
  return gpio_get_value(SDA_GPIO);
}


//Function to clear the SCL GPIO

static void MyDevice_I2C_Clear_SCL(void)
{
  gpio_direction_output(SCL_GPIO, 0);
  gpio_set_value(SCL_GPIO, 0);
}

// Function to clear the SDA GPIO

static void MyDevice_I2C_Clear_SDA(void)
{
  gpio_direction_output(SDA_GPIO, 0);
  gpio_set_value(SDA_GPIO, 0);
}


//Function to set the SCL GPIO

static void MyDevice_I2C_Set_SCL(void)
{
  gpio_direction_output(SCL_GPIO, 1);
  gpio_set_value(SCL_GPIO, 1);
}


// Function to set the SDA GPIO

static void MyDevice_I2C_Set_SDA(void)
{
  gpio_direction_output(SDA_GPIO, 1);
  gpio_set_value(SDA_GPIO, 1);
}


// Function to Initialize the GPIOs
static int MyDevice_I2C_Init( void )
{
  int ret = 0;

  do      //Break if any error
  {
    //Checking the SCL GPIO is valid or not
    if(gpio_is_valid(SCL_GPIO) == false){
          pr_err("SCL GPIO %d is not valid\n", SCL_GPIO);
          ret = -1;
          break;
    }

    //Checking the SDA GPIO is valid or not
    if(gpio_is_valid(SDA_GPIO) == false){
          pr_err("SDA GPIO %d is not valid\n", SDA_GPIO);
          ret = -1;
          break;
    }
    
    //Requesting the SCL GPIO
    if(gpio_request(SCL_GPIO,"SCL_GPIO") < 0){
          pr_err("ERROR: SCL GPIO %d request\n", SCL_GPIO);
          ret = -1;
          break;
    }

    //Requesting the SDA GPIO
    if(gpio_request(SDA_GPIO,"SDA_GPIO") < 0){
          pr_err("ERROR: SDA GPIO %d request\n", SDA_GPIO);
          //free already requested SCL GPIO
          gpio_free(SCL_GPIO);
          ret = -1;
          break;
    }
    
    gpio_direction_output(SCL_GPIO, 1);
    gpio_direction_output(SDA_GPIO, 1);

  } while(false);

  return ret;  
}


// Function to Deinitialize the GPIOs

static void MyDevice_I2C_DeInit( void )
{
  //free both the GPIOs
  gpio_free(SCL_GPIO);
  gpio_free(SDA_GPIO);
}


// Function to send the START condition

static void MyDevice_I2C_Start( void )
{
  MyDevice_I2C_Set_SDA();
  MyDevice_I2C_Set_SCL();
  I2C_DELAY;
  MyDevice_I2C_Clear_SDA();
  I2C_DELAY;  
  MyDevice_I2C_Clear_SCL();
  I2C_DELAY;  
}


// Function to send the STOP condition

static void MyDevice_I2C_Stop( void )
{
  MyDevice_I2C_Clear_SDA();
  I2C_DELAY;
  MyDevice_I2C_Set_SCL();
  I2C_DELAY;
  MyDevice_I2C_Set_SDA();
  I2C_DELAY;
  MyDevice_I2C_Clear_SCL();
}


// Function to reads the SDA to get the status and Returns 0 for NACK, returns 1 for ACK

static int MyDevice_I2C_Read_NACK_ACK( void )
{
  int ret = 1;

  //reading ACK/NACK
  I2C_DELAY;
  MyDevice_I2C_Set_SCL();
  I2C_DELAY;
  if( MyDevice_I2C_Read_SDA() )      //check for ACK/NACK
  {
    ret = 0;
  }

  MyDevice_I2C_Clear_SCL();
  return ret;
}


// Function to send the 7-bit address to the slave and Returns 0 if success otherwise negative number

static int MyDevice_I2C_Send_Addr( u8 byte, bool is_read )
{
  int ret   = -1;
  u8  bit;
  u8  i     = 0;
  u8  size  = 7;

  //Writing 7bit slave address
  for(i = 0; i < size ; i++)
  {
    bit = ( ( byte >> ( size - ( i + 1 ) ) ) & 0x01 );  //store MSB value
    (bit) ? MyDevice_I2C_Set_SDA() : MyDevice_I2C_Clear_SDA();    //write MSB value     
    I2C_DELAY;
    MyDevice_I2C_Set_SCL();
    I2C_DELAY;
    MyDevice_I2C_Clear_SCL();
  }

  // Writing Read/Write bit (8th bit)
  (is_read) ? MyDevice_I2C_Set_SDA() : MyDevice_I2C_Clear_SDA();  //read = 1, write = 0
  I2C_DELAY;
  MyDevice_I2C_Set_SCL();
  I2C_DELAY;
  MyDevice_I2C_Clear_SCL();
  I2C_DELAY;

  if( MyDevice_I2C_Read_NACK_ACK() )
  {
    //got ACK
    ret = 0;
  }

  return ret;
}


// Function to send the one byte to the slave and Returns 0 if success otherwise negative number
static int MyDevice_I2C_Send_Byte( u8 byte )
{
  int ret   = -1;
  u8  bit;
  u8  i     = 0;
  u8  size  = 7;

  for(i = 0; i <= size ; i++)
  {
    bit = ( ( byte >> ( size - i ) ) & 0x01 );        //store MSB value
    (bit) ? MyDevice_I2C_Set_SDA() : MyDevice_I2C_Clear_SDA();  //write MSB value     
    I2C_DELAY;
    MyDevice_I2C_Set_SCL();
    I2C_DELAY;
    MyDevice_I2C_Clear_SCL();
  }
  
  if( MyDevice_I2C_Read_NACK_ACK() )
  {
    //got ACK
    ret = 0;
  }

  return ret;
}


//Function to read the one byte from the slave and Returns 0 if success otherwise negative number

static bool  MyDevice_I2C_Read_bit(  ){
  bool i;

  MyDevice_I2C_Read_SDA();
  I2C_DELAY();
  while (MyDevice_I2C_Read_SCL() == 0){
    I2C_SLEEP();
  }
  i= MyDevice_I2C_Read_SDA();
  I2C_DELAY();
  MyDevice_I2C_Clear_SCL();
  return i;
}
static int MyDevice_I2C_Read_Byte( u8 *byte )
{
  int ret = 0;
  
 unsigned bit;
    for (bit = 0; bit < 8; bit++)
    {
        *byte = (*byte << 1) | MyDevice_I2C_Read_bit();
    }
if (sizeof(*byte)!=sizeof(u8)){
  ret=-1;
}
  return ret;
}


//Function to send the number of bytes to the slave and returns 0 if success otherwise negative number

static int MyDevice_I2C_Send( u8 slave_addr, u8 *buf, u16 len )
{
  int ret = 0;
  u16 num = 0;

  do      //Break if any error
  {

    if( MyDevice_I2C_Send_Addr(slave_addr, false) < 0 )   // Send the slave address
    {
      pr_err("ERROR: MyDevice_I2C_Send_Byte - Slave Addr\n");
      ret = -1;
      break;
    }

    for( num = 0; num < len; num++)
    {
      if( MyDevice_I2C_Send_Byte(buf[num]) < 0 )   // Send the data bytes
      {
        pr_err("ERROR: MyDevice_I2C_Send_Byte - [Data = 0x%02x]\n", buf[num]);
        ret = -1;
        break;
      }
    }
  } while(false);

  return ret;  
}



/*------------ I2C related functions - Bit banging method - END ----------------*/


 //This function used to get the functionalities that are supported by this bus driver.
static u32 MyDevice_func(struct i2c_adapter *adapter)
{
  return (I2C_FUNC_I2C             |
          I2C_FUNC_SMBUS_QUICK     |
          I2C_FUNC_SMBUS_BYTE      |
          I2C_FUNC_SMBUS_BYTE_DATA |
          I2C_FUNC_SMBUS_WORD_DATA |
          I2C_FUNC_SMBUS_BLOCK_DATA);
}


//This function will be called whenever you call I2C read, wirte APIs like i2c_master_send(), i2c_master_recv() etc.

static s32 MyDevice_i2c_xfer( struct i2c_adapter *adap, struct i2c_msg *msgs,int num )
{
  int i;
  s32 ret = 0;
  do      //Break if any error
  {

    if( MyDevice_I2C_Init() < 0 )                  // Init the GPIOs
    {
      pr_err("ERROR: MyDevice_I2C_Init\n");
      break;
    }

    MyDevice_I2C_Start();                           // Send START conditon
    
    for(i = 0; i < num; i++)
    {
      //int j;
      struct i2c_msg *msg_temp = &msgs[i];

      if( MyDevice_I2C_Send(msg_temp->addr, msg_temp->buf, msg_temp->len) < 0 )
      {
        ret = 0;
        break;
      }
      ret++;

    }
  } while(false);

  MyDevice_I2C_Stop();                             // Send STOP condtion

  MyDevice_I2C_DeInit();                           // Deinit the GPIOs

  return ret;
}

/*
** I2C algorithm Structure
*/
static struct i2c_algorithm MyDevice_i2c_algorithm = {
  .smbus_xfer     = MyDevice_smbus_xfer,
  .master_xfer    = MyDevice_i2c_xfer,
  .functionality  = MyDevice_func,
};

/*
** I2C adapter Structure
*/
static struct i2c_adapter MyDevice_i2c_adapter = {
  .owner  = THIS_MODULE,
  .class  = I2C_CLASS_HWMON,//| I2C_CLASS_SPD,
  .algo   = &MyDevice_i2c_algorithm,
  .name   = ADAPTER_NAME,
  .nr     = 5,
};

/*
** Module Init function
*/
static int __init MyDevice_driver_init(void)
{
  int ret = -1;
  
  ret = i2c_add_numbered_adapter(&MyDevice_i2c_adapter);
  
  pr_info("Bus Driver Added!!!\n");
  return ret;
}

/*
** Module Exit function
*/
static void __exit MyDevice_driver_exit(void)
{
  i2c_del_adapter(&MyDevice_i2c_adapter);
  pr_info("Bus Driver Removed!!!\n");
}

module_init(MyDevice_driver_init);
module_exit(MyDevice_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ali Mahjoub");
MODULE_DESCRIPTION("I2C Bus driver ");
MODULE_VERSION("1.37");