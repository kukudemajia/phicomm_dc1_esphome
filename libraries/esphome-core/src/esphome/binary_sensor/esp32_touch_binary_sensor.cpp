#include "esphome/defines.h"

#ifdef USE_ESP32_TOUCH_BINARY_SENSOR

#include "esphome/binary_sensor/esp32_touch_binary_sensor.h"
#include "esphome/log.h"

ESPHOME_NAMESPACE_BEGIN

namespace binary_sensor {

static const char *TAG = "binary_sensor.esp32_touch";

void ESP32TouchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ESP32 Touch Hub...");
  touch_pad_init();

  if (this->iir_filter_enabled_()) {
    touch_pad_filter_start(this->iir_filter_);
  }

  touch_pad_set_meas_time(this->sleep_cycle_, this->meas_cycle_);
  touch_pad_set_voltage(this->high_voltage_reference_, this->low_voltage_reference_, this->voltage_attenuation_);

  for (auto *child : this->children_) {
    // Disable interrupt threshold
    touch_pad_config(child->get_touch_pad(), 0);
  }

  add_shutdown_hook([this](const char *cause) {
    if (this->iir_filter_enabled_()) {
      touch_pad_filter_stop();
      touch_pad_filter_delete();
    }
    touch_pad_deinit();
  });
}

void ESP32TouchComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Config for ESP32 Touch Hub:");
  ESP_LOGCONFIG(TAG, "  Meas cycle: %.2fms", this->meas_cycle_ / (8000000.0f / 1000.0f));
  ESP_LOGCONFIG(TAG, "  Sleep cycle: %.2fms", this->sleep_cycle_ / (150000.0f / 1000.0f));

  const char *lv_s;
  switch (this->low_voltage_reference_) {
    case TOUCH_LVOLT_0V5:
      lv_s = "0.5V";
      break;
    case TOUCH_LVOLT_0V6:
      lv_s = "0.6V";
      break;
    case TOUCH_LVOLT_0V7:
      lv_s = "0.7V";
      break;
    case TOUCH_LVOLT_0V8:
      lv_s = "0.8V";
      break;
    default:
      lv_s = "UNKNOWN";
      break;
  }
  ESP_LOGCONFIG(TAG, "  Low Voltage Reference: %s", lv_s);

  const char *hv_s;
  switch (this->high_voltage_reference_) {
    case TOUCH_HVOLT_2V4:
      hv_s = "2.4V";
      break;
    case TOUCH_HVOLT_2V5:
      hv_s = "2.5V";
      break;
    case TOUCH_HVOLT_2V6:
      hv_s = "2.6V";
      break;
    case TOUCH_HVOLT_2V7:
      hv_s = "2.7V";
      break;
    default:
      hv_s = "UNKNOWN";
      break;
  }
  ESP_LOGCONFIG(TAG, "  High Voltage Reference: %s", hv_s);

  const char *atten_s;
  switch (this->voltage_attenuation_) {
    case TOUCH_HVOLT_ATTEN_1V5:
      atten_s = "1.5V";
      break;
    case TOUCH_HVOLT_ATTEN_1V:
      atten_s = "1V";
      break;
    case TOUCH_HVOLT_ATTEN_0V5:
      atten_s = "0.5V";
      break;
    case TOUCH_HVOLT_ATTEN_0V:
      atten_s = "0V";
      break;
    default:
      atten_s = "UNKNOWN";
      break;
  }
  ESP_LOGCONFIG(TAG, "  Voltage Attenuation: %s", atten_s);

  if (this->iir_filter_enabled_()) {
    ESP_LOGCONFIG(TAG, "    IIR Filter: %ums", this->iir_filter_);
    touch_pad_filter_start(this->iir_filter_);
  } else {
    ESP_LOGCONFIG(TAG, "  IIR Filter DISABLED");
  }
  if (this->setup_mode_) {
    ESP_LOGCONFIG(TAG, "  Setup Mode ENABLED!");
  }

  for (auto *child : this->children_) {
    LOG_BINARY_SENSOR("  ", "Touch Pad", child);
    ESP_LOGCONFIG(TAG, "    Pad: T%d", child->get_touch_pad());
    ESP_LOGCONFIG(TAG, "    Threshold: %u", child->get_threshold());
  }
}

void ESP32TouchComponent::loop() {
  for (auto *child : this->children_) {
    uint16_t value;
    if (this->iir_filter_enabled_()) {
      touch_pad_read_filtered(child->get_touch_pad(), &value);
    } else {
      touch_pad_read(child->get_touch_pad(), &value);
    }

    child->publish_state(value < child->get_threshold());

    if (this->setup_mode_) {
      ESP_LOGD(TAG, "Touch Pad '%s' (T%u): %u", child->get_name().c_str(), child->get_touch_pad(), value);
    }
  }

  if (this->setup_mode_) {
    // Avoid spamming logs
    delay(250);
  }
}
ESP32TouchBinarySensor *ESP32TouchComponent::make_touch_pad(const std::string &name, touch_pad_t touch_pad,
                                                            uint16_t threshold) {
  auto *sensor = new ESP32TouchBinarySensor(name, touch_pad, threshold);
  this->children_.push_back(sensor);
  return sensor;
}
void ESP32TouchComponent::set_setup_mode(bool setup_mode) { this->setup_mode_ = setup_mode; }
bool ESP32TouchComponent::iir_filter_enabled_() const { return this->iir_filter_ > 0; }

void ESP32TouchComponent::set_iir_filter(uint32_t iir_filter) { this->iir_filter_ = iir_filter; }
float ESP32TouchComponent::get_setup_priority() const { return setup_priority::HARDWARE_LATE; }
void ESP32TouchComponent::set_sleep_duration(uint16_t sleep_duration) { this->sleep_cycle_ = sleep_duration; }
void ESP32TouchComponent::set_measurement_duration(uint16_t meas_cycle) { this->meas_cycle_ = meas_cycle; }
void ESP32TouchComponent::set_low_voltage_reference(touch_low_volt_t low_voltage_reference) {
  this->low_voltage_reference_ = low_voltage_reference;
}
void ESP32TouchComponent::set_high_voltage_reference(touch_high_volt_t high_voltage_reference) {
  this->high_voltage_reference_ = high_voltage_reference;
}
void ESP32TouchComponent::set_voltage_attenuation(touch_volt_atten_t voltage_attenuation) {
  this->voltage_attenuation_ = voltage_attenuation;
}

ESP32TouchBinarySensor::ESP32TouchBinarySensor(const std::string &name, touch_pad_t touch_pad, uint16_t threshold)
    : BinarySensor(name), touch_pad_(touch_pad), threshold_(threshold) {}
touch_pad_t ESP32TouchBinarySensor::get_touch_pad() const { return this->touch_pad_; }
uint16_t ESP32TouchBinarySensor::get_threshold() const { return this->threshold_; }

}  // namespace binary_sensor

ESPHOME_NAMESPACE_END

#endif  // USE_ESP32_TOUCH_BINARY_SENSOR
