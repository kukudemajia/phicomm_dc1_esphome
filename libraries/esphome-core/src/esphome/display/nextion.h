#ifndef ESPHOME_DISPLAY_NEXTION_H
#define ESPHOME_DISPLAY_NEXTION_H

#include "esphome/defines.h"

#ifdef USE_NEXTION

#include "esphome/component.h"
#include "esphome/uart_component.h"
#include "esphome/time/rtc_component.h"
#include "esphome/binary_sensor/binary_sensor.h"

ESPHOME_NAMESPACE_BEGIN

namespace display {

class NextionTouchComponent;
class Nextion;

using nextion_writer_t = std::function<void(Nextion &)>;

class Nextion : public PollingComponent, public UARTDevice {
 public:
  /**
   * Set the text of a component to a static string.
   * @param component The component name.
   * @param text The static text to set.
   */
  void set_component_text(const char *component, const char *text);
  /**
   * Set the text of a component to a formatted string
   * @param component The component name.
   * @param format The printf-style format string.
   * @param ... The arguments to the format.
   */
  void set_component_text_printf(const char *component, const char *format, ...) __attribute__((format(printf, 3, 4)));
  /**
   * Set the integer value of a component
   * @param component The component name.
   * @param value The value to set.
   */
  void set_component_value(const char *component, int value);
  /**
   * Set the picture of an image component.
   * @param component The component name.
   * @param value The picture name.
   */
  void set_component_picture(const char *component, const char *picture);
  /**
   * Set the background color of a component.
   * @param component The component name.
   * @param color The color (as a string).
   */
  void set_component_background_color(const char *component, const char *color);
  /**
   * Set the pressed background color of a component.
   * @param component The component name.
   * @param color The color (as a string).
   */
  void set_component_pressed_background_color(const char *component, const char *color);
  /**
   * Set the font color of a component.
   * @param component The component name.
   * @param color The color (as a string).
   */
  void set_component_font_color(const char *component, const char *color);
  /**
   * Set the pressed font color of a component.
   * @param component The component name.
   * @param color The color (as a string).
   */
  void set_component_pressed_font_color(const char *component, const char *color);
  /**
   * Set the coordinates of a component on screen.
   * @param component The component name.
   * @param x The x coordinate.
   * @param y The y coordinate.
   */
  void set_component_coordinates(const char *component, int x, int y);
  /**
   * Set the font id for a component.
   * @param component The component name.
   * @param font_id The ID of the font (number).
   */
  void set_component_font(const char *component, uint8_t font_id);
#ifdef USE_TIME
  /**
   * Send the current time to the nextion display.
   * @param time The time instance to send (get this with id(my_time).now() ).
   */
  void set_nextion_rtc_time(time::ESPTime time);
#endif

  /**
   * Show the page with a given name.
   * @param page The name of the page.
   */
  void goto_page(const char *page);
  /**
   * Hide a component.
   * @param component The component name.
   */
  void hide_component(const char *component);
  /**
   * Show a component.
   * @param component The component name.
   */
  void show_component(const char *component);
  /**
   * Enable touch for a component.
   * @param component The component name.
   */
  void enable_component_touch(const char *component);
  /**
   * Disable touch for a component.
   * @param component The component name.
   */
  void disable_component_touch(const char *component);
  /**
   * Add waveform data to a waveform component
   * @param component_id The integer component id.
   * @param channel_number The channel number to write to.
   * @param value The value to write.
   */
  void add_waveform_data(int component_id, uint8_t channel_number, uint8_t value);
  /**
   * Display a picture at coordinates.
   * @param picture_id The picture id.
   * @param x1 The x coordinate.
   * @param y1 The y coordniate.
   */
  void display_picture(int picture_id, int x_start, int y_start);
  /**
   * Fill a rectangle with a color.
   * @param x1 The starting x coordinate.
   * @param y1 The starting y coordinate.
   * @param width The width to draw.
   * @param height The height to draw.
   * @param color The color to draw with (as a string).
   */
  void fill_area(int x1, int y1, int width, int height, const char *color);
  /**
   * Draw a line on the screen.
   * @param x1 The starting x coordinate.
   * @param y1 The starting y coordinate.
   * @param x2 The ending x coordinate.
   * @param y2 The ending y coordinate.
   * @param color The color to draw with (as a string).
   */
  void line(int x1, int y1, int x2, int y2, const char *color);
  /**
   * Draw a rectangle outline.
   * @param x1 The starting x coordinate.
   * @param y1 The starting y coordinate.
   * @param width The width of the rectangle.
   * @param height The height of the rectangle.
   * @param color The color to draw with (as a string).
   */
  void rectangle(int x1, int y1, int width, int height, const char *color);
  /**
   * Draw a circle outline
   * @param center_x The center x coordinate.
   * @param center_y The center y coordinate.
   * @param radius The circle radius.
   * @param color The color to draw with (as a string).
   */
  void circle(int center_x, int center_y, int radius, const char *color);
  /**
   * Draw a filled circled.
   * @param center_x The center x coordinate.
   * @param center_y The center y coordinate.
   * @param radius The circle radius.
   * @param color The color to draw with (as a string).
   */
  void filled_circle(int center_x, int center_y, int radius, const char *color);

  /** Set the brightness of the backlight.
   *
   * @param brightness The brightness, from 0 to 100.
   */
  void set_backlight_brightness(uint8_t brightness);
  /**
   * Set the touch sleep timeout of the display.
   * @param timeout Timeout in seconds.
   */
  void set_touch_sleep_timeout(uint16_t timeout);

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  Nextion(UARTComponent *parent, uint32_t update_interval = 5000);
  NextionTouchComponent *make_touch_component(const std::string &name, uint8_t page_id, uint8_t component_id);
  void setup() override;
  float get_setup_priority() const override;
  void update() override;
  void loop() override;
  void set_writer(const nextion_writer_t &writer);

  /**
   * Manually send a raw command to the display and don't wait for an acknowledgement packet.
   * @param command The command to write, for example "vis b0,0".
   */
  void send_command_no_ack(const char *command);
  /**
   * Manually send a raw formatted command to the display.
   * @param format The printf-style command format, like "vis %s,0"
   * @param ... The format arguments
   * @return Whether the send was successful.
   */
  bool send_command_printf(const char *format, ...) __attribute__((format(printf, 2, 3)));

  void set_wait_for_ack(bool wait_for_ack);

 protected:
  bool ack_();
  bool read_until_ack_();

  std::vector<NextionTouchComponent *> touch_;
  optional<nextion_writer_t> writer_;
  bool wait_for_ack_{true};
};

class NextionTouchComponent : public binary_sensor::BinarySensor {
 public:
  NextionTouchComponent(const std::string &name, uint8_t page_id, uint8_t component_id);
  void process(uint8_t page_id, uint8_t component_id, bool on);

 protected:
  uint8_t page_id_;
  uint8_t component_id_;
};

}  // namespace display

ESPHOME_NAMESPACE_END

#endif  // USE_NEXTION

#endif  // ESPHOME_DISPLAY_NEXTION_H
