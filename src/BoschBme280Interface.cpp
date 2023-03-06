
#include "BoschBME280Interface.hpp"
#include "BME280Sensor.hpp"
#include "../driver/bme280_defs.h"
#include <esp_log.h>
#include "I2C.hpp"
#include <memory>
#include <cstring>

using namespace std;
namespace Environment
{
    struct bme280_dev bme280Device;
    uint32_t bme280MinDelay;
    BME280Sensor *_bme280Sensor;
    void boschDelayUs(uint32_t period, void *intf_ptr)
    {
        esp_rom_delay_us(period);
    }

    int8_t boschI2cRead(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
    {
        uint8_t dev_addr = *(uint8_t *)intf_ptr;
        try
        {
            _bme280Sensor->getBus()->syncWrite(I2CAddress(dev_addr), {reg_addr});
            vector<uint8_t> data = _bme280Sensor->getBus()->syncRead(I2CAddress(dev_addr), len);
            memcpy(reg_data, data.data(), len);
            return ESP_OK;
        }
        catch (const I2CException &e)
        {
            ESP_LOGI("TAG", "I2C Exception with error: %s (0x%X)", e.what(), e.error);
            ESP_LOGI("TAG", "Couldn't read sensor!");
            return ESP_FAIL;
        }
    }

    int8_t boschI2cWrite(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
    {
        uint8_t dev_addr = *(uint8_t *)intf_ptr;
        try
        {
            vector<uint8_t> data;
            data.push_back(reg_addr);
            for (int i = 0; i < len; i++)
            {
                data.push_back(reg_data[i]);
            }
            _bme280Sensor->getBus()->syncWrite(I2CAddress(dev_addr), data);
            return ESP_OK;
        }
        catch (const I2CException &e)
        {
            ESP_LOGI("TAG", "I2C Exception with error: %s (0x%X)", e.what(), e.error);
            ESP_LOGI("TAG", "Couldn't write sensor!");
            return ESP_FAIL;
        }
    }

    esp_err_t bme280Init(BME280Sensor *environmentSensor)
    {
        _bme280Sensor = environmentSensor;

        int8_t rslt = BME280_OK;
        static uint8_t dev_addr{CONFIG_BME280_ADDRESS};

        bme280Device.intf_ptr = &dev_addr;
        bme280Device.intf = BME280_I2C_INTF;
        bme280Device.read = boschI2cRead;
        bme280Device.write = boschI2cWrite;
        bme280Device.delay_us = boschDelayUs;
        rslt = bme280_init(&bme280Device);
        uint8_t settings_sel;

        /* Recommended mode of operation: Indoor navigation */
        bme280Device.settings.osr_h = BME280_OVERSAMPLING_1X;
        bme280Device.settings.osr_p = BME280_OVERSAMPLING_16X;
        bme280Device.settings.osr_t = BME280_OVERSAMPLING_1X;
        bme280Device.settings.filter = BME280_FILTER_COEFF_OFF;
        bme280Device.settings.standby_time = BME280_STANDBY_TIME_1000_MS;

        settings_sel = BME280_OSR_PRESS_SEL;
        settings_sel |= BME280_OSR_TEMP_SEL;
        settings_sel |= BME280_OSR_HUM_SEL;
        settings_sel |= BME280_STANDBY_SEL;
        settings_sel |= BME280_FILTER_SEL;
        rslt = bme280_set_sensor_settings(settings_sel, &bme280Device);
        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &bme280Device);
        bme280MinDelay = bme280_cal_meas_delay(&bme280Device.settings);
        if (rslt == BME280_OK)
        {
            ESP_LOGI("TAG", "BME280 Init with success.");
            return ESP_OK;
        }
        else
        {
            ESP_LOGE("TAG", "BME280 can't be initialized.");
            return ESP_ERR_INVALID_STATE;
        }
    }

    esp_err_t bme280Read()
    {
        int8_t rslt = BME280_OK;
        struct bme280_data comp_data = {};
        bme280Device.delay_us(70, bme280Device.intf_ptr);
        bme280Device.delay_us(bme280MinDelay, bme280Device.intf_ptr);
        ESP_LOGI("BME280", "Temp(ºC): %0.2f   Pressure(Pa): %0.2f   Humidity(%%) %0.2f", comp_data.temperature, comp_data.pressure, comp_data.humidity);
        _bme280Sensor->setClimate(comp_data.temperature, comp_data.humidity, comp_data.pressure);
        return rslt == BME280_OK ? ESP_OK : ESP_ERR_INVALID_STATE;
    }

}