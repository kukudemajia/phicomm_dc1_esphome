#include "esphome/defines.h"

#ifdef USE_ULTRASONIC_SENSOR

#include "esphome/sensor/ultrasonic_sensor.h"

#include "esphome/log.h"
#include "esphome/helpers.h"

ESPHOME_NAMESPACE_BEGIN

namespace sensor {

static const char *TAG = "sensor.ultrasonic";

UltrasonicSensorComponent::UltrasonicSensorComponent(const std::string &name, GPIOPin *trigger_pin, GPIOPin *echo_pin,
                                                     uint32_t update_interval)
    : PollingSensorComponent(name, update_interval), trigger_pin_(trigger_pin), echo_pin_(echo_pin) {}
void UltrasonicSensorComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Ultrasonic Sensor...");
  this->trigger_pin_->setup();
  this->trigger_pin_->digital_write(false);
  this->echo_pin_->setup();
}
void UltrasonicSensorComponent::update() {
  this->trigger_pin_->digital_write(true);
  delayMicroseconds(this->pulse_time_us_);
  this->trigger_pin_->digital_write(false);

  uint32_t time = pulseIn(this->echo_pin_->get_pin(), uint8_t(!this->echo_pin_->is_inverted()), this->timeout_us_);

  ESP_LOGV(TAG, "Echo took %uµs", time);

  if (time == 0) {
    ESP_LOGD(TAG, "'%s' - Distance measurement timed out!", this->name_.c_str());
    this->publish_state(NAN);
  } else {
    float result = UltrasonicSensorComponent::us_to_m(time);
    this->publish_state(result);
    ESP_LOGD(TAG, "'%s' - Got distance: %.2f m", this->name_.c_str(), result);
    this->publish_state(result);
  }
}
void UltrasonicSensorComponent::dump_config() {
  LOG_SENSOR("", "Ultrasonic Sensor", this);
  LOG_PIN("  Echo Pin: ", this->echo_pin_);
  LOG_PIN("  Trigger Pin: ", this->trigger_pin_);
  ESP_LOGCONFIG(TAG, "  Pulse time: %u µs", this->pulse_time_us_);
  ESP_LOGCONFIG(TAG, "  Timeout: %u µs", this->timeout_us_);
  LOG_UPDATE_INTERVAL(this);
}
float UltrasonicSensorComponent::us_to_m(uint32_t us) {
  const float speed_sound_m_per_s = 343.0f;
  const float time_s = us / 1e6f;
  const float total_dist = time_s * speed_sound_m_per_s;
  return total_dist / 2.0f;
}
float UltrasonicSensorComponent::get_setup_priority() const { return setup_priority::HARDWARE_LATE; }
void UltrasonicSensorComponent::set_pulse_time_us(uint32_t pulse_time_us) { this->pulse_time_us_ = pulse_time_us; }
std::string UltrasonicSensorComponent::unit_of_measurement() { return "m"; }
std::string UltrasonicSensorComponent::icon() { return "mdi:arrow-expand-vertical"; }
int8_t UltrasonicSensorComponent::accuracy_decimals() {
  return 2;  // cm precision
}
void UltrasonicSensorComponent::set_timeout_us(uint32_t timeout_us) { this->timeout_us_ = timeout_us; }

}  // namespace sensor

ESPHOME_NAMESPACE_END

#endif  // USE_ULTRASONIC_SENSOR
