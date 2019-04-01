#include "esphome/defines.h"

#ifdef USE_HX711

#include "esphome/sensor/hx711.h"
#include "esphome/log.h"

ESPHOME_NAMESPACE_BEGIN

namespace sensor {

static const char *TAG = "sensor.hx711";

void HX711Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HX711 '%s'...", this->name_.c_str());
  this->sck_pin_->setup();
  this->dout_pin_->setup();

  // Read sensor once without publishing to set the gain
  this->read_sensor_(nullptr);
}

void HX711Sensor::dump_config() {
  LOG_SENSOR("", "HX711", this);
  LOG_PIN("  DOUT Pin: ", this->dout_pin_);
  LOG_PIN("  SCK Pin: ", this->sck_pin_);
  LOG_UPDATE_INTERVAL(this);
}
float HX711Sensor::get_setup_priority() const { return setup_priority::HARDWARE_LATE; }
void HX711Sensor::update() {
  uint32_t result;
  if (this->read_sensor_(&result)) {
    ESP_LOGD(TAG, "'%s': Got value %u", this->name_.c_str(), result);
    this->publish_state(result);
  }
}
bool HX711Sensor::read_sensor_(uint32_t *result) {
  if (this->dout_pin_->digital_read()) {
    ESP_LOGW(TAG, "HX711 is not ready for new measurements yet!");
    this->status_set_warning();
    return false;
  }

  this->status_clear_warning();
  uint32_t data = 0;

  for (uint8_t i = 0; i < 24; i++) {
    this->sck_pin_->digital_write(true);
    delayMicroseconds(1);
    data |= uint32_t(this->dout_pin_->digital_read()) << (24 - i);
    this->sck_pin_->digital_write(false);
    delayMicroseconds(1);
  }

  // Cycle clock pin for gain setting
  for (uint8_t i = 0; i < this->gain_; i++) {
    this->sck_pin_->digital_write(true);
    this->sck_pin_->digital_write(false);
  }

  data ^= 0x800000;
  if (result != nullptr)
    *result = data;
  return true;
}
std::string HX711Sensor::unit_of_measurement() {
  // datasheet gives no unit
  return "";
}
std::string HX711Sensor::icon() { return "mdi:scale"; }
int8_t HX711Sensor::accuracy_decimals() { return 0; }
HX711Sensor::HX711Sensor(const std::string &name, GPIOPin *dout, GPIOPin *sck, uint32_t update_interval)
    : PollingSensorComponent(name, update_interval), dout_pin_(dout), sck_pin_(sck) {}

void HX711Sensor::set_gain(HX711Gain gain) { this->gain_ = gain; }

}  // namespace sensor

ESPHOME_NAMESPACE_END

#endif  // USE_HX711
