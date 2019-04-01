#ifndef ESPHOME_HELPERS_H
#define ESPHOME_HELPERS_H

#include <string>
#include <IPAddress.h>
#include <memory>
#include <queue>
#include <functional>
#include <ArduinoJson.h>

#include "esphome/esphal.h"
#include "esphome/defines.h"
#include "esphome/optional.h"

#ifndef JSON_BUFFER_SIZE
#define JSON_BUFFER_SIZE (JSON_OBJECT_SIZE(16))
#endif

#ifdef ARDUINO_ARCH_ESP32
#include <driver/rmt.h>
#endif

#ifdef CLANG_TIDY
#undef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR
#undef ICACHE_RODATA_ATTR
#define ICACHE_RODATA_ATTR
#endif

ESPHOME_NAMESPACE_BEGIN

/// Callback function typedef for parsing JsonObjects.
using json_parse_t = std::function<void(JsonObject &)>;

/// Callback function typedef for building JsonObjects.
using json_build_t = std::function<void(JsonObject &)>;

/// The characters that are allowed in a hostname.
extern const char *HOSTNAME_CHARACTER_WHITELIST;

/// Gets the MAC address as a string, this can be used as way to identify this ESP.
std::string get_mac_address();

std::string get_mac_address_pretty();

void tick_status_led();

void feed_wdt();

std::string to_string(std::string val);
std::string to_string(String val);
std::string to_string(int val);
std::string to_string(long val);
std::string to_string(long long val);
std::string to_string(unsigned val);
std::string to_string(unsigned long val);
std::string to_string(unsigned long long val);
std::string to_string(float val);
std::string to_string(double val);
std::string to_string(long double val);
optional<float> parse_float(const std::string &str);

/// Constructs a hostname by concatenating base, a hyphen, and the MAC address.
std::string generate_hostname(const std::string &base);

/// Sanitize the hostname by removing characters that are not in the whitelist and truncating it to 63 chars.
std::string sanitize_hostname(const std::string &hostname);

/// Truncate a string to a specific length
std::string truncate_string(const std::string &s, size_t length);

/// Checks whether the provided IPAddress is empty (is 0.0.0.0).
bool is_empty(const IPAddress &address);

/// Force a shutdown (and reboot) of the ESP, calling any registered shutdown hooks.
void reboot(const char *cause) __attribute__((noreturn));

/// Add a shutdown callback.
void add_shutdown_hook(std::function<void(const char *)> &&f);

/// Create a safe shutdown (and reboot) of the ESP, calling any registered shutdown and safe shutdown hooks.
void safe_reboot(const char *cause) __attribute__((noreturn));

/// Run shutdown hooks.
void run_shutdown_hooks(const char *cause);

/// Add a safe shutdown callback that will be called if the device is shut down intentionally.
void add_safe_shutdown_hook(std::function<void(const char *)> &&f);

/// Run safe shutdown and force shutdown hooks.
void run_safe_shutdown_hooks(const char *cause);

/// Convert the string to lowercase_underscore.
std::string to_lowercase_underscore(std::string s);

/// Build a JSON string with the provided json build function.
const char *build_json(const json_build_t &f, size_t *length);

std::string build_json(const json_build_t &f);

/// Parse a JSON string and run the provided json parse function if it's valid.
void parse_json(const std::string &data, const json_parse_t &f);

class HighFrequencyLoopRequester {
 public:
  void start();
  void stop();

  static bool is_high_frequency();

 protected:
  bool started_{false};
};

/** Clamp the value between min and max.
 *
 * @tparam T The input/output typename.
 * @param min The minimum value.
 * @param max The maximum value.
 * @param val The value.
 * @return val clamped in between min and max.
 */
template<typename T> T clamp(T min, T max, T val);

/** Linearly interpolate between end start and end by completion.
 *
 * @tparam T The input/output typename.
 * @param start The start value.
 * @param end The end value.
 * @param completion The completion. 0 is start value, 1 is end value.
 * @return The linearly interpolated value.
 */
template<typename T> T lerp(T start, T end, T completion);

/// std::make_unique
template<typename T, typename... Args> std::unique_ptr<T> make_unique(Args &&... args);

/// Return a random 32 bit unsigned integer.
uint32_t random_uint32();

/** Returns a random double between 0 and 1.
 *
 * Note: This function probably doesn't provide a truly uniform distribution.
 */
double random_double();

/// Returns a random float between 0 and 1. Essentially just casts random_double() to a float.
float random_float();

void fast_random_set_seed(uint32_t seed);
uint32_t fast_random_32();
uint16_t fast_random_16();
uint8_t fast_random_8();

/// Applies gamma correction with the provided gamma to value.
float gamma_correct(float value, float gamma);

/// Create a string from a value and an accuracy in decimals.
std::string value_accuracy_to_string(float value, int8_t accuracy_decimals);

/// Convert a uint64_t to a hex string
std::string uint64_to_string(uint64_t num);

/// Convert a uint32_t to a hex string
std::string uint32_to_string(uint32_t num);

/// Sanitizes the input string with the whitelist.
std::string sanitize_string_whitelist(const std::string &s, const std::string &whitelist);

uint8_t reverse_bits_8(uint8_t x);
uint16_t reverse_bits_16(uint16_t x);
uint32_t reverse_bits_32(uint32_t x);

/** Cross-platform method to disable interrupts.
 *
 * Useful when you need to do some timing-dependent communication.
 *
 * @see Do not forget to call `enable_interrupts()` again or otherwise things will go very wrong.
 */
void disable_interrupts();

/// Cross-platform method to enable interrupts after they have been disabled.
void enable_interrupts();

/// Calculate a crc8 of data with the provided data length.
uint8_t crc8(uint8_t *data, uint8_t len);

