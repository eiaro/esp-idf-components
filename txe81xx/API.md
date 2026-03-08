# API Reference

## Header files

- [include/txe81xx.h](#file-includetxe81xxh)

## File include/txe81xx.h





## Structures and Types

| Type | Name |
| ---: | :--- |
| enum  | [**txe81xx\_chip\_t**](#enum-txe81xx_chip_t)  <br> |
| struct | [**txe81xx\_handle\_config\_t**](#struct-txe81xx_handle_config_t) <br> |
| typedef [**txe81xx\_t**](#typedef-txe81xx_t) \* | [**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t)  <br> |
| typedef void(\* | [**txe81xx\_irq\_cb\_t**](#typedef-txe81xx_irq_cb_t)  <br> |
| struct | [**txe81xx\_irq\_config\_t**](#struct-txe81xx_irq_config_t) <br> |
| enum  | [**txe81xx\_port\_t**](#enum-txe81xx_port_t)  <br> |
| struct | [**txe81xx\_spi\_config\_t**](#struct-txe81xx_spi_config_t) <br> |
| typedef struct [**txe81xx\_t**](#typedef-txe81xx_t) | [**txe81xx\_t**](#typedef-txe81xx_t)  <br> |
| typedef esp\_err\_t(\* | [**txe81xx\_xfer24\_fn**](#typedef-txe81xx_xfer24_fn)  <br> |

## Functions

| Type | Name |
| ---: | :--- |
|  esp\_err\_t | [**txe81xx\_deinit**](#function-txe81xx_deinit) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev) <br> |
|  esp\_err\_t | [**txe81xx\_glitch\_filter\_enable**](#function-txe81xx_glitch_filter_enable) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t pins\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_init\_handle**](#function-txe81xx_init_handle) (const [**txe81xx\_handle\_config\_t**](#struct-txe81xx_handle_config_t) \*cfg, [**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) \*out) <br> |
|  esp\_err\_t | [**txe81xx\_init\_spi**](#function-txe81xx_init_spi) (const [**txe81xx\_spi\_config\_t**](#struct-txe81xx_spi_config_t) \*cfg, [**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) \*out) <br> |
|  esp\_err\_t | [**txe81xx\_irq\_clear\_pending**](#function-txe81xx_irq_clear_pending) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev) <br> |
|  esp\_err\_t | [**txe81xx\_irq\_configure\_port**](#function-txe81xx_irq_configure_port) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, bool smart\_enable, uint8\_t mask) <br> |
|  esp\_err\_t | [**txe81xx\_irq\_enable\_pins**](#function-txe81xx_irq_enable_pins) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t pins\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_irq\_install**](#function-txe81xx_irq_install) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, const [**txe81xx\_irq\_config\_t**](#struct-txe81xx_irq_config_t) \*cfg) <br> |
|  esp\_err\_t | [**txe81xx\_irq\_read\_flags**](#function-txe81xx_irq_read_flags) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t \*out\_flags) <br> |
|  esp\_err\_t | [**txe81xx\_irq\_read\_port\_status**](#function-txe81xx_irq_read_port_status) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, uint8\_t \*out\_status) <br> |
|  esp\_err\_t | [**txe81xx\_irq\_uninstall**](#function-txe81xx_irq_uninstall) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev) <br> |
|  esp\_err\_t | [**txe81xx\_read\_device\_id**](#function-txe81xx_read_device_id) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, uint8\_t \*out\_id) <br> |
|  esp\_err\_t | [**txe81xx\_read\_fault\_status**](#function-txe81xx_read_fault_status) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, uint8\_t \*out\_status) <br> |
|  esp\_err\_t | [**txe81xx\_read\_feature8**](#function-txe81xx_read_feature8) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, uint8\_t feature, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t \*out\_value) <br> |
|  esp\_err\_t | [**txe81xx\_read\_inputs**](#function-txe81xx_read_inputs) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t \*out\_value) <br> |
|  esp\_err\_t | [**txe81xx\_read\_outputs**](#function-txe81xx_read_outputs) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t \*out\_value) <br> |
|  esp\_err\_t | [**txe81xx\_set\_direction**](#function-txe81xx_set_direction) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t dir\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_set\_pull\_enable\_mask**](#function-txe81xx_set_pull_enable_mask) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t enable\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_set\_pull\_select\_mask**](#function-txe81xx_set_pull_select_mask) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t select\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_set\_pushpull\_od**](#function-txe81xx_set_pushpull_od) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t mask) <br> |
|  esp\_err\_t | [**txe81xx\_software\_reset**](#function-txe81xx_software_reset) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev) <br> |
|  esp\_err\_t | [**txe81xx\_update\_direction**](#function-txe81xx_update_direction) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t set\_mask, uint8\_t clear\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_update\_glitch\_filter**](#function-txe81xx_update_glitch_filter) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t set\_mask, uint8\_t clear\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_update\_pull\_enable\_mask**](#function-txe81xx_update_pull_enable_mask) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t set\_mask, uint8\_t clear\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_update\_pull\_select\_mask**](#function-txe81xx_update_pull_select_mask) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t set\_mask, uint8\_t clear\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_update\_pushpull\_od**](#function-txe81xx_update_pushpull_od) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t set\_mask, uint8\_t clear\_mask) <br> |
|  esp\_err\_t | [**txe81xx\_write\_feature8**](#function-txe81xx_write_feature8) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, uint8\_t feature, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t value) <br> |
|  esp\_err\_t | [**txe81xx\_write\_outputs**](#function-txe81xx_write_outputs) ([**txe81xx\_handle\_t**](#typedef-txe81xx_handle_t) dev, [**txe81xx\_port\_t**](#enum-txe81xx_port_t) port, uint8\_t value) <br> |


## Structures and Types Documentation

### enum `txe81xx_chip_t`

```c
enum txe81xx_chip_t {
    TXE_CHIP_8116 = 0,
    TXE_CHIP_8124 = 1
};
```

### struct `txe81xx_handle_config_t`


Variables:

-  [**txe81xx\_chip\_t**](#enum-txe81xx_chip_t) chip  

-  spi\_device\_handle\_t spi  

-  bool use_polling  

-  [**txe81xx\_xfer24\_fn**](#typedef-txe81xx_xfer24_fn) xfer24  

-  void \* xfer24_ctx  

### typedef `txe81xx_handle_t`

```c
typedef txe81xx_t* txe81xx_handle_t;
```

### typedef `txe81xx_irq_cb_t`

```c
typedef void(* txe81xx_irq_cb_t) (txe81xx_port_t port, uint8_t flags, uint8_t port_state, void *user_ctx);
```

### struct `txe81xx_irq_config_t`


Variables:

-  bool active_low  

-  [**txe81xx\_irq\_cb\_t**](#typedef-txe81xx_irq_cb_t) cb  

-  gpio\_num\_t int_gpio  

-  int task_core  

-  UBaseType\_t task_prio  

-  uint32\_t task_stack  

-  void \* user_ctx  

### enum `txe81xx_port_t`

```c
enum txe81xx_port_t {
    TXE_PORT0 = 0,
    TXE_PORT1 = 1,
    TXE_PORT2 = 2
};
```

### struct `txe81xx_spi_config_t`


Variables:

-  [**txe81xx\_chip\_t**](#enum-txe81xx_chip_t) chip  

-  int clock_hz  

-  int cs_gpio  

-  spi\_host\_device\_t host  

-  uint8\_t mode  

-  int queue_size  

-  bool use_polling  

### typedef `txe81xx_t`

```c
typedef struct txe81xx_t txe81xx_t;
```

### typedef `txe81xx_xfer24_fn`

```c
typedef esp_err_t(* txe81xx_xfer24_fn) (void *user_ctx, const uint8_t tx[3], uint8_t rx[3]);
```


## Functions Documentation

### function `txe81xx_deinit`

```c
esp_err_t txe81xx_deinit (
    txe81xx_handle_t dev
) 
```

### function `txe81xx_glitch_filter_enable`

```c
esp_err_t txe81xx_glitch_filter_enable (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t pins_mask
) 
```


Enable glitch filter for selected pins (feature 0x0D).
### function `txe81xx_init_handle`

```c
esp_err_t txe81xx_init_handle (
    const txe81xx_handle_config_t *cfg,
    txe81xx_handle_t *out
) 
```

### function `txe81xx_init_spi`

```c
esp_err_t txe81xx_init_spi (
    const txe81xx_spi_config_t *cfg,
    txe81xx_handle_t *out
) 
```

### function `txe81xx_irq_clear_pending`

```c
esp_err_t txe81xx_irq_clear_pending (
    txe81xx_handle_t dev
) 
```

### function `txe81xx_irq_configure_port`

```c
esp_err_t txe81xx_irq_configure_port (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    bool smart_enable,
    uint8_t mask
) 
```


Configure TXE interrupt for a port:

* smart\_enable: if true =&gt; smart interrupt enabled (bit=0), else disabled (bit=1)
* mask: 1=masked, 0=enabled per pin (datasheet)
### function `txe81xx_irq_enable_pins`

```c
esp_err_t txe81xx_irq_enable_pins (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t pins_mask
) 
```


Convenience: enable interrupts for specific pins (pins\_mask=1 means enable). Internally writes Interrupt Mask reg with 0 for enabled pins.
### function `txe81xx_irq_install`

```c
esp_err_t txe81xx_irq_install (
    txe81xx_handle_t dev,
    const txe81xx_irq_config_t *cfg
) 
```


Installs ESP GPIO ISR and starts an IRQ handling task. Does NOT configure TXE interrupt registers by itself.
### function `txe81xx_irq_read_flags`

```c
esp_err_t txe81xx_irq_read_flags (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t *out_flags
) 
```

### function `txe81xx_irq_read_port_status`

```c
esp_err_t txe81xx_irq_read_port_status (
    txe81xx_handle_t dev,
    uint8_t *out_status
) 
```

### function `txe81xx_irq_uninstall`

```c
esp_err_t txe81xx_irq_uninstall (
    txe81xx_handle_t dev
) 
```


Stops IRQ task and removes ISR handler.
### function `txe81xx_read_device_id`

```c
esp_err_t txe81xx_read_device_id (
    txe81xx_handle_t dev,
    uint8_t *out_id
) 
```

### function `txe81xx_read_fault_status`

```c
esp_err_t txe81xx_read_fault_status (
    txe81xx_handle_t dev,
    uint8_t *out_status
) 
```

### function `txe81xx_read_feature8`

```c
esp_err_t txe81xx_read_feature8 (
    txe81xx_handle_t dev,
    uint8_t feature,
    txe81xx_port_t port,
    uint8_t *out_value
) 
```

### function `txe81xx_read_inputs`

```c
esp_err_t txe81xx_read_inputs (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t *out_value
) 
```

### function `txe81xx_read_outputs`

```c
esp_err_t txe81xx_read_outputs (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t *out_value
) 
```

### function `txe81xx_set_direction`

```c
esp_err_t txe81xx_set_direction (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t dir_mask
) 
```

### function `txe81xx_set_pull_enable_mask`

```c
esp_err_t txe81xx_set_pull_enable_mask (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t enable_mask
) 
```

### function `txe81xx_set_pull_select_mask`

```c
esp_err_t txe81xx_set_pull_select_mask (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t select_mask
) 
```

### function `txe81xx_set_pushpull_od`

```c
esp_err_t txe81xx_set_pushpull_od (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t mask
) 
```

### function `txe81xx_software_reset`

```c
esp_err_t txe81xx_software_reset (
    txe81xx_handle_t dev
) 
```

### function `txe81xx_update_direction`

```c
esp_err_t txe81xx_update_direction (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t set_mask,
    uint8_t clear_mask
) 
```

### function `txe81xx_update_glitch_filter`

```c
esp_err_t txe81xx_update_glitch_filter (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t set_mask,
    uint8_t clear_mask
) 
```

### function `txe81xx_update_pull_enable_mask`

```c
esp_err_t txe81xx_update_pull_enable_mask (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t set_mask,
    uint8_t clear_mask
) 
```

### function `txe81xx_update_pull_select_mask`

```c
esp_err_t txe81xx_update_pull_select_mask (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t set_mask,
    uint8_t clear_mask
) 
```

### function `txe81xx_update_pushpull_od`

```c
esp_err_t txe81xx_update_pushpull_od (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t set_mask,
    uint8_t clear_mask
) 
```

### function `txe81xx_write_feature8`

```c
esp_err_t txe81xx_write_feature8 (
    txe81xx_handle_t dev,
    uint8_t feature,
    txe81xx_port_t port,
    uint8_t value
) 
```

### function `txe81xx_write_outputs`

```c
esp_err_t txe81xx_write_outputs (
    txe81xx_handle_t dev,
    txe81xx_port_t port,
    uint8_t value
) 
```



