#ifndef ESPHOME_SENSOR_ULTRASONIC_SENSOR_H
#define ESPHOME_SENSOR_ULTRASONIC_SENSOR_H

#include "esphome/defines.h"

#ifdef USE_ULTRASONIC_SENSOR

#include "esphome/sensor/sensor.h"
#include "esphome/esphal.h"

ESPHOME_NAMESPACE_BEGIN

namespace sensor {

class UltrasonicSensorComponent : public PollingSensorComponent {
 public:
  /** Construct the ultrasonic sensor with the specified trigger pin and echo pin.
   *
   * @param trigger_pin The trigger pin where pulses are sent to.
   * @param echo_pin The echo pin where the echo is listened for.
   * @param update_interval The interval in ms the sensor should check for new values.
   */
  UltrasonicSensorComponent(const std::string &name, GPIOPin *trigger_pin, GPIOPin *echo_pin,
                            uint32_t update_interval = 60000);

  /// Set the timeout for waiting for the echo in µs.
  void set_timeout_us(uint32_t timeout_us);

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  /// Set up pins and register interval.
  void setup() override;
  void dump_config() override;

  void update() override;

  std::string unit_of_measurement() override;
  std::string icon() override;
  int8_t accuracy_decimals() override;

  float get_setup_priority() const override;

  /// Set the time in µs the trigger pin should be enabled for in µs, defaults to 10µs (for HC-SR04)
  void set_pulse_time_us(uint32_t pulse_time_us);

 protected:
  /// Helper function to convert the specified echo duration in µs to meters.
  static float us_to_m(uint32_t us);
  /// Helper function to convert the specified distance in meters to the echo duration in µs.

  GPIOPin *trigger_pin_;
  GPIOPin *echo_pin_;
  uint32_t timeout_us_{11662};  /// 2 meters.
  uint32_t pulse_time_us_{10};
};

}  // namespace sensor

ESPHOME_NAMESPACE_END

#endif  // USE_ULTRASONIC_SENSOR

#endif  // ESPHOME_SENSOR_ULTRASONIC_SENSOR_H
