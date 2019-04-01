#include "esphome/defines.h"

#ifdef USE_UART

#include "esphome/uart_component.h"
#include "esphome/helpers.h"
#include "esphome/log.h"

ESPHOME_NAMESPACE_BEGIN

static const char *TAG = "uart";

#ifdef ARDUINO_ARCH_ESP32
uint8_t next_uart_num = 1;
#endif

UARTComponent::UARTComponent(uint32_t baud_rate) : baud_rate_(baud_rate) {}

float UARTComponent::get_setup_priority() const { return setup_priority::PRE_HARDWARE; }

#ifdef ARDUINO_ARCH_ESP32
void UARTComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up UART...");
  // Use Arduino HardwareSerial UARTs if all used pins match the ones
  // preconfigured by the platform. For example if RX disabled but TX pin
  // is 1 we still want to use Serial.
  if (this->tx_pin_.value_or(1) == 1 && this->rx_pin_.value_or(3) == 3) {
    this->hw_serial_ = &Serial;
  } else if (this->tx_pin_.value_or(9) == 9 && this->rx_pin_.value_or(10) == 10) {
    this->hw_serial_ = &Serial1;
  } else if (this->tx_pin_.value_or(16) == 16 && this->rx_pin_.value_or(17) == 17) {
    this->hw_serial_ = &Serial2;
  } else {
    this->hw_serial_ = new HardwareSerial(next_uart_num++);
  }
  int8_t tx = this->tx_pin_.has_value() ? *this->tx_pin_ : -1;
  int8_t rx = this->rx_pin_.has_value() ? *this->rx_pin_ : -1;
  this->hw_serial_->begin(this->baud_rate_, SERIAL_8N1, rx, tx);
}

void UARTComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "UART Bus:");
  if (this->tx_pin_.has_value()) {
    ESP_LOGCONFIG(TAG, "  TX Pin: GPIO%d", *this->tx_pin_);
  }
  if (this->rx_pin_.has_value()) {
    ESP_LOGCONFIG(TAG, "  RX Pin: GPIO%d", *this->rx_pin_);
  }
  ESP_LOGCONFIG(TAG, "  Baud Rate: %u baud", this->baud_rate_);
}

