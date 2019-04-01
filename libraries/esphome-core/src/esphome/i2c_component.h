#ifndef ESPHOME_I2C_COMPONENT_H
#define ESPHOME_I2C_COMPONENT_H

#include "esphome/defines.h"

#ifdef USE_I2C

#include "esphome/component.h"
#include <Wire.h>

ESPHOME_NAMESPACE_BEGIN

#define LOG_I2C_DEVICE(this) ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);

/** The I2CComponent is the base of ESPHome's i2c communication.
 *
 * It handles setting up the bus (with pins, clock frequency) and provides nice helper functions to
 * make reading from the i2c bus easier (see read_bytes, write_bytes) and safe (with read timeouts).
 *
 * For the user, it has a few setters (see set_sda_pin, set_scl_pin, set_frequency)
 * to setup some parameters for the bus. Additionally, the i2c component has a scan feature that will
 * scan the entire 7-bit i2c address range for devices that respond to transmissions to make finding
 * the address of an i2c device easier.
 *
 * On the ESP32, you can even have multiple I2C bus for communication, simply create multiple
 * I2CComponents, each with different SDA and SCL pins and use `set_parent` on all I2CDevices that use
 * the non-first I2C bus.
 */
class I2CComponent : public Component {
 public:
  I2CComponent(uint8_t sda_pin, uint8_t scl_pin, bool scan = false);

  /// Set the i2c SDA pin for this bus.
  void set_sda_pin(uint8_t sda_pin);
  /// Set the i2c SCL pin for this bus.
  void set_scl_pin(uint8_t scl_pin);
  /// Set the i2c clock frequency in Hz for this bus, defaults to 1000 Hz.
  void set_frequency(uint32_t frequency);
  /// Set if a scan of the entire i2c address range should be done on startup.
  void set_scan(bool scan);

  /** Read len amount of bytes from a register into data. Optionally with a conversion time after
   * writing the register value to the bus.
   *
   * @param address The address to send the request to.
   * @param a_register The register number to write to the bus before reading.
   * @param data An array to store len amount of 8-bit bytes into.
   * @param len The amount of bytes to request and write into data.
   * @param conversion The time in ms between writing the register value and reading out the value.
   * @return If the operation was successful.
   */
  bool read_bytes(uint8_t address, uint8_t a_register, uint8_t *data, uint8_t len, uint32_t conversion = 0);

  /** Read len amount of 16-bit words (MSB first) from a register into data.
   *
   * @param address The address to send the request to.
   * @param a_register The register number to write to the bus before reading.
   * @param data An array to store len amount of 16-bit words into.
   * @param len The amount of 16-bit words to request and write into data.
   * @param conversion The time in ms between writing the register value and reading out the value.
   * @return If the operation was successful.
   */
  bool read_bytes_16(uint8_t address, uint8_t a_register, uint16_t *data, uint8_t len, uint32_t conversion = 0);

  /// Read a single byte from a register into the data variable. Return true if successful.
  bool read_byte(uint8_t address, uint8_t a_register, uint8_t *data, uint32_t conversion = 0);

  /// Read a single 16-bit words (MSB first) from a register into the data variable. Return true if successful.
  bool read_byte_16(uint8_t address, uint8_t a_register, uint16_t *data, uint32_t conversion = 0);

  /** Write len amount of 8-bit bytes to the specified register for address.
   *
   * @param address The address to use for the transmission.
   * @param a_register The register to write the values to.
   * @param data An array from which len bytes of data will be written to the bus.
   * @param len The amount of bytes to write to the bus.
   * @return If the operation was successful.
   */
  bool write_bytes(uint8_t address, uint8_t a_register, const uint8_t *data, uint8_t len);

  /** Write len amount of 16-bit words (MSB first) to the specified register for address.
   *
   * @param address The address to use for the transmission.
   * @param a_register The register to write the values to.
   * @param data An array from which len 16-bit words of data will be written to the bus.
   * @param len The amount of bytes to write to the bus.
   * @return If the operation was successful.
   */
  bool write_bytes_16(uint8_t address, uint8_t a_register, const uint16_t *data, uint8_t len);

  /// Write a single byte of data into the specified register of address. Return true if successful.
  bool write_byte(uint8_t address, uint8_t a_register, uint8_t data);

  /// Write a single 16-bit word of data into the specified register of address. Return true if successful.
  bool write_byte_16(uint8_t address, uint8_t a_register, uint16_t data);

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  /// Begin a write transmission to an address.
  void raw_begin_transmission(uint8_t address);

  /// End a write transmission to an address, return true if successful.
  bool raw_end_transmission(uint8_t address);

