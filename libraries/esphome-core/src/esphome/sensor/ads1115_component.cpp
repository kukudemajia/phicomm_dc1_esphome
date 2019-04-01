#include "esphome/defines.h"

#ifdef USE_ADS1115_SENSOR

#include "esphome/log.h"
#include "esphome/sensor/ads1115_component.h"

ESPHOME_NAMESPACE_BEGIN

namespace sensor {

static const char *TAG = "sensor.ads1115";
static const uint8_t ADS1115_REGISTER_CONVERSION = 0x00;
static const uint8_t ADS1115_REGISTER_CONFIG = 0x01;

static const uint8_t ADS1115_DATA_RATE_860_SPS = 0b111;

void ADS1115Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ADS1115...");
  uint16_t value;
  if (!this->read_byte_16(ADS1115_REGISTER_CONVERSION, &value)) {
    this->mark_failed();
    return;
  }
  uint16_t config = 0;
  // Clear single-shot bit
  //        0b0xxxxxxxxxxxxxxx
  config |= 0b0000000000000000;
  // Setup multiplexer
  //        0bx000xxxxxxxxxxxx
  config |= ADS1115_MULTIPLEXER_P0_N1 << 12;

  // Setup Gain
  //        0bxxxx000xxxxxxxxx
  config |= ADS1115_GAIN_6P144 << 9;

  // Set singleshot mode
  //        0bxxxxxxx1xxxxxxxx
  config |= 0b0000000100000000;

  // Set data rate - 860 samples per second (we're in singleshot mode)
  //        0bxxxxxxxx100xxxxx
  config |= ADS1115_DATA_RATE_860_SPS << 5;

  // Set comparator mode - hysteresis
  //        0bxxxxxxxxxxx0xxxx
  config |= 0b0000000000000000;

  // Set comparator polarity - active low
  //        0bxxxxxxxxxxxx0xxx
  config |= 0b0000000000000000;

  // Set comparator latch enabled - false
  //        0bxxxxxxxxxxxxx0xx
  config |= 0b0000000000000000;

  // Set comparator que mode - disabled
  //        0bxxxxxxxxxxxxxx11
  config |= 0b0000000000000011;

  if (!this->write_byte_16(ADS1115_REGISTER_CONFIG, config)) {
    this->mark_failed();
    return;
  }
  for (auto *sensor : this->sensors_) {
    this->set_interval(sensor->get_name(), sensor->update_interval(),
                       [this, sensor] { this->request_measurement_(sensor); });
  }
}
void ADS1115Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Setting up ADS1115...");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with ADS1115 failed!");
  }

  for (auto *sensor : this->sensors_) {
    LOG_SENSOR("  ", "Sensor", sensor);
    ESP_LOGCONFIG(TAG, "    Multiplexer: %u", sensor->get_multiplexer());
    ESP_LOGCONFIG(TAG, "    Gain: %u", sensor->get_gain());
  }
}
float ADS1115Component::get_setup_priority() const { return setup_priority::HARDWARE_LATE; }
void ADS1115Component::request_measurement_(ADS1115Sensor *sensor) {
  uint16_t config;
  if (!this->read_byte_16(ADS1115_REGISTER_CONFIG, &config)) {
    this->status_set_warning();
    return;
  }
  // Multiplexer
  //        0bxBBBxxxxxxxxxxxx
  config &= 0b1000111111111111;
  config |= (sensor->get_multiplexer() & 0b111) << 12;

  // Gain
  //        0bxxxxBBBxxxxxxxxx
  config &= 0b1111000111111111;
  config |= (sensor->get_gain() & 0b111) << 9;
  // Start conversion
  config |= 0b1000000000000000;

  if (!this->write_byte_16(ADS1115_REGISTER_CONFIG, config)) {
    this->status_set_warning();
    return;
  }

  // about 1.6 ms with 860 samples per second
  delay(2);

  uint32_t start = millis();
  while (this->read_byte_16(ADS1115_REGISTER_CONFIG, &config) && (config >> 15) == 0) {
    if (millis() - start > 100) {
      ESP_LOGW(TAG, "Reading ADS1115 timed out");
      this->status_set_warning();
      return;
    }
    yield();
  }

  uint16_t raw_conversion;
  if (!this->read_byte_16(ADS1115_REGISTER_CONVERSION, &raw_conversion)) {
    this->status_set_warning();
    return;
  }
  auto signed_conversion = static_cast<int16_t>(raw_conversion);

  float millivolts;
  switch (sensor->get_gain()) {
    case ADS1115_GAIN_6P144:
      millivolts = signed_conversion * 0.187500f;
      break;
    case ADS1115_GAIN_4P096:
      millivolts = signed_conversion * 0.125000f;
      break;
    case ADS1115_GAIN_2P048:
      millivolts = signed_conversion * 0.062500f;
      break;
    case ADS1115_GAIN_1P024:
      millivolts = signed_conversion * 0.031250f;
      break;
    case ADS1115_GAIN_0P512:
      millivolts = signed_conversion * 0.015625f;
      break;
    case ADS1115_GAIN_0P256:
      millivolts = signed_conversion * 0.007813f;
      break;
    default:
      millivolts = NAN;
  }

  float v = millivolts / 1000.0f;
  ESP_LOGD(TAG, "'%s': Got Voltage=%fV", sensor->get_name().c_str(), v);
  sensor->publish_state(v);
  this->status_clear_warning();
}

ADS1115Sensor *ADS1115Component::get_sensor(const std::string &name, ADS1115Multiplexer multiplexer, ADS1115Gain gain,
                                            uint32_t update_interval) {
  auto s = new ADS1115Sensor(name, multiplexer, gain, update_interval);
  this->sensors_.push_back(s);
  return s;
}
ADS1115Component::ADS1115Component(I2CComponent *parent, uint8_t address) : I2CDevice(parent, address) {}

uint8_t ADS1115Sensor::get_multiplexer() const { return this->multiplexer_; }
void ADS1115Sensor::set_multiplexer(ADS1115Multiplexer multiplexer) { this->multiplexer_ = multiplexer; }
uint8_t ADS1115Sensor::get_gain() const { return this->gain_; }
void ADS1115Sensor::set_gain(ADS1115Gain gain) { this->gain_ = gain; }
ADS1115Sensor::ADS1115Sensor(const std::string &name, ADS1115Multiplexer multiplexer, ADS1115Gain gain,
                             uint32_t update_interval)
    : EmptySensor(name), multiplexer_(multiplexer), gain_(gain), update_interval_(update_interval) {}
uint32_t ADS1115Sensor::update_interval() { return this->update_interval_; }

}  // namespace sensor

ESPHOME_NAMESPACE_END

#endif  // USE_ADS1115_SENSOR