void UARTComponent::write_byte(uint8_t data) {
  this->hw_serial_->write(data);
  ESP_LOGVV(TAG, "    Wrote 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data), data);
}
void UARTComponent::write_array(const uint8_t *data, size_t len) {
  this->hw_serial_->write(data, len);
  for (size_t i = 0; i < len; i++) {
    ESP_LOGVV(TAG, "    Wrote 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data[i]), data[i]);
  }
}
void UARTComponent::write_str(const char *str) {
  this->hw_serial_->write(str);
  ESP_LOGVV(TAG, "    Wrote \"%s\"", str);
}
bool UARTComponent::read_byte(uint8_t *data) {
  if (!this->check_read_timeout_())
    return false;
  *data = this->hw_serial_->read();
  ESP_LOGVV(TAG, "    Read 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(*data), *data);
  return true;
}
bool UARTComponent::peek_byte(uint8_t *data) {
  if (!this->check_read_timeout_())
    return false;
  *data = this->hw_serial_->peek();
  return true;
}
bool UARTComponent::read_array(uint8_t *data, size_t len) {
  if (!this->check_read_timeout_(len))
    return false;
  this->hw_serial_->readBytes(data, len);
  for (size_t i = 0; i < len; i++) {
    ESP_LOGVV(TAG, "    Read 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data[i]), data[i]);
  }

  return true;
}
bool UARTComponent::check_read_timeout_(size_t len) {
  if (this->available() >= len)
    return true;

  uint32_t start_time = millis();
  while (this->available() < len) {
    if (millis() - start_time > 1000) {
      ESP_LOGE(TAG, "Reading from UART timed out at byte %u!", this->available());
      return false;
    }
  }
  return true;
}
int UARTComponent::available() { return this->hw_serial_->available(); }
void UARTComponent::flush() {
  ESP_LOGVV(TAG, "    Flushing...");
  this->hw_serial_->flush();
}
#endif  // ESP32

#ifdef ARDUINO_ARCH_ESP8266
void UARTComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up UART bus...");
  // Use Arduino HardwareSerial UARTs if all used pins match the ones
  // preconfigured by the platform. For example if RX disabled but TX pin
  // is 1 we still want to use Serial.
  if (this->tx_pin_.value_or(1) == 1 && this->rx_pin_.value_or(3) == 3) {
    this->hw_serial_ = &Serial;
    this->hw_serial_->begin(this->baud_rate_);
  } else if (this->tx_pin_.value_or(15) == 15 && this->rx_pin_.value_or(13) == 13) {
    this->hw_serial_ = &Serial;
    this->hw_serial_->begin(this->baud_rate_);
    this->hw_serial_->swap();
  } else if (this->tx_pin_.value_or(2) == 2 && this->rx_pin_.value_or(8) == 8) {
    this->hw_serial_ = &Serial1;
    this->hw_serial_->begin(this->baud_rate_);
  } else {
    this->sw_serial_ = new ESP8266SoftwareSerial();
    int8_t tx = this->tx_pin_.has_value() ? *this->tx_pin_ : -1;
    int8_t rx = this->rx_pin_.has_value() ? *this->rx_pin_ : -1;
    this->sw_serial_->setup(tx, rx, this->baud_rate_);
  }
}

void UARTComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "UART Bus:");
  if (this->tx_pin_.has_value()) {
    ESP_LOGCONFIG(TAG, "  TX Pin: GPIO%d", *this->tx_pin_);
  }
  if (this->rx_pin_.has_value()) {
    ESP_LOGCONFIG(TAG, "  RX Pin: GPIO%d", *this->rx_pin_);
  }
  ESP_LOGCONFIG(TAG, "  Baud Rate: %u baud", this->baud_rate_);
  if (this->hw_serial_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Using hardware serial interface.");
  } else {
    ESP_LOGCONFIG(TAG, "  Using software serial");
  }
}

void UARTComponent::write_byte(uint8_t data) {
  if (this->hw_serial_ != nullptr) {
    this->hw_serial_->write(data);
  } else {
    this->sw_serial_->write_byte(data);
  }
  ESP_LOGVV(TAG, "    Wrote 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data), data);
}
void UARTComponent::write_array(const uint8_t *data, size_t len) {
  if (this->hw_serial_ != nullptr) {
    this->hw_serial_->write(data, len);
  } else {
    for (size_t i = 0; i < len; i++)
      this->sw_serial_->write_byte(data[i]);
  }
  for (size_t i = 0; i < len; i++) {
    ESP_LOGVV(TAG, "    Wrote 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data[i]), data[i]);
  }
}
void UARTComponent::write_str(const char *str) {
  if (this->hw_serial_ != nullptr) {
    this->hw_serial_->write(str);
  } else {
    const auto *data = reinterpret_cast<const uint8_t *>(str);
    for (size_t i = 0; data[i] != 0; i++)
      this->sw_serial_->write_byte(data[i]);
  }
  ESP_LOGVV(TAG, "    Wrote \"%s\"", str);
}
bool UARTComponent::read_byte(uint8_t *data) {
  if (!this->check_read_timeout_())
    return false;
  if (this->hw_serial_ != nullptr) {
    *data = this->hw_serial_->read();
  } else {
    *data = this->sw_serial_->read_byte();
  }
  ESP_LOGVV(TAG, "    Read 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(*data), *data);
  return true;
}
bool UARTComponent::peek_byte(uint8_t *data) {
  if (!this->check_read_timeout_())
    return false;
  if (this->hw_serial_ != nullptr) {
    *data = this->hw_serial_->peek();
  } else {
    *data = this->sw_serial_->peek_byte();
  }
  return true;
}
bool UARTComponent::read_array(uint8_t *data, size_t len) {
  if (!this->check_read_timeout_(len))
    return false;
  if (this->hw_serial_ != nullptr) {
    this->hw_serial_->readBytes(data, len);
  } else {
    for (size_t i = 0; i < len; i++)
      data[i] = this->sw_serial_->read_byte();
  }
  for (size_t i = 0; i < len; i++) {
    ESP_LOGVV(TAG, "    Read 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data[i]), data[i]);
  }

  return true;
}
bool UARTComponent::check_read_timeout_(size_t len) {
  if (this->available() >= len)
    return true;

  uint32_t start_time = millis();
  while (this->available() < len) {
    if (millis() - start_time > 100) {
      ESP_LOGE(TAG, "Reading from UART timed out at byte %u!", this->available());
      return false;
    }
    yield();
  }
  return true;
}
int UARTComponent::available() {
  if (this->hw_serial_ != nullptr) {
    return this->hw_serial_->available();
  } else {
    return this->sw_serial_->available();
  }
}
void UARTComponent::flush() {
  ESP_LOGVV(TAG, "    Flushing...");
  if (this->hw_serial_ != nullptr) {
    this->hw_serial_->flush();
  } else {
    this->sw_serial_->flush();
  }
}

void ESP8266SoftwareSerial::setup(int8_t tx_pin, int8_t rx_pin, uint32_t baud_rate) {
  this->bit_time_ = F_CPU / baud_rate;
  if (tx_pin != -1) {
    this->tx_pin_ = GPIOOutputPin(tx_pin).copy();
    this->tx_pin_->setup();
    this->tx_pin_->digital_write(true);
  }
  if (rx_pin != -1) {
    auto pin = GPIOInputPin(rx_pin);
    pin.setup();
    this->rx_pin_ = pin.to_isr();
    this->rx_buffer_ = new uint8_t[this->rx_buffer_size_];
    pin.attach_interrupt(ESP8266SoftwareSerial::gpio_intr, this, FALLING);
  }
}
void ICACHE_RAM_ATTR ESP8266SoftwareSerial::gpio_intr(ESP8266SoftwareSerial *arg) {
  uint32_t wait = arg->bit_time_ + arg->bit_time_ / 3 - 500;
  const uint32_t start = ESP.getCycleCount();
  uint8_t rec = 0;
  // Manually unroll the loop
  rec |= arg->read_bit_(&wait, start) << 0;
  rec |= arg->read_bit_(&wait, start) << 1;
  rec |= arg->read_bit_(&wait, start) << 2;
  rec |= arg->read_bit_(&wait, start) << 3;
  rec |= arg->read_bit_(&wait, start) << 4;
  rec |= arg->read_bit_(&wait, start) << 5;
  rec |= arg->read_bit_(&wait, start) << 6;
  rec |= arg->read_bit_(&wait, start) << 7;
  // Stop bit
  arg->wait_(&wait, start);

  arg->rx_buffer_[arg->rx_in_pos_] = rec;
  arg->rx_in_pos_ = (arg->rx_in_pos_ + 1) % arg->rx_buffer_size_;
  // Clear RX pin so that the interrupt doesn't re-trigger right away again.
  arg->rx_pin_->clear_interrupt();
}
void ICACHE_RAM_ATTR HOT ESP8266SoftwareSerial::write_byte(uint8_t data) {
  if (this->tx_pin_ == nullptr) {
    ESP_LOGE(TAG, "UART doesn't have TX pins set!");
    return;
  }

  disable_interrupts();
  uint32_t wait = this->bit_time_;
  const uint32_t start = ESP.getCycleCount();
  // Start bit
  this->write_bit_(false, &wait, start);
  this->write_bit_(data & (1 << 0), &wait, start);
  this->write_bit_(data & (1 << 1), &wait, start);
  this->write_bit_(data & (1 << 2), &wait, start);
  this->write_bit_(data & (1 << 3), &wait, start);
  this->write_bit_(data & (1 << 4), &wait, start);
  this->write_bit_(data & (1 << 5), &wait, start);
  this->write_bit_(data & (1 << 6), &wait, start);
  this->write_bit_(data & (1 << 7), &wait, start);
  // Stop bit
  this->write_bit_(true, &wait, start);
  enable_interrupts();
}
void ESP8266SoftwareSerial::wait_(uint32_t *wait, const uint32_t &start) {
  while (ESP.getCycleCount() - start < *wait)
    ;
  *wait += this->bit_time_;
}
bool ESP8266SoftwareSerial::read_bit_(uint32_t *wait, const uint32_t &start) {
  this->wait_(wait, start);
  return this->rx_pin_->digital_read();
}
void ESP8266SoftwareSerial::write_bit_(bool bit, uint32_t *wait, const uint32_t &start) {
  this->tx_pin_->digital_write(bit);
  this->wait_(wait, start);
}
uint8_t ESP8266SoftwareSerial::read_byte() {
  if (this->rx_in_pos_ == this->rx_out_pos_)
    return 0;
  uint8_t data = this->rx_buffer_[this->rx_out_pos_];
  this->rx_out_pos_ = (this->rx_out_pos_ + 1) % this->rx_buffer_size_;
  return data;
}
uint8_t ESP8266SoftwareSerial::peek_byte() {
  if (this->rx_in_pos_ == this->rx_out_pos_)
    return 0;
  return this->rx_buffer_[this->rx_out_pos_];
}
void ESP8266SoftwareSerial::flush() { this->rx_in_pos_ = this->rx_out_pos_ = 0; }
int ESP8266SoftwareSerial::available() {
  int avail = int(this->rx_in_pos_) - int(this->rx_out_pos_);
  if (avail < 0)
    return avail + this->rx_buffer_size_;
  return avail;
}
#endif  // ESP8266

size_t UARTComponent::write(uint8_t data) {
  this->write_byte(data);
  return 1;
}
int UARTComponent::read() {
  uint8_t data;
  if (!this->read_byte(&data))
    return -1;
  return data;
}
int UARTComponent::peek() {
  uint8_t data;
  if (!this->peek_byte(&data))
    return -1;
  return data;
}
void UARTComponent::set_tx_pin(uint8_t tx_pin) { this->tx_pin_ = tx_pin; }
void UARTComponent::set_rx_pin(uint8_t rx_pin) { this->rx_pin_ = rx_pin; }

void UARTDevice::write_byte(uint8_t data) { this->parent_->write_byte(data); }
void UARTDevice::write_array(const uint8_t *data, size_t len) { this->parent_->write_array(data, len); }
void UARTDevice::write_str(const char *str) { this->parent_->write_str(str); }
bool UARTDevice::read_byte(uint8_t *data) { return this->parent_->read_byte(data); }
bool UARTDevice::peek_byte(uint8_t *data) { return this->parent_->peek_byte(data); }
bool UARTDevice::read_array(uint8_t *data, size_t len) { return this->parent_->read_array(data, len); }
int UARTDevice::available() { return this->parent_->available(); }
void UARTDevice::flush() { return this->parent_->flush(); }
UARTDevice::UARTDevice(UARTComponent *parent) : parent_(parent) {}
size_t UARTDevice::write(uint8_t data) { return this->parent_->write(data); }
int UARTDevice::read() { return this->parent_->read(); }
int UARTDevice::peek() { return this->parent_->peek(); }

ESPHOME_NAMESPACE_END

#endif  // USE_UART
