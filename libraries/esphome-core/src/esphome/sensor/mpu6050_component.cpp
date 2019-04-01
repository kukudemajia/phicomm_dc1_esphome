// Based on:
//   - https://www.invensense.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
//   - https://github.com/jarzebski/Arduino-MPU6050
//   - https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050

#include "esphome/defines.h"

#ifdef USE_MPU6050

#include "esphome/sensor/mpu6050_component.h"
#include "esphome/log.h"

ESPHOME_NAMESPACE_BEGIN

namespace sensor {

static const char *TAG = "sensor.mpu6050";

const uint8_t MPU6050_REGISTER_WHO_AM_I = 0x75;
const uint8_t MPU6050_REGISTER_POWER_MANAGEMENT_1 = 0x6B;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG = 0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG = 0x1C;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H = 0x3B;
const uint8_t MPU6050_CLOCK_SOURCE_X_GYRO = 0b001;
const uint8_t MPU6050_SCALE_2000_DPS = 0b11;
const float MPU6050_SCALE_DPS_PER_DIGIT_2000 = 0.060975f;
const uint8_t MPU6050_RANGE_2G = 0b00;
const float MPU6050_RANGE_PER_DIGIT_2G = 0.000061f;
const uint8_t MPU6050_BIT_SLEEP_ENABLED = 6;
const uint8_t MPU6050_BIT_TEMPERATURE_DISABLED = 3;
const float GRAVITY_EARTH = 9.80665f;

MPU6050Component::MPU6050Component(I2CComponent *parent, uint8_t address, uint32_t update_interval)
    : PollingComponent(update_interval), I2CDevice(parent, address) {}

void MPU6050Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MPU6050...");
  uint8_t who_am_i;
  if (!this->read_byte(MPU6050_REGISTER_WHO_AM_I, &who_am_i) || who_am_i != 0x68) {
    this->mark_failed();
    return;
  }

