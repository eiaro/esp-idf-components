# API Reference

## Header files

- [include/max31856.h](#file-includemax31856h)
- [include/max31856.hpp](#file-includemax31856hpp)
- [include/max31856_priv.h](#file-includemax31856_privh)

## File include/max31856.h





## Structures and Types

| Type | Name |
| ---: | :--- |
| enum  | [**max31856\_average\_t**](#enum-max31856_average_t)  <br> |
| struct | [**max31856\_dev**](#struct-max31856_dev) <br>_Structure representing a MAX31856 device instance._ |
| typedef struct [**max31856\_dev**](#struct-max31856_dev) | [**max31856\_dev\_t**](#typedef-max31856_dev_t)  <br>_Structure representing a MAX31856 device instance._ |
| enum  | [**max31856\_oc\_detection\_t**](#enum-max31856_oc_detection_t)  <br> |
| enum  | [**max31856\_thermocouple\_type\_t**](#enum-max31856_thermocouple_type_t)  <br> |

## Functions

| Type | Name |
| ---: | :--- |
|  esp\_err\_t | [**max31856\_init**](#function-max31856_init) ([**max31856\_dev\_t**](#typedef-max31856_dev_t) \*data) <br>_Initialize the MAX31856 device._ |
|  esp\_err\_t | [**max31856\_read\_cold\_junction**](#function-max31856_read_cold_junction) ([**max31856\_dev\_t**](#typedef-max31856_dev_t) \*data, float \*temperature) <br>_Reads the cold junction temperature from the MAX31856 device._ |
|  esp\_err\_t | [**max31856\_read\_fault\_status**](#function-max31856_read_fault_status) ([**max31856\_dev\_t**](#typedef-max31856_dev_t) \*data, uint8\_t \*status) <br>_Reads the fault status register from the MAX31856 device._ |
|  esp\_err\_t | [**max31856\_read\_thermocouple**](#function-max31856_read_thermocouple) ([**max31856\_dev\_t**](#typedef-max31856_dev_t) \*data, float \*temperature) <br>_Reads the thermocouple temperature from the MAX31856 device._ |


## Structures and Types Documentation

### enum `max31856_average_t`

```c
enum max31856_average_t {
    MAX31856_AVG_1 = 0b000,
    MAX31856_AVG_2 = 0b001,
    MAX31856_AVG_4 = 0b010,
    MAX31856_AVG_8 = 0b011,
    MAX31856_AVG_16 = 0b100
};
```

### struct `max31856_dev`

_Structure representing a MAX31856 device instance._

This structure holds all relevant information and configuration required to interface with the MAX31856 thermocouple-to-digital converter. Populate and manage this structure when initializing and communicating with the device.

Variables:

-  [**max31856\_average\_t**](#enum-max31856_average_t) averaging  

-  bool filter_50hz  

-  [**max31856\_oc\_detection\_t**](#enum-max31856_oc_detection_t) oc_detection  

-  spi\_device\_handle\_t spi_dev  

-  [**max31856\_thermocouple\_type\_t**](#enum-max31856_thermocouple_type_t) thermocouple_type  

-  bool use_cold_junction  

### typedef `max31856_dev_t`

_Structure representing a MAX31856 device instance._
```c
typedef struct max31856_dev max31856_dev_t;
```


This structure holds all relevant information and configuration required to interface with the MAX31856 thermocouple-to-digital converter. Populate and manage this structure when initializing and communicating with the device.
### enum `max31856_oc_detection_t`

```c
enum max31856_oc_detection_t {
    MAX31856_OC_DETECT_OFF = 0b00,
    MAX31856_OC_DETECT_1 = 0b01,
    MAX31856_OC_DETECT_2 = 0b10,
    MAX31856_OC_DETECT_3 = 0b11
};
```

### enum `max31856_thermocouple_type_t`

```c
enum max31856_thermocouple_type_t {
    MAX31856_TC_TYPE_B = 0b0000,
    MAX31856_TC_TYPE_E = 0b0001,
    MAX31856_TC_TYPE_J = 0b0010,
    MAX31856_TC_TYPE_K = 0b0011,
    MAX31856_TC_TYPE_N = 0b0100,
    MAX31856_TC_TYPE_R = 0b0101,
    MAX31856_TC_TYPE_S = 0b0110,
    MAX31856_TC_TYPE_T = 0b0111
};
```


## Functions Documentation

### function `max31856_init`

_Initialize the MAX31856 device._
```c
esp_err_t max31856_init (
    max31856_dev_t *data
) 
```


This function initializes the MAX31856 device structure and configures the necessary hardware interfaces.



**Parameters:**


* `data` Pointer to the MAX31856 device structure to initialize. 


**Returns:**



* ESP\_OK on success
* Appropriate error code from esp\_err\_t on failure
### function `max31856_read_cold_junction`

_Reads the cold junction temperature from the MAX31856 device._
```c
esp_err_t max31856_read_cold_junction (
    max31856_dev_t *data,
    float *temperature
) 
```


This function retrieves the current cold junction (reference junction) temperature from the specified MAX31856 device and stores the result in the provided temperature pointer.



**Parameters:**


* `data` Pointer to the MAX31856 device structure. 
* `temperature` Pointer to a float variable where the temperature (in degrees Celsius) will be stored.


**Returns:**



* ESP\_OK on success
* Appropriate esp\_err\_t error code otherwise
### function `max31856_read_fault_status`

_Reads the fault status register from the MAX31856 device._
```c
esp_err_t max31856_read_fault_status (
    max31856_dev_t *data,
    uint8_t *status
) 
```


This function retrieves the current fault status from the MAX31856 thermocouple-to-digital converter and stores it in the provided status variable.



**Parameters:**


* `data` Pointer to the MAX31856 device structure. 
* `status` Pointer to a variable where the fault status byte will be stored.


**Returns:**



* ESP\_OK on success
* Appropriate error code otherwise
### function `max31856_read_thermocouple`

_Reads the thermocouple temperature from the MAX31856 device._
```c
esp_err_t max31856_read_thermocouple (
    max31856_dev_t *data,
    float *temperature
) 
```


This function communicates with the MAX31856 thermocouple-to-digital converter to retrieve the current thermocouple temperature measurement.



**Parameters:**


* `data` Pointer to the MAX31856 device structure. 
* `temperature` Pointer to a float variable where the measured temperature (in °C) will be stored.


**Returns:**



* ESP\_OK on success
* Appropriate esp\_err\_t error code otherwise


## File include/max31856.hpp











## File include/max31856_priv.h






## Functions

| Type | Name |
| ---: | :--- |
|  esp\_err\_t | [**max31856\_read**](#function-max31856_read) ([**max31856\_dev\_t**](#typedef-max31856_dev_t) \*data, uint8\_t addr, uint8\_t \*value) <br>_Read a value from a specific register address of the MAX31856 device._ |
|  esp\_err\_t | [**max31856\_write**](#function-max31856_write) ([**max31856\_dev\_t**](#typedef-max31856_dev_t) \*data, uint8\_t addr, uint8\_t value) <br>_Write a value to a specific register address of the MAX31856 device._ |

## Macros

| Type | Name |
| ---: | :--- |
| define  | [**GENMASK**](#define-genmask) (h, l) (((1UL &lt;&lt; ((h) - (l) + 1)) - 1) &lt;&lt; (l))<br>_Creates a contiguous bitmask spanning from bit position 'l' to 'h' (inclusive)._ |
| define  | [**MAX31856\_CJHF\_REG**](#define-max31856_cjhf_reg)  0x03<br> |
| define  | [**MAX31856\_CJLF\_REG**](#define-max31856_cjlf_reg)  0x04<br> |
| define  | [**MAX31856\_CJTH\_REG**](#define-max31856_cjth_reg)  0x0A<br> |
| define  | [**MAX31856\_CJTL\_REG**](#define-max31856_cjtl_reg)  0x0B<br> |
| define  | [**MAX31856\_CJTO\_REG**](#define-max31856_cjto_reg)  0x09<br> |
| define  | [**MAX31856\_CR0\_1SHOT**](#define-max31856_cr0_1shot)  BIT(6)<br> |
| define  | [**MAX31856\_CR0\_AUTOCONVERT**](#define-max31856_cr0_autoconvert)  BIT(7)<br> |
| define  | [**MAX31856\_CR0\_CJ**](#define-max31856_cr0_cj)  BIT(3)<br> |
| define  | [**MAX31856\_CR0\_FAULT**](#define-max31856_cr0_fault)  BIT(2)<br> |
| define  | [**MAX31856\_CR0\_OC\_MASK**](#define-max31856_cr0_oc_mask)  [**GENMASK**](#define-genmask)(5, 4)<br> |
| define  | [**MAX31856\_CR0\_OC\_SHIFT**](#define-max31856_cr0_oc_shift)  4<br> |
| define  | [**MAX31856\_CR0\_REG**](#define-max31856_cr0_reg)  0x00<br> |
| define  | [**MAX31856\_CR1\_AVERAGE\_MASK**](#define-max31856_cr1_average_mask)  [**GENMASK**](#define-genmask)(6, 4)<br> |
| define  | [**MAX31856\_CR1\_AVERAGE\_SHIFT**](#define-max31856_cr1_average_shift)  4<br> |
| define  | [**MAX31856\_CR1\_REG**](#define-max31856_cr1_reg)  0x01<br> |
| define  | [**MAX31856\_CR1\_TCTYPE\_MASK**](#define-max31856_cr1_tctype_mask)  [**GENMASK**](#define-genmask)(3, 0)<br> |
| define  | [**MAX31856\_LTCBH\_REG**](#define-max31856_ltcbh_reg)  0x0C<br> |
| define  | [**MAX31856\_LTCBL\_REG**](#define-max31856_ltcbl_reg)  0x0E<br> |
| define  | [**MAX31856\_LTCBM\_REG**](#define-max31856_ltcbm_reg)  0x0D<br> |
| define  | [**MAX31856\_LTHFTH\_REG**](#define-max31856_lthfth_reg)  0x05<br> |
| define  | [**MAX31856\_LTHFTL\_REG**](#define-max31856_lthftl_reg)  0x06<br> |
| define  | [**MAX31856\_LTLFTH\_REG**](#define-max31856_ltlfth_reg)  0x07<br> |
| define  | [**MAX31856\_LTLFTL\_REG**](#define-max31856_ltlftl_reg)  0x08<br> |
| define  | [**MAX31856\_MASK\_REG**](#define-max31856_mask_reg)  0x02<br> |
| define  | [**MAX31856\_RD\_WR\_MASK**](#define-max31856_rd_wr_mask)  BIT(7)<br> |
| define  | [**MAX31856\_SR\_REG**](#define-max31856_sr_reg)  0x0F<br> |


## Functions Documentation

### function `max31856_read`

_Read a value from a specific register address of the MAX31856 device._
```c
esp_err_t max31856_read (
    max31856_dev_t *data,
    uint8_t addr,
    uint8_t *value
) 
```


This function reads a single byte value from the specified register address of the MAX31856 thermocouple-to-digital converter via SPI.



**Parameters:**


* `data` Pointer to the MAX31856 device structure. 
* `addr` Register address to read from. 
* `value` Pointer to store the read value.


**Returns:**



* ESP\_OK on success
* Appropriate esp\_err\_t error code otherwise
### function `max31856_write`

_Write a value to a specific register address of the MAX31856 device._
```c
esp_err_t max31856_write (
    max31856_dev_t *data,
    uint8_t addr,
    uint8_t value
) 
```


This function writes a single byte value to the specified register address of the MAX31856 thermocouple-to-digital converter via SPI.



**Parameters:**


* `data` Pointer to the MAX31856 device structure. 
* `addr` Register address to write to. 
* `value` Value to write to the register.


**Returns:**



* ESP\_OK on success
* Appropriate esp\_err\_t error code otherwise

## Macros Documentation

### define `GENMASK`

_Creates a contiguous bitmask spanning from bit position 'l' to 'h' (inclusive)._
```c
#define GENMASK (
    h,
    l
) (((1UL << ((h) - (l) + 1)) - 1) << (l))
```


This macro generates a value where bits in positions l through h are set to 1, and all other bits are set to 0. It's commonly used for masking specific bit fields within registers.



**Parameters:**


* `h` The high bit position (most significant bit of the mask) 
* `l` The low bit position (least significant bit of the mask) 


**Returns:**

A value with bits set from position l to position h


Example:[**GENMASK(3, 0)**](#define-genmask) produces 0b1111 (0xF)[**GENMASK(7, 4)**](#define-genmask) produces 0b11110000 (0xF0)
### define `MAX31856_CJHF_REG`

```c
#define MAX31856_CJHF_REG 0x03
```

### define `MAX31856_CJLF_REG`

```c
#define MAX31856_CJLF_REG 0x04
```

### define `MAX31856_CJTH_REG`

```c
#define MAX31856_CJTH_REG 0x0A
```

### define `MAX31856_CJTL_REG`

```c
#define MAX31856_CJTL_REG 0x0B
```

### define `MAX31856_CJTO_REG`

```c
#define MAX31856_CJTO_REG 0x09
```

### define `MAX31856_CR0_1SHOT`

```c
#define MAX31856_CR0_1SHOT BIT(6)
```

### define `MAX31856_CR0_AUTOCONVERT`

```c
#define MAX31856_CR0_AUTOCONVERT BIT(7)
```

### define `MAX31856_CR0_CJ`

```c
#define MAX31856_CR0_CJ BIT(3)
```

### define `MAX31856_CR0_FAULT`

```c
#define MAX31856_CR0_FAULT BIT(2)
```

### define `MAX31856_CR0_OC_MASK`

```c
#define MAX31856_CR0_OC_MASK GENMASK (5, 4)
```

### define `MAX31856_CR0_OC_SHIFT`

```c
#define MAX31856_CR0_OC_SHIFT 4
```

### define `MAX31856_CR0_REG`

```c
#define MAX31856_CR0_REG 0x00
```

### define `MAX31856_CR1_AVERAGE_MASK`

```c
#define MAX31856_CR1_AVERAGE_MASK GENMASK (6, 4)
```

### define `MAX31856_CR1_AVERAGE_SHIFT`

```c
#define MAX31856_CR1_AVERAGE_SHIFT 4
```

### define `MAX31856_CR1_REG`

```c
#define MAX31856_CR1_REG 0x01
```

### define `MAX31856_CR1_TCTYPE_MASK`

```c
#define MAX31856_CR1_TCTYPE_MASK GENMASK (3, 0)
```

### define `MAX31856_LTCBH_REG`

```c
#define MAX31856_LTCBH_REG 0x0C
```

### define `MAX31856_LTCBL_REG`

```c
#define MAX31856_LTCBL_REG 0x0E
```

### define `MAX31856_LTCBM_REG`

```c
#define MAX31856_LTCBM_REG 0x0D
```

### define `MAX31856_LTHFTH_REG`

```c
#define MAX31856_LTHFTH_REG 0x05
```

### define `MAX31856_LTHFTL_REG`

```c
#define MAX31856_LTHFTL_REG 0x06
```

### define `MAX31856_LTLFTH_REG`

```c
#define MAX31856_LTLFTH_REG 0x07
```

### define `MAX31856_LTLFTL_REG`

```c
#define MAX31856_LTLFTL_REG 0x08
```

### define `MAX31856_MASK_REG`

```c
#define MAX31856_MASK_REG 0x02
```

### define `MAX31856_RD_WR_MASK`

```c
#define MAX31856_RD_WR_MASK BIT(7)
```

### define `MAX31856_SR_REG`

```c
#define MAX31856_SR_REG 0x0F
```


