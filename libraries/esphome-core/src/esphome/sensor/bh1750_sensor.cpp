// Based on:
//  - http://www.mouser.com/ds/2/348/bh1750fvi-e-186247.pdf
//  - https://github.com/claws/BH1750
//  - https://github.com/mysensors/MySensorsArduinoExamples/tree/master/libraries/BH1750

#include "esphome/defines.h"

#ifdef USE_BH1750

#include "esphome/sensor/bh1750_sensor.h"
#include "esphome/log.h"

ESPHOME_NAMESPACE_BEGIN

namespace sensor {

static const char *TAG = "sensor.bh1750";

static const uint8_t BH1750_COMMAND_POWER_ON = 0b00000001;

BH1750Sensor::BH1750Sensor(I2CComponent *parent, const std::string &name, uint8_t address, uint32_t update_interval)
    : PollingSensorComponent(name, update_interval), I2CDevice(parent, address) {}

void BH1750Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up BH1750 '%s'...", this->name_.c_str());
  if (!this->write_bytes(BH1750_COMMAND_POWER_ON, nullptr, 0)) {
    this->mark_failed();
    return;
  }
}
void BH1750Sensor::dump_config() {
  LOG_SENSOR("", "BH1750", this);
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with BH1750 failed!");
  }

  const char *resolution_s;
  switch (this->resolution_) {
    case BH1750_RESOLUTION_0P5_LX:
      resolution_s = "0.5";
      break;
    case BH1750_RESOLUTION_1P0_LX:
      resolution_s = "1";
      break;
    case BH1750_RESOLUTION_4P0_LX:
      resolution_s = "4";
      break;
    default:
      resolution_s = "Unknown";
      break;
  }
  ESP_LOGCONFIG(TAG, "  Resolution: %s", resolution_s);
  LOG_UPDATE_INTERVAL(this);
}

void BH1750Sensor::update() {
  if (!this->write_bytes(this->resolution_, nullptr, 0))
    return;

  uint32_t wait = 0;
  // use max conversion times
  switch (this->resolution_) {
    case BH1750_RESOLUTION_0P5_LX:
    case BH1750_RESOLUTION_1P0_LX:
      wait = 180;
      break;
    case BH1750_RESOLUTION_4P0_LX:
      wait = 24;
      break;
  }

  this->set_timeout("illuminance", wait, [this]() { this->read_data_(); });
}
std::string BH1750Sensor::unit_of_measurement() { return UNIT_LX; }
std::string BH1750Sensor::icon() { return ICON_BRIGHTNESS_5; }
int8_t BH1750Sensor::accuracy_decimals() { return 1; }
float BH1750Sensor::get_setup_priority() const { return setup_priority::HARDWARE_LATE; }
void BH1750Sensor::read_data_() {
  uint16_t raw_value;
  if (!this->parent_->raw_receive_16(this->address_, &raw_value, 1)) {
    this->status_set_warning();
    return;
  }

  float lx = float(raw_value) / 1.2f;
  ESP_LOGD(TAG, "'%s': Got illuminance=%.1flx", this->get_name().c_str(), lx);
  this->publish_state(lx);
  this->status_clear_warning();
}
void BH1750Sensor::set_resolution(BH1750Resolution resolution) { this->resolution_ = resolution; }

}  // namespace sensor

ESPHOME_NAMESPACE_END

#endif  // USE_BH1750