enum ParseOnOffState {
  PARSE_NONE = 0,
  PARSE_ON,
  PARSE_OFF,
  PARSE_TOGGLE,
};

ParseOnOffState parse_on_off(const char *str, const char *on = nullptr, const char *off = nullptr);

/// Helper class that implements a sliding window moving average.
class SlidingWindowMovingAverage {
 public:
  /** Create the SlidingWindowMovingAverage.
   *
   * @param max_size The window size.
   */
  explicit SlidingWindowMovingAverage(size_t max_size);

  /** Add value to the interval buffer.
   *
   * @param value The value.
   * @return The new average.
   */
  float next_value(float value);

  /// Return the average across the sliding window.
  float calculate_average();

  size_t get_max_size() const;
  void set_max_size(size_t max_size);

 protected:
  std::queue<float> queue_;
  size_t max_size_;
  float sum_;
};

/// Helper class that implements an exponential moving average.
class ExponentialMovingAverage {
 public:
  explicit ExponentialMovingAverage(float alpha);

  float next_value(float value);

  float calculate_average();

  void set_alpha(float alpha);
  float get_alpha() const;

 protected:
  bool first_value_{true};
  float alpha_;
  float accumulator_;
};

// https://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer/7858971#7858971
template<int...> struct seq {};                                       // NOLINT
template<int N, int... S> struct gens : gens<N - 1, N - 1, S...> {};  // NOLINT
template<int... S> struct gens<0, S...> { using type = seq<S...>; };  // NOLINT

template<typename... X> class CallbackManager;

/** Simple helper class to allow having multiple subscribers to a signal.
 *
 * @tparam Ts The arguments for the callback, wrapped in void().
 */
template<typename... Ts> class CallbackManager<void(Ts...)> {
 public:
  /// Add a callback to the internal callback list.
  void add(std::function<void(Ts...)> &&callback);

  /// Call all callbacks in this manager.
  void call(Ts... args);

 protected:
  std::vector<std::function<void(Ts...)>> callbacks_;
};

// https://stackoverflow.com/a/37161919/8924614
template<class T, class... Args>
struct is_callable  // NOLINT
{
  template<class U> static auto test(U *p) -> decltype((*p)(std::declval<Args>()...), void(), std::true_type());

  template<class U> static auto test(...) -> decltype(std::false_type());

  static constexpr auto value = decltype(test<T>(nullptr))::value;  // NOLINT
};

template<bool B, class T = void> using enable_if_t = typename std::enable_if<B, T>::type;

template<typename T, typename... X> class TemplatableValue {
 public:
  TemplatableValue() : type_(EMPTY) {}

  template<typename F, enable_if_t<!is_callable<F, X...>::value, int> = 0>
  TemplatableValue(F value) : type_(VALUE), value_(value) {}

  template<typename F, enable_if_t<is_callable<F, X...>::value, int> = 0>
  TemplatableValue(F f) : type_(LAMBDA), f_(f) {}

  bool has_value() { return this->type_ != EMPTY; }

  T value(X... x) {
    if (this->type_ == LAMBDA) {
      return this->f_(x...);
    }
    // return value also when empty
    return this->value_;
  }

 protected:
  enum {
    EMPTY,
    VALUE,
    LAMBDA,
  } type_;

  T value_;
  std::function<T(X...)> f_;
};

extern CallbackManager<void(const char *)> shutdown_hooks;
extern CallbackManager<void(const char *)> safe_shutdown_hooks;

#ifdef ARDUINO_ARCH_ESP32
extern rmt_channel_t next_rmt_channel;

rmt_channel_t select_next_rmt_channel();
#endif

void delay_microseconds_accurate(uint32_t usec);

class VectorJsonBuffer : public ArduinoJson::Internals::JsonBufferBase<VectorJsonBuffer> {
 public:
  class String {
   public:
    String(VectorJsonBuffer *parent);

    void append(char c) const;

    const char *c_str() const;

   protected:
    VectorJsonBuffer *parent_;
    uint32_t start_;
  };

  void *alloc(size_t bytes) override;

  size_t size() const;

  void clear();

  String startString();  // NOLINT

 protected:
  void *do_alloc(size_t bytes);  // NOLINT

  void resize(size_t size);  // NOLINT

  void reserve(size_t size);  // NOLINT

  char *buffer_{nullptr};
  size_t size_{0};
  size_t capacity_{0};
  std::vector<char *> free_blocks_;
};

extern VectorJsonBuffer global_json_buffer;

template<typename T> class Deduplicator {
 public:
  bool next(T value);
  bool has_value() const;

 protected:
  bool has_value_{false};
  T last_value_{};
};

uint32_t fnv1_hash(const std::string &str);

// ================================================
//                 Definitions
// ================================================

template<typename T> T clamp(T min, T max, T val) {
  if (min > max)
    std::swap(min, max);
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

template<typename T> T lerp(T start, T end, T completion) { return start + (end - start) * completion; }

template<typename T, typename... Args> std::unique_ptr<T> make_unique(Args &&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename... Ts> void CallbackManager<void(Ts...)>::add(std::function<void(Ts...)> &&callback) {
  this->callbacks_.push_back(std::move(callback));
}
template<typename... Ts> void CallbackManager<void(Ts...)>::call(Ts... args) {
  for (auto &cb : this->callbacks_)
    cb(args...);
}

template<typename T> bool Deduplicator<T>::next(T value) {
  if (this->has_value_) {
    if (this->last_value_ == value)
      return false;
  }
  this->has_value_ = true;
  this->last_value_ = value;
  return true;
}
template<typename T> bool Deduplicator<T>::has_value() const { return this->has_value_; }

ESPHOME_NAMESPACE_END

#endif  // ESPHOME_HELPERS_H
