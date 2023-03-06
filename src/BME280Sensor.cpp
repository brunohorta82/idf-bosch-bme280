#include "BME280Sensor.hpp"
#include "BoschBME280Interface.hpp"

namespace Environment
{
    esp_err_t BME280Sensor::init()
    {
        esp_err_t result = ESP_OK;
        result = bme280Init(this);
        return result;
    }
    esp_err_t BME280Sensor::readTemperature()
    {
        return bme280Read();
    }

}
