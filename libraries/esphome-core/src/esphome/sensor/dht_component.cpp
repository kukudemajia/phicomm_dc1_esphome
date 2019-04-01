// Based on:
//   - https://github.com/markruys/arduino-DHT

#include "esphome/defines.h"

#ifdef USE_DHT_SENSOR

#include "esphome/sensor/dht_component.h"

#include "esphome/log.h"
#include "esphome/espmath.h"
#include "esphome/helpers.h"
#include "esphome/espmath.h"

ESPHOME_NAMESPACE_BEGIN

namespace sensor {

static const char *TAG = "sensor.dht";

DHTComponent::DHTComponent(const std::string &temperature_name, const std::string &humidity_name, GPIOPin *pin,
                           uint32_t update_interval)
    : PollingComponent(update_interval),
      pin_(pin),
      temperature_sensor_(new DHTTemperatureSensor(temperature_name, this)),
      humidity_sensor_(new DHTHumiditySensor(humidity_name, this)) {}

void DHTComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DHT...");
  this->pin_->digital_write(true);
  this->pin_->setup();
  this->pin_->digital_write(true);
}
void DHTComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DHT:");
  LOG_PIN("  Pin: ", this->pin_);
  if (this->is_auto_detect_) {
    ESP_LOGCONFIG(TAG, "  Auto-detected model: %s", this->model_ == DHT_MODEL_DHT11 ? "DHT11" : "DHT22");
  } else if (this->model_ == DHT_MODEL_DHT11) {
    ESP_LOGCONFIG(TAG, "  Model: DHT11");
  } else {
    ESP_LOGCONFIG(TAG, "  Model: DHT22 (or equivalent)");
  }

  LOG_UPDATE_INTERVAL(this);

  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
}

void DHTComponent::update() {
  float temperature, humidity;
  bool error;
  if (this->model_ == DHT_MODEL_AUTO_DETECT) {
    this->model_ = DHT_MODEL_DHT22;
    error = this->read_sensor_(&temperature, &humidity, false);
    if (error) {
      this->model_ = DHT_MODEL_DHT11;
      return;
    }
  } else {
    error = this->read_sensor_(&temperature, &humidity, true);
  }

  if (error) {
    ESP_LOGD(TAG, "Got Temperature=%.1f°C Humidity=%.1f%%", temperature, humidity);

    this->temperature_sensor_->publish_state(temperature);
    this->humidity_sensor_->publish_state(humidity);
    this->status_clear_warning();
  } else {
    const char *str = "";
    if (this->is_auto_detect_) {
      str = " and consider manually specifying the DHT model using the model option";
    }
    ESP_LOGW(TAG, "Invalid readings! Please check your wiring (pull-up resistor, pin number)%s.", str);
    this->status_set_warning();
  }
}

float DHTComponent::get_setup_priority() const { return setup_priority::HARDWARE_LATE; }
void DHTComponent::set_dht_model(DHTModel model) {
  this->model_ = model;
  this->is_auto_detect_ = model == DHT_MODEL_AUTO_DETECT;
}
DHTTemperatureSensor *DHTComponent::get_temperature_sensor() const { return this->temperature_sensor_; }
DHTHumiditySensor *DHTComponent::get_humidity_sensor() const { return this->humidity_sensor_; }
bool HOT DHTComponent::read_sensor_(float *temperature, float *humidity, bool report_errors) {
  *humidity = NAN;
  *temperature = NAN;

  disable_interrupts();
  this->pin_->digital_write(false);
  this->pin_->pin_mode(OUTPUT);
  this->pin_->digital_write(false);

  if (this->model_ == DHT_MODEL_DHT11) {
    delayMicroseconds(18000);
  } else if (this->model_ == DHT_MODEL_SI7021) {
    delayMicroseconds(500);
    this->pin_->digital_write(true);
    delayMicroseconds(40);
  } else {
    delayMicroseconds(800);
  }
  this->pin_->pin_mode(INPUT_PULLUP);
  delayMicroseconds(40);

  uint8_t data[5] = {0, 0, 0, 0, 0};
  uint8_t bit = 7;
  uint8_t byte = 0;

  for (int8_t i = -1; i < 40; i++) {
    uint32_t start_time = micros();

    // Wait for rising edge
    while (!this->pin_->digital_read()) {
      if (micros() - start_time > 90) {
        enable_interrupts();
        if (report_errors) {
          if (i < 0) {
            ESP_LOGW(TAG, "Waiting for DHT communication to clear failed!");
          } else {
            ESP_LOGW(TAG, "Rising edge for bit %d failed!", i);
          }
        }
        return false;
      }
    }

    start_time = micros();
    uint32_t end_time = start_time;

    // Wait for falling edge
    while (this->pin_->digital_read()) {
      if ((end_time = micros()) - start_time > 90) {
        enable_interrupts();
        if (report_errors) {
          if (i < 0) {
            ESP_LOGW(TAG, "Requesting data from DHT failed!");
          } else {
            ESP_LOGW(TAG, "Falling edge for bit %d failed!", i);
          }
        }
        return false;
      }
    }

    if (i < 0)
      continue;

    if (end_time - start_time >= 40) {
      data[byte] |= 1 << bit;
    }
    if (bit == 0) {
      bit = 7;
      byte++;
    } else
      bit--;
  }
  enable_interrupts();

  ESP_LOGVV(TAG,
            "Data: Hum=0b" BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN
            ", Temp=0b" BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN ", Checksum=0b" BYTE_TO_BINARY_PATTERN,
            BYTE_TO_BINARY(data[0]), BYTE_TO_BINARY(data[1]), BYTE_TO_BINARY(data[2]), BYTE_TO_BINARY(data[3]),
            BYTE_TO_BINARY(data[4]));

  uint8_t checksum_a = data[0] + data[1] + data[2] + data[3];
  // On the DHT11, two algorithms for the checksum seem to be used, either the one from the DHT22,
  // or just using bytes 0 and 2
  uint8_t checksum_b = this->model_ == DHT_MODEL_DHT11 ? (data[0] + data[2]) : checksum_a;

  if (checksum_a != data[4] && checksum_b != data[4]) {
    if (report_errors) {
      ESP_LOGE(TAG, "Checksum invalid: %u!=%u", checksum_a, data[4]);
    }
    return false;
  }

  if (this->model_ == DHT_MODEL_DHT11) {
    *humidity = data[0];
    *temperature = data[2];
  } else {
    uint16_t raw_humidity = (uint16_t(data[0] & 0xFF) << 8) | (data[1] & 0xFF);
    uint16_t raw_temperature = (uint16_t(data[2] & 0xFF) << 8) | (data[3] & 0xFF);
    *humidity = raw_humidity * 0.1f;

    if ((raw_temperature & 0x8000) != 0)
      raw_temperature = ~(raw_temperature & 0x7FFF);

    *temperature = int16_t(raw_temperature) * 0.1f;
  }

  if (*temperature == 0.0f && (*humidity == 1.0f || *humidity == 2.0f)) {
    if (report_errors) {
      ESP_LOGE(TAG, "DHT reports invalid data. Is the update interval too high or the sensor damaged?");
    }
    return false;
  }

  return true;
}

}  // namespace sensor

ESPHOME_NAMESPACE_END

#endif  // USE_DHT_SENSOR