  /** Request data from an address with a number of (8-bit) bytes.
   *
   * @param address The address to request the bytes from.
   * @param len The number of bytes to receive, must not be 0.
   * @return True if all requested bytes were read, false otherwise.
   */
  bool raw_request_from(uint8_t address, uint8_t len);

  /// Write len amount of bytes from data to address. begin_transmission_ must be called before this.
  void raw_write(uint8_t address, const uint8_t *data, uint8_t len);

  /// Write len amount of 16-bit words from data to address. begin_transmission_ must be called before this.
  void raw_write_16(uint8_t address, const uint16_t *data, uint8_t len);

  /// Request len amount of bytes from address and write the result it into data. Returns true iff was successful.
  bool raw_receive(uint8_t address, uint8_t *data, uint8_t len);

  /// Request len amount of 16-bit words from address and write the result into data. Returns true iff was successful.
  bool raw_receive_16(uint8_t address, uint16_t *data, uint8_t len);

  /// Setup the i2c. bus
  void setup() override;
  void dump_config() override;
  /// Set a very high setup priority to make sure it's loaded before all other hardware.
  float get_setup_priority() const override;

 protected:
  TwoWire *wire_;
  uint8_t sda_pin_;
  uint8_t scl_pin_;
  bool scan_;
  uint32_t frequency_{50000};
};

#ifdef ARDUINO_ARCH_ESP32
extern uint8_t next_i2c_bus_num_;
#endif

/** All components doing communication on the I2C bus should subclass I2CDevice.
 *
 * This class stores 1. the address of the i2c device and has a helper function to allow
 * users to manually set the address and 2. stores a reference to the "parent" I2CComponent.
 *
 *
 * All this class basically does is to expose all helper functions from I2CComponent.
 */
class I2CDevice {
 public:
  I2CDevice(I2CComponent *parent, uint8_t address);

  /// Manually set the i2c address of this device.
  void set_address(uint8_t address);

  /// Manually set the parent i2c bus for this device.
  void set_parent(I2CComponent *parent);

 protected:
  /** Read len amount of bytes from a register into data. Optionally with a conversion time after
   * writing the register value to the bus.
   *
   * @param a_register The register number to write to the bus before reading.
   * @param data An array to store len amount of 8-bit bytes into.
   * @param len The amount of bytes to request and write into data.
   * @param conversion The time in ms between writing the register value and reading out the value.
   * @return If the operation was successful.
   */
  bool read_bytes(uint8_t a_register, uint8_t *data, uint8_t len, uint32_t conversion = 0);  // NOLINT

  /** Read len amount of 16-bit words (MSB first) from a register into data.
   *
   * @param a_register The register number to write to the bus before reading.
   * @param data An array to store len amount of 16-bit words into.
   * @param len The amount of 16-bit words to request and write into data.
   * @param conversion The time in ms between writing the register value and reading out the value.
   * @return If the operation was successful.
   */
  bool read_bytes_16(uint8_t a_register, uint16_t *data, uint8_t len, uint32_t conversion = 0);  // NOLINT

  /// Read a single byte from a register into the data variable. Return true if successful.
  bool read_byte(uint8_t a_register, uint8_t *data, uint32_t conversion = 0);  // NOLINT

  /// Read a single 16-bit words (MSB first) from a register into the data variable. Return true if successful.
  bool read_byte_16(uint8_t a_register, uint16_t *data, uint32_t conversion = 0);  // NOLINT

  /** Write len amount of 8-bit bytes to the specified register.
   *
   * @param a_register The register to write the values to.
   * @param data An array from which len bytes of data will be written to the bus.
   * @param len The amount of bytes to write to the bus.
   * @return If the operation was successful.
   */
  bool write_bytes(uint8_t a_register, const uint8_t *data, uint8_t len);  // NOLINT

  /** Write len amount of 16-bit words (MSB first) to the specified register.
   *
   * @param a_register The register to write the values to.
   * @param data An array from which len 16-bit words of data will be written to the bus.
   * @param len The amount of bytes to write to the bus.
   * @return If the operation was successful.
   */
  bool write_bytes_16(uint8_t a_register, const uint16_t *data, uint8_t len);  // NOLINT

  /// Write a single byte of data into the specified register. Return true if successful.
  bool write_byte(uint8_t a_register, uint8_t data);  // NOLINT

  /// Write a single 16-bit word of data into the specified register. Return true if successful.
  bool write_byte_16(uint8_t a_register, uint16_t data);  // NOLINT

  uint8_t address_;
  I2CComponent *parent_;
};

ESPHOME_NAMESPACE_END

#endif  // USE_I2C

#endif  // ESPHOME_I2C_COMPONENT_H
