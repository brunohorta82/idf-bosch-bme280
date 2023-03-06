#pragma once
#include "BME280Sensor.hpp"
#include "../driver/bme280.h"
#include "I2C.hpp"
#include <memory>
using namespace std;
namespace Environment
{
    esp_err_t bme280Init(BME280Sensor *environmentSensor);
    esp_err_t bme280Read();

}
