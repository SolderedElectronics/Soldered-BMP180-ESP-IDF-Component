idf_component_register(SRCS "src/soldered_bmp180.c"
                            "src/bmp180_compensation.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_i2c)
