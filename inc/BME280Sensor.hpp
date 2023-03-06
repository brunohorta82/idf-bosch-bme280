#pragma once
#include "I2C.hpp"
using namespace std;
using namespace Components;
namespace Environment
{
    class BME280Sensor
    {
    private:
        shared_ptr<I2CMaster> i2cMasterBus;

    public:
        BME280Sensor(shared_ptr<I2CMaster> i2cMasterBus)
        {
            this->i2cMasterBus = i2cMasterBus;
        }
        shared_ptr<I2CMaster> getBus()
        {
            return this->i2cMasterBus;
        }
        esp_err_t init();
        esp_err_t readTemperature();
        esp_err_t readAir();
    };
}
