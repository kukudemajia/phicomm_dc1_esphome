#ifndef ESPHOME_UART_COMPONENT_H
#define ESPHOME_UART_COMPONENT_H

#include "esphome/defines.h"

#ifdef USE_UART

#include <HardwareSerial.h>
#include "esphome/component.h"

ESPHOME_NAMESPACE_BEGIN

#ifdef ARDUINO_ARCH_ESP8266
class ESP8266SoftwareSerial {
 public:
  void setup(int8_t tx_pin, int8_t rx_pin, uint32_t baud_rate);

  uint8_t read_byte();
  uint8_t peek_byte();

  void flush();

  void write_byte(uint8_t data);

  int available();

 protected:
  static void gpio_intr(ESP8266SoftwareSerial *arg);

  inline void wait_(uint32_t *wait, const uint32_t &start);
  inline bool read_bit_(uint32_t *wait, const uint32_t &start);
  inline void write_bit_(bool bit, uint32_t *wait, const uint32_t &start);

  uint32_t bit_time_{0};
  uint8_t *rx_buffer_{nullptr};
  size_t rx_buffer_size_{64};
  volatile size_t rx_in_pos_{0};
  size_t rx_out_pos_{0};
  GPIOPin *tx_pin_{nullptr};
  ISRInternalGPIOPin *rx_pin_{nullptr};
};
#endif

class UARTComponent : public Component, public Stream {
 public:
  UARTComponent(uint32_t baud_rate = 9600);

  void setup() override;

  void dump_config() override;

  void write_byte(uint8_t data);

  void write_array(const uint8_t *data, size_t len);

  void write_str(const char *str);

  bool peek_byte(uint8_t *data);

  bool read_byte(uint8_t *data);

  bool read_array(uint8_t *data, size_t len);

  int available() override;

  void flush() override;

  float get_setup_priority() const override;

  size_t write(uint8_t data) override;
  int read() override;
  int peek() override;

  void set_tx_pin(uint8_t tx_pin);
  void set_rx_pin(uint8_t rx_pin);

 protected:
  bool check_read_timeout_(size_t len = 1);

  HardwareSerial *hw_serial_{nullptr};
#ifdef ARDUINO_ARCH_ESP8266
  ESP8266SoftwareSerial *sw_serial_{nullptr};
#endif
  optional<uint8_t> tx_pin_;
  optional<uint8_t> rx_pin_;
  uint32_t baud_rate_;
};

#ifdef ARDUINO_ARCH_ESP32
extern uint8_t next_uart_num;
#endif

class UARTDevice : public Stream {
 public:
  UARTDevice(UARTComponent *parent);

  void write_byte(uint8_t data);

  void write_array(const uint8_t *data, size_t len);

  void write_str(const char *str);

  bool read_byte(uint8_t *data);
  bool peek_byte(uint8_t *data);

  bool read_array(uint8_t *data, size_t len);

  int available() override;

  void flush() override;

  size_t write(uint8_t data) override;
  int read() override;
  int peek() override;

 protected:
  UARTComponent *parent_;
};

ESPHOME_NAMESPACE_END

#endif  // USE_UART

#endif  // ESPHOME_UART_COMPONENT_H
