# I2C-BUS-DRIVER(BIT-BANGING)
![results_1](https://github.com/Ali-Mahjoub/azertyu/blob/main/images/images.jpg)

<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#introduction">Introduction</a></li>  
    <li><a href="#walking-through-the-code">Walking through the code</a></li>
      <ul>
        <li><a href="#apis-used-for-the-i2c-bus-driver"> APIs used for the I2C bus driver</a></li>
        <li><a href="#implementation-of-the-i2c-communication-by-using-the-bitbanging-method">Implementation of the I2C communication by using the bit-banging method</a></li>
      </ul>
    </li>      
    
    <li><a href="#conclusion">Conclusion</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgements">Acknowledgements</a></li>
       
  </ol>
</details>

## Introduction
The I2C bit banging is a technique for serial communications using software instead of a dedicated hardware module. This means that the code controls the state of the MCU pins, related to all parameters of the signals: timing, levels and synchronization.
Every I2C Hardware Device had its internal I2C bus drive for their specific use. So in this project i will implement a generic simple linux i2c bus driver that implements everything using Bit banging right from the START condition, STOP condition, Read ACKNWOLEGEMENT condition, etc 

## Walking through the code:

### APIs used for the I2C bus driver 

There are two structures that you need to use in order to write the i2c bus driver in the Linux kernel.
* **Algorithm Structure**
```c
 
// I2C algorithm Structure

static struct i2c_algorithm MyDevice_i2c_algorithm = {
  .smbus_xfer     = MyDevice_smbus_xfer,
  .master_xfer    = MyDevice_i2c_xfer,
  .functionality  = MyDevice_func,
};
```
Where,

<br > **master_xfer**   — Issue a set of i2c transactions to the given I2C adapter defined by the msgs array, with num messages available to transfer via the adapter specified by adap. This function will be called whenever we call I2C read-write APIs from the client driver.<br /> 
<br > **smbus_xfer**   — Issue SMBus transactions to the given I2C adapter. If this is not present, then the bus layer will try and convert the SMBus calls into I2C transfers instead. This function will be called whenever we call SMBus read-write APIs from the client driver.<br /> 
<br > **functionality** — Return the flags that this algorithm/adapter pair supports from the I2C_FUNC_* flags.<br /> 
* Adapter Structure<br />
This structure is used to identify a physical i2c bus along with the access algorithms necessary to access it.
```c
 
// I2C adapter Structure

static struct i2c_adapter MyDevice_i2c_adapter = {
  .owner  = THIS_MODULE,
  .class  = I2C_CLASS_HWMON,//| I2C_CLASS_SPD,
  .algo   = &MyDevice_i2c_algorithm,
  .name   = ADAPTER_NAME,
  .nr     = 5,
};
```
Where,
<br >***owner**      — Owner of the module(usually set this to THIS_MODULE).<br />
<br >**class**      — the type of I2C class devices that this driver supports. Usually, this is set to any one of the I2C_CLASS_* based on our need.<br />
<br >***algo**       — a pointer to the struct i2c_algorithm structure<br />
<br >**nr**         — bus number which you want to create. This will be applicable only for i2c_add_numbered_adapter().<br />
<br >**char name[I2C_NAME_SIZE]** — I2C bus driver name. This value shows up in the sysfs filename associated with this I2C adapter.<br />
After you create the two structures, then we have to add the adapter to the i2c subsystem.
* **Add the adapter to the subsystem**<br />
```c
static int __init MyDevice_driver_init(void)
{
  int ret = -1;
  
  ret = i2c_add_numbered_adapter(&MyDevice_i2c_adapter);
  
  pr_info("Bus Driver Added!!!\n");
  return ret;
}
```
  **i2c_add_numbered_adapter**
This API is used to register the adapter to the subsystem. But it assigns the number that we asked for if only it is available. We have to initialize the member called nr in the i2c_adapter structure before calling this.
* **Delete the adapter from the subsystem**<br />
```c
static void __exit MyDevice_driver_exit(void)
{
  i2c_del_adapter(&MyDevice_i2c_adapter);
  pr_info("Bus Driver Removed!!!\n");
}
```
  **i2c_del_adapter**
This API is used to unregister the adapter from the subsystem.
### Implementation of the I2C communication by using the bitbanging method
* Reading and writing (setting or resetting) the SCL,SDA GPIOs:
* **Function to read the SCL GPIO**
```c
static bool MyDevice_I2C_Read_SCL(void)
{
  gpio_direction_input(SCL_GPIO);
  return gpio_get_value(SCL_GPIO);
}
```
* **Function to read the SDA GPIO**
```c
static bool MyDevice_I2C_Read_SDA(void)
{
  gpio_direction_input(SDA_GPIO);
  return gpio_get_value(SDA_GPIO);
}
```
* **Function to clear the SCL GPIO**
```c
static void MyDevice_I2C_Clear_SCL(void)
{
  gpio_direction_output(SCL_GPIO, 0);
  gpio_set_value(SCL_GPIO, 0);
}
```
* **Function to clear the SDA GPIO**
```c
static void MyDevice_I2C_Clear_SDA(void)
{
  gpio_direction_output(SDA_GPIO, 0);
  gpio_set_value(SDA_GPIO, 0);
}
```
* **Function to set the SCL GPIO**
```c
static void MyDevice_I2C_Set_SCL(void)
{
  gpio_direction_output(SCL_GPIO, 1);
  gpio_set_value(SCL_GPIO, 1);
}
```
* **Function to set the SDA GPIO**
```c
static void MyDevice_I2C_Set_SDA(void)
{
  gpio_direction_output(SDA_GPIO, 1);
  gpio_set_value(SDA_GPIO, 1);
}
```
### Initializing and Deinitializing the GPIOs:
```c
* **Function to Initialize the GPIOs**

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
    
    // configure the SCL GPIO as output, We will change the direction later as per our need
    
    gpio_direction_output(SCL_GPIO, 1);
    /*
    ** configure the SDA GPIO as output, We will change the 
    ** direction later as per our need.
    */
    gpio_direction_output(SDA_GPIO, 1);
  } while(false);
  return ret;  
}
```
* **Function to Deinitialize the GPIOs*
```c
static void MyDevice_I2C_DeInit( void )
{
  //free both the GPIOs
  gpio_free(SCL_GPIO);
  gpio_free(SDA_GPIO);
}
```
### Starting and Stopping conditions:

This functioning conditions are translated as:
* **Function to send the START condition**
```c
 static void MyDevice_I2C_Sart( void )
{
  MyDevice_I2C_Set_SDA();
  MyDevice_I2C_Set_SCL();
  I2C_DELAY;
  MyDevice_I2C_Clear_SDA();
  I2C_DELAY;  
  MyDevice_I2C_Clear_SCL();
  I2C_DELAY;  
}
```
* **Function to send the STOP condition**
```c
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
```
### Checking ACK/NACK:
Function to reads the SDA to get the status and Returns 0 for NACK, returns 1 for ACK
```c
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
```
### Sending adress:
Function to send the 7-bit address to the slave 
```c
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
  (is_read) ? MyDevice_I2C_Set_SDA() :MyDevice_I2C_Clear_SDA();  //read = 1, write = 0
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
```
 ### Writing a byte to the slave function:
```c
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
```
### Reading a byte from the slave function:
```c
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
```          
  ## Conclusion:
In this project, we successfullty detected and tracked a hand and its landmarks ,using the mediapipe module, and were able to extract data in order to create an interactive hand gesture mini-game with basic gameplay features such as  score , difficulty level and losing conditions.
  
  ### Contact:
* Mail : ali.mahjoub1998@gmail.com 
* Linked-in profile: https://www.linkedin.com/in/ali-mahjoub-b83a86196/

### Acknowledgements:
* The Embetronicx admins: https://embetronicx.com/

