# Soldered BMP180 Temperature and Pressure Sensor Component

| ![Pressure & temperature sensor BMP180 breakout](https://soldered.com/cdn/shop/files/333060_featured-photo_70581e_3ec34161-1c04-4f2b-b780-f5809f582bcf.jpg) |
| :---: |
| [Pressure & temperature sensor BMP180 breakout](https://www.solde.red/333060) |

The BMP180 is a Bosch barometric pressure and temperature sensor: 300 to 1100 hPa with 0.01 hPa resolution, and temperature with 0.1 °C resolution. Since pressure falls with height, it can also estimate altitude. The board uses I2C at the fixed address 0x77 and connects with an easyC/Qwiic cable, see the [Qwiic ecosystem](https://soldered.com/collections/qwiic-ecosystem).

### Installation

Add it to your project with the component manager:

```bash
idf.py add-dependency "solderedelectronics/soldered-bmp180-esp-idf-component"
```

Or clone this repository into your project's `components/` folder.

### Usage

The I2C bus belongs to your application, not to the driver, so that other Qwiic devices can share it. Create the bus first, then hand it over:

```c
#include "driver/i2c_master.h"
#include "soldered_bmp180.h"

i2c_master_bus_config_t bus_cfg = {
    .i2c_port = I2C_NUM_0,
    .sda_io_num = GPIO_NUM_21,
    .scl_io_num = GPIO_NUM_22,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};
i2c_master_bus_handle_t bus;
ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

bmp180_t sensor;
ESP_ERROR_CHECK(soldered_bmp180_init(&sensor, bus, 3)); // pressure oversampling 0..3

float temperature, pressure;
if (soldered_bmp180_read(&sensor, &temperature, &pressure) == ESP_OK) {
    printf("%.1f degC, %.2f hPa\n", temperature, pressure);
}
```

`soldered_bmp180_sealevel()` converts the absolute pressure to sea-level pressure for a known altitude, and `soldered_bmp180_altitude()` turns a pressure and a reference pressure into an altitude.

### Examples

- **basic_readings** - reads temperature and pressure every 5 s and prints the sea-level pressure and altitude

```bash
cd examples/basic_readings
idf.py set-target esp32
idf.py build flash monitor
```

### Repository Contents

- **/src** - source files (.c)
- **/include** - header files (.h)
- **/examples** - examples for using the library
- **/test** - host-side test of the compensation math
- **_other_** - idf_component.yml manifest file for ESP Component Registry

### Hardware design

You can find hardware design for this board in Pressure & temperature sensor BMP180 breakout hardware repository.

### Documentation

Access library documentation [here](https://docs.soldered.com/).

### About Soldered

<img src="https://raw.githubusercontent.com/SolderedElectronics/Soldered-Generic-Arduino-Library/dev/extras/Soldered-logo-color.png" alt="soldered-logo" width="500"/>

At Soldered, we design and manufacture a wide selection of electronic products to help you turn your ideas into acts and bring you one step closer to your final project. Our products are intented for makers and crafted in-house by our experienced team in Osijek, Croatia. We believe that sharing is a crucial element for improvement and innovation, and we work hard to stay connected with all our makers regardless of their skill or experience level. Therefore, all our products are open-source. Finally, we always have your back. If you face any problem concerning either your shopping experience or your electronics project, our team will help you deal with it, offering efficient customer service and cost-free technical support anytime. Some of those might be useful for you:

- [Web Store](https://www.soldered.com/shop)
- [Tutorials & Projects](https://soldered.com/learn)
- [Documentation](https://docs.soldered.com)

### Original source

The compensation math follows the integer algorithm in the Bosch BMP180 datasheet (BST-BMP180-DS000, section 3.5).

### Open-source license

Soldered invests vast amounts of time into hardware & software for these products, which are all open-source. Please support future development by buying one of our products.

Check license details in the LICENSE file. Long story short, use these open-source files for any purpose you want to, as long as you apply the same open-source licence to it and disclose the original source. No warranty - all designs in this repository are distributed in the hope that they will be useful, but without any warranty. They are provided "AS IS", therefore without warranty of any kind, either expressed or implied. The entire quality and performance of what you do with the contents of this repository are your responsibility. In no event, Soldered (TAVU) will be liable for your damages, losses, including any general, special, incidental or consequential damage arising out of the use or inability to use the contents of this repository.

## Have fun!

And thank you from your fellow makers at Soldered Electronics.