  ESP_LOGV(TAG, "  Setting up Power Management...");
  // Setup power management
  uint8_t power_management;
  if (!this->read_byte(MPU6050_REGISTER_POWER_MANAGEMENT_1, &power_management)) {
    this->mark_failed();
    return;
  }
  ESP_LOGV(TAG, "  Input power_management: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(power_management));
  // Set clock source - X-Gyro
  power_management &= 0b11111000;
  power_management |= MPU6050_CLOCK_SOURCE_X_GYRO;
  // Disable sleep
  power_management &= ~(1 << MPU6050_BIT_SLEEP_ENABLED);
  // Enable temperature
  power_management &= ~(1 << MPU6050_BIT_TEMPERATURE_DISABLED);
  ESP_LOGV(TAG, "  Output power_management: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(power_management));
  if (!this->write_byte(MPU6050_REGISTER_POWER_MANAGEMENT_1, power_management)) {
    this->mark_failed();
    return;
  }

  ESP_LOGV(TAG, "  Setting up Gyro Config...");
  // Set scale - 2000DPS
  uint8_t gyro_config;
  if (!this->read_byte(MPU6050_REGISTER_GYRO_CONFIG, &gyro_config)) {
    this->mark_failed();
    return;
  }
  ESP_LOGV(TAG, "  Input gyro_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(gyro_config));
  gyro_config &= 0b11100111;
  gyro_config |= MPU6050_SCALE_2000_DPS << 3;
  ESP_LOGV(TAG, "  Output gyro_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(gyro_config));
  if (!this->write_byte(MPU6050_REGISTER_GYRO_CONFIG, gyro_config)) {
    this->mark_failed();
    return;
  }

  ESP_LOGV(TAG, "  Setting up Accel Config...");
  // Set range - 2G
  uint8_t accel_config;
  if (!this->read_byte(MPU6050_REGISTER_ACCEL_CONFIG, &accel_config)) {
    this->mark_failed();
    return;
  }
  ESP_LOGV(TAG, "    Input accel_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(accel_config));
  accel_config &= 0b11100111;
  accel_config |= (MPU6050_RANGE_2G << 3);
  ESP_LOGV(TAG, "    Output accel_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(accel_config));
  if (!this->write_byte(MPU6050_REGISTER_GYRO_CONFIG, gyro_config)) {
    this->mark_failed();
    return;
  }
}
void MPU6050Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MPU6050:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with MPU6050 failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Acceleration X", this->accel_x_sensor_);
  LOG_SENSOR("  ", "Acceleration Y", this->accel_y_sensor_);
  LOG_SENSOR("  ", "Acceleration Z", this->accel_z_sensor_);
  LOG_SENSOR("  ", "Gyro X", this->gyro_x_sensor_);
  LOG_SENSOR("  ", "Gyro Y", this->gyro_y_sensor_);
  LOG_SENSOR("  ", "Gyro Z", this->gyro_z_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
}

void MPU6050Component::update() {
  ESP_LOGV(TAG, "    Updating MPU6050...");
  uint16_t data[7];
  if (!this->read_bytes_16(MPU6050_REGISTER_ACCEL_XOUT_H, data, 7)) {
    this->status_set_warning();
    return;
  }

  float accel_x = data[0] * MPU6050_RANGE_PER_DIGIT_2G * GRAVITY_EARTH;
  float accel_y = data[1] * MPU6050_RANGE_PER_DIGIT_2G * GRAVITY_EARTH;
  float accel_z = data[2] * MPU6050_RANGE_PER_DIGIT_2G * GRAVITY_EARTH;

  float temperature = data[3] / 340.0f + 36.53f;

  float gyro_x = data[4] * MPU6050_SCALE_DPS_PER_DIGIT_2000;
  float gyro_y = data[5] * MPU6050_SCALE_DPS_PER_DIGIT_2000;
  float gyro_z = data[6] * MPU6050_SCALE_DPS_PER_DIGIT_2000;

  ESP_LOGD(TAG,
           "Got accel={x=%.3f m/s², y=%.3f m/s², z=%.3f m/s²}, "
           "gyro={x=%.3f °/s, y=%.3f °/s, z=%.3f °/s}, temp=%.3f°C",
           accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, temperature);

  if (this->accel_x_sensor_ != nullptr)
    this->accel_x_sensor_->publish_state(accel_x);
  if (this->accel_y_sensor_ != nullptr)
    this->accel_y_sensor_->publish_state(accel_y);
  if (this->accel_z_sensor_ != nullptr)
    this->accel_z_sensor_->publish_state(accel_z);

  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(temperature);

  if (this->gyro_x_sensor_ != nullptr)
    this->gyro_x_sensor_->publish_state(gyro_x);
  if (this->gyro_y_sensor_ != nullptr)
    this->gyro_y_sensor_->publish_state(gyro_y);
  if (this->gyro_z_sensor_ != nullptr)
    this->gyro_z_sensor_->publish_state(gyro_z);

  this->status_clear_warning();
}
MPU6050AccelSensor *MPU6050Component::make_accel_x_sensor(const std::string &name) {
  return this->accel_x_sensor_ = new MPU6050AccelSensor(name, this);
}
MPU6050AccelSensor *MPU6050Component::make_accel_y_sensor(const std::string &name) {
  return this->accel_y_sensor_ = new MPU6050AccelSensor(name, this);
}
MPU6050AccelSensor *MPU6050Component::make_accel_z_sensor(const std::string &name) {
  return this->accel_z_sensor_ = new MPU6050AccelSensor(name, this);
}
MPU6050GyroSensor *MPU6050Component::make_gyro_x_sensor(const std::string &name) {
  return this->gyro_x_sensor_ = new MPU6050GyroSensor(name, this);
}
MPU6050GyroSensor *MPU6050Component::make_gyro_y_sensor(const std::string &name) {
  return this->gyro_y_sensor_ = new MPU6050GyroSensor(name, this);
}
MPU6050GyroSensor *MPU6050Component::make_gyro_z_sensor(const std::string &name) {
  return this->gyro_z_sensor_ = new MPU6050GyroSensor(name, this);
}
MPU6050TemperatureSensor *MPU6050Component::make_temperature_sensor(const std::string &name) {
  return this->temperature_sensor_ = new MPU6050TemperatureSensor(name, this);
}
float MPU6050Component::get_setup_priority() const { return setup_priority::HARDWARE_LATE; }

}  // namespace sensor

ESPHOME_NAMESPACE_END

#endif  // USE_MPUT6050
