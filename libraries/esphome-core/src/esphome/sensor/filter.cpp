#include "esphome/defines.h"

#ifdef USE_SENSOR

#include "esphome/sensor/filter.h"
#include "esphome/sensor/sensor.h"

#include "esphome/log.h"
#include "esphome/espmath.h"

ESPHOME_NAMESPACE_BEGIN

namespace sensor {

static const char *TAG = "sensor.filter";

// Filter
uint32_t Filter::expected_interval(uint32_t input) { return input; }
void Filter::input(float value) {
  ESP_LOGVV(TAG, "Filter(%p)::input(%f)", this, value);
  optional<float> out = this->new_value(value);
  if (out.has_value())
    this->output(*out);
}
void Filter::output(float value) {
  if (this->next_ == nullptr) {
    ESP_LOGVV(TAG, "Filter(%p)::output(%f) -> SENSOR", this, value);
    this->parent_->internal_send_state_to_frontend(value);
  } else {
    ESP_LOGVV(TAG, "Filter(%p)::output(%f) -> %p", this, value, this->next_);
    this->next_->input(value);
  }
}
void Filter::initialize(Sensor *parent, Filter *next) {
  ESP_LOGVV(TAG, "Filter(%p)::initialize(parent=%p next=%p)", this, parent, next);
  this->parent_ = parent;
  this->next_ = next;
}
uint32_t Filter::calculate_remaining_interval(uint32_t input) {
  uint32_t this_interval = this->expected_interval(input);
  ESP_LOGVV(TAG, "Filter(%p)::calculate_remaining_interval(%u) -> %u", this, input, this_interval);
  if (this->next_ == nullptr) {
    return this_interval;
  } else {
    return this->next_->calculate_remaining_interval(this_interval);
  }
}

// SlidingWindowMovingAverageFilter
SlidingWindowMovingAverageFilter::SlidingWindowMovingAverageFilter(size_t window_size, size_t send_every,
                                                                   size_t send_first_at)
    : send_every_(send_every),
      send_at_(send_every - send_first_at),
      average_(SlidingWindowMovingAverage(window_size)) {}
size_t SlidingWindowMovingAverageFilter::get_send_every() const { return this->send_every_; }
void SlidingWindowMovingAverageFilter::set_send_every(size_t send_every) { this->send_every_ = send_every; }
size_t SlidingWindowMovingAverageFilter::get_window_size() const { return this->average_.get_max_size(); }
void SlidingWindowMovingAverageFilter::set_window_size(size_t window_size) { this->average_.set_max_size(window_size); }
optional<float> SlidingWindowMovingAverageFilter::new_value(float value) {
  float average_value = this->average_.next_value(value);
  ESP_LOGVV(TAG, "SlidingWindowMovingAverageFilter(%p)::new_value(%f) -> %f", this, value, average_value);

  if (++this->send_at_ >= this->send_every_) {
    this->send_at_ = 0;
    ESP_LOGVV(TAG, "SlidingWindowMovingAverageFilter(%p)::new_value(%f) SENDING", this, value);
    return average_value;
  }
  return {};
}

uint32_t SlidingWindowMovingAverageFilter::expected_interval(uint32_t input) { return input * this->send_every_; }

// ExponentialMovingAverageFilter
ExponentialMovingAverageFilter::ExponentialMovingAverageFilter(float alpha, size_t send_every)
    : send_every_(send_every), send_at_(send_every - 1), average_(ExponentialMovingAverage(alpha)) {}
optional<float> ExponentialMovingAverageFilter::new_value(float value) {
  float average_value = this->average_.next_value(value);
  ESP_LOGVV(TAG, "ExponentialMovingAverageFilter(%p)::new_value(%f) -> %f", this, value, average_value);

  if (++this->send_at_ >= this->send_every_) {
    ESP_LOGVV(TAG, "ExponentialMovingAverageFilter(%p)::new_value(%f) SENDING", this, value);
    this->send_at_ = 0;
    return average_value;
  }
  return {};
}
size_t ExponentialMovingAverageFilter::get_send_every() const { return this->send_every_; }
void ExponentialMovingAverageFilter::set_send_every(size_t send_every) { this->send_every_ = send_every; }
float ExponentialMovingAverageFilter::get_alpha() const { return this->average_.get_alpha(); }
void ExponentialMovingAverageFilter::set_alpha(float alpha) { this->average_.set_alpha(alpha); }
uint32_t ExponentialMovingAverageFilter::expected_interval(uint32_t input) { return input * this->send_every_; }

// LambdaFilter
LambdaFilter::LambdaFilter(lambda_filter_t lambda_filter) : lambda_filter_(std::move(lambda_filter)) {}
const lambda_filter_t &LambdaFilter::get_lambda_filter() const { return this->lambda_filter_; }
void LambdaFilter::set_lambda_filter(const lambda_filter_t &lambda_filter) { this->lambda_filter_ = lambda_filter; }

optional<float> LambdaFilter::new_value(float value) {
  auto it = this->lambda_filter_(value);
  ESP_LOGVV(TAG, "LambdaFilter(%p)::new_value(%f) -> %f", this, value, it.value_or(INFINITY));
  return it;
}

// OffsetFilter
OffsetFilter::OffsetFilter(float offset) : offset_(offset) {}

optional<float> OffsetFilter::new_value(float value) { return value + this->offset_; }

// MultiplyFilter
MultiplyFilter::MultiplyFilter(float multiplier) : multiplier_(multiplier) {}

optional<float> MultiplyFilter::new_value(float value) { return value * this->multiplier_; }

// FilterOutValueFilter
FilterOutValueFilter::FilterOutValueFilter(float value_to_filter_out) : value_to_filter_out_(value_to_filter_out) {}

optional<float> FilterOutValueFilter::new_value(float value) {
  if (isnan(this->value_to_filter_out_)) {
    if (isnan(value))
      return {};
    else
      return value;
  } else {
    if (value == this->value_to_filter_out_)
      return {};
    else
      return value;
  }
}

// ThrottleFilter
ThrottleFilter::ThrottleFilter(uint32_t min_time_between_inputs)
    : min_time_between_inputs_(min_time_between_inputs), Filter() {}
optional<float> ThrottleFilter::new_value(float value) {
  const uint32_t now = millis();
  if (this->last_input_ == 0 || now - this->last_input_ >= min_time_between_inputs_) {
    this->last_input_ = now;
    return value;
  }
  return {};
}

// DeltaFilter
DeltaFilter::DeltaFilter(float min_delta) : min_delta_(min_delta), last_value_(NAN) {}
optional<float> DeltaFilter::new_value(float value) {
  if (isnan(value))
    return {};
  if (isnan(this->last_value_)) {
    return this->last_value_ = value;
  }
  if (fabsf(value - this->last_value_) >= this->min_delta_) {
    return this->last_value_ = value;
  }
  return {};
}

// OrFilter
OrFilter::OrFilter(std::vector<Filter *> filters) : filters_(std::move(filters)), phi_(this) {}
OrFilter::PhiNode::PhiNode(OrFilter *parent) : parent_(parent) {}

optional<float> OrFilter::PhiNode::new_value(float value) {
  this->parent_->output(value);

  return {};
}
optional<float> OrFilter::new_value(float value) {
  for (Filter *filter : this->filters_)
    filter->input(value);

  return {};
}
void OrFilter::initialize(Sensor *parent, Filter *next) {
  Filter::initialize(parent, next);
  for (Filter *filter : this->filters_) {
    filter->initialize(parent, &this->phi_);
  }
  this->phi_.initialize(parent, nullptr);
}

uint32_t OrFilter::expected_interval(uint32_t input) {
  uint32_t min_interval = UINT32_MAX;
  for (Filter *filter : this->filters_) {
    min_interval = std::min(min_interval, filter->calculate_remaining_interval(input));
  }

  return min_interval;
}
// DebounceFilter
optional<float> DebounceFilter::new_value(float value) {
  this->set_timeout("debounce", this->time_period_, [this, value]() { this->output(value); });

  return {};
}

DebounceFilter::DebounceFilter(uint32_t time_period) : time_period_(time_period) {}
float DebounceFilter::get_setup_priority() const { return setup_priority::HARDWARE; }

// HeartbeatFilter
HeartbeatFilter::HeartbeatFilter(uint32_t time_period) : time_period_(time_period), last_input_(NAN) {}

optional<float> HeartbeatFilter::new_value(float value) {
  ESP_LOGVV(TAG, "HeartbeatFilter(%p)::new_value(value=%f)", this, value);
  this->last_input_ = value;
  this->has_value_ = true;

  return {};
}
uint32_t HeartbeatFilter::expected_interval(uint32_t input) { return this->time_period_; }
void HeartbeatFilter::setup() {
  this->set_interval("heartbeat", this->time_period_, [this]() {
    ESP_LOGVV(TAG, "HeartbeatFilter(%p)::interval(has_value=%s, last_input=%f)", this, YESNO(this->has_value_),
              this->last_input_);
    if (!this->has_value_)
      return;

    this->output(this->last_input_);
  });
}
float HeartbeatFilter::get_setup_priority() const { return setup_priority::HARDWARE; }

optional<float> CalibrateLinearFilter::new_value(float value) { return value * this->slope_ + this->bias_; }
CalibrateLinearFilter::CalibrateLinearFilter(float slope, float bias) : slope_(slope), bias_(bias) {}

}  // namespace sensor

ESPHOME_NAMESPACE_END

#endif  // USE_SENSOR
