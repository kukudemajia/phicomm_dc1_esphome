#include "esphome/defines.h"

#ifdef USE_MQTT_LIGHT

#include "esphome/light/mqtt_json_light_component.h"

#include "esphome/log.h"

ESPHOME_NAMESPACE_BEGIN

namespace light {

static const char *TAG = "light.mqtt_json";

std::string MQTTJSONLightComponent::component_type() const { return "light"; }

void MQTTJSONLightComponent::setup() {
  this->subscribe_json(this->get_command_topic_(), [this](const std::string &topic, JsonObject &root) {
    this->state_->make_call().parse_json(root).perform();
  });

  auto f = std::bind(&MQTTJSONLightComponent::publish_state_, this);
  this->state_->add_new_remote_values_callback([this, f]() { this->defer("send", f); });
}

MQTTJSONLightComponent::MQTTJSONLightComponent(LightState *state) : MQTTComponent(), state_(state) {}

bool MQTTJSONLightComponent::publish_state_() {
  return this->publish_json(this->get_state_topic_(), [this](JsonObject &root) { this->state_->dump_json(root); });
}
LightState *MQTTJSONLightComponent::get_state() const { return this->state_; }
std::string MQTTJSONLightComponent::friendly_name() const { return this->state_->get_name(); }
void MQTTJSONLightComponent::send_discovery(JsonObject &root, mqtt::SendDiscoveryConfig &config) {
  root["schema"] = "json";
  if (this->state_->get_traits().has_brightness())
    root["brightness"] = true;
  if (this->state_->get_traits().has_rgb())
    root["rgb"] = true;
  if (this->state_->get_traits().has_color_temperature())
    root["color_temp"] = true;
  if (this->state_->get_traits().has_rgb_white_value())
    root["white_value"] = true;
  if (this->state_->supports_effects()) {
    root["effect"] = true;
    JsonArray &effect_list = root.createNestedArray("effect_list");
    for (auto *effect : this->state_->get_effects())
      effect_list.add(effect->get_name());
    effect_list.add("None");
  }
}
bool MQTTJSONLightComponent::send_initial_state() { return this->publish_state_(); }
bool MQTTJSONLightComponent::is_internal() { return this->state_->is_internal(); }
void MQTTJSONLightComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "MQTT Light '%s':", this->state_->get_name().c_str());
  LOG_MQTT_COMPONENT(true, true)
}

}  // namespace light

ESPHOME_NAMESPACE_END

#endif  // USE_MQTT_LIGHT
