#ifndef ESPHOME_AUTOMATION_H
#define ESPHOME_AUTOMATION_H

#include <vector>
#include "esphome/espmath.h"
#include "esphome/component.h"
#include "esphome/helpers.h"
#include "esphome/defines.h"
#include "esphome/esppreferences.h"

ESPHOME_NAMESPACE_BEGIN

template<typename... Ts> class Condition {
 public:
  virtual bool check(Ts... x) = 0;

  bool check_tuple(const std::tuple<Ts...> &tuple);

 protected:
  template<int... S> bool check_tuple_(const std::tuple<Ts...> &tuple, seq<S...>);
};

template<typename... Ts> class AndCondition : public Condition<Ts...> {
 public:
  explicit AndCondition(const std::vector<Condition<Ts...> *> &conditions);
  bool check(Ts... x) override;

 protected:
  std::vector<Condition<Ts...> *> conditions_;
};

template<typename... Ts> class OrCondition : public Condition<Ts...> {
 public:
  explicit OrCondition(const std::vector<Condition<Ts...> *> &conditions);
  bool check(Ts... x) override;

 protected:
  std::vector<Condition<Ts...> *> conditions_;
};

template<typename... Ts> class LambdaCondition : public Condition<Ts...> {
 public:
  explicit LambdaCondition(std::function<bool(Ts...)> &&f);
  bool check(Ts... x) override;

 protected:
  std::function<bool(Ts...)> f_;
};

class RangeCondition : public Condition<float> {
 public:
  explicit RangeCondition();
  bool check(float x) override;

  template<typename V> void set_min(V value) { this->min_ = value; }
  template<typename V> void set_max(V value) { this->max_ = value; }

 protected:
  TemplatableValue<float, float> min_{NAN};
  TemplatableValue<float, float> max_{NAN};
};

template<typename... Ts> class Automation;

template<typename... Ts> class Trigger {
 public:
  void trigger(Ts... x);
  void set_parent(Automation<Ts...> *parent);
  void stop();

 protected:
  Automation<Ts...> *parent_{nullptr};
};

class StartupTrigger : public Trigger<>, public Component {
 public:
  explicit StartupTrigger(float setup_priority = setup_priority::LATE);
  void setup() override;
  float get_setup_priority() const override;

 protected:
  float setup_priority_;
};

class ShutdownTrigger : public Trigger<const char *> {
 public:
  ShutdownTrigger();
};

class LoopTrigger : public Trigger<>, public Component {
 public:
  void loop() override;
  float get_setup_priority() const override;
};

class IntervalTrigger : public Trigger<>, public PollingComponent {
 public:
  IntervalTrigger(uint32_t update_interval);
  void update() override;
  float get_setup_priority() const override;
};

template<typename... Ts> class ScriptExecuteAction;
template<typename... Ts> class ScriptStopAction;

template<typename... Ts> class ScriptStopAction;

class Script : public Trigger<> {
 public:
  void execute();

  void stop();

  template<typename... Ts> ScriptExecuteAction<Ts...> *make_execute_action();

  template<typename... Ts> ScriptStopAction<Ts...> *make_stop_action();
};

template<typename... Ts> class ActionList;

template<typename... Ts> class Action {
 public:
  virtual void play(Ts... x) = 0;
  void play_next(Ts... x);
  virtual void stop();
  void stop_next();

  void play_next_tuple(const std::tuple<Ts...> &tuple);

 protected:
  friend ActionList<Ts...>;

  template<int... S> void play_next_tuple_(const std::tuple<Ts...> &tuple, seq<S...>);

  Action<Ts...> *next_ = nullptr;
};

template<typename... Ts> class DelayAction : public Action<Ts...>, public Component {
 public:
  explicit DelayAction();

  template<typename V> void set_delay(V value) { this->delay_ = value; }
  void stop() override;

  void play(Ts... x) override;
  float get_setup_priority() const override;

 protected:
  TemplatableValue<uint32_t, Ts...> delay_{0};
};

template<typename... Ts> class LambdaAction : public Action<Ts...> {
 public:
  explicit LambdaAction(std::function<void(Ts...)> &&f);
  void play(Ts... x) override;

 protected:
  std::function<void(Ts...)> f_;
};

template<typename... Ts> class IfAction : public Action<Ts...> {
 public:
  explicit IfAction(std::vector<Condition<Ts...> *> conditions);

  void add_then(const std::vector<Action<Ts...> *> &actions);

  void add_else(const std::vector<Action<Ts...> *> &actions);

  void play(Ts... x) override;

  void stop() override;

 protected:
  std::vector<Condition<Ts...> *> conditions_;
  ActionList<Ts...> then_;
  ActionList<Ts...> else_;
};

template<typename... Ts> class WhileAction : public Action<Ts...> {
 public:
  WhileAction(const std::vector<Condition<Ts...> *> &conditions);

  void add_then(const std::vector<Action<Ts...> *> &actions);

  void play(Ts... x) override;

  void stop() override;

 protected:
  std::vector<Condition<Ts...> *> conditions_;
  ActionList<Ts...> then_;
  bool is_running_{false};
};

template<typename... Ts> class WaitUntilAction : public Action<Ts...>, public Component {
 public:
  WaitUntilAction(const std::vector<Condition<Ts...> *> &conditions);

  void play(Ts... x) override;

  void stop() override;

  void loop() override;

  float get_setup_priority() const override;

 protected:
  std::vector<Condition<Ts...> *> conditions_;
  bool triggered_{false};
  std::tuple<Ts...> var_{};
};

template<typename... Ts> class UpdateComponentAction : public Action<Ts...> {
 public:
  UpdateComponentAction(PollingComponent *component);
  void play(Ts... x) override;

 protected:
  PollingComponent *component_;
};

template<typename... Ts> class ScriptExecuteAction : public Action<Ts...> {
 public:
  ScriptExecuteAction(Script *script);

  void play(Ts... x) override;

 protected:
  Script *script_;
};

template<typename... Ts> class ScriptStopAction : public Action<Ts...> {
 public:
  ScriptStopAction(Script *script);

  void play(Ts... x) override;

 protected:
  Script *script_;
};

template<typename... Ts> class ActionList {
 public:
  Action<Ts...> *add_action(Action<Ts...> *action);
  void add_actions(const std::vector<Action<Ts...> *> &actions);
  void play(Ts... x);
  void stop();
  bool empty() const;

 protected:
  Action<Ts...> *actions_begin_{nullptr};
  Action<Ts...> *actions_end_{nullptr};
};

template<typename... Ts> class Automation {
 public:
  explicit Automation(Trigger<Ts...> *trigger);

  Condition<Ts...> *add_condition(Condition<Ts...> *condition);
  void add_conditions(const std::vector<Condition<Ts...> *> &conditions);

  Action<Ts...> *add_action(Action<Ts...> *action);
  void add_actions(const std::vector<Action<Ts...> *> &actions);

  void stop();

  void trigger(Ts... x);

 protected:
  Trigger<Ts...> *trigger_;
  std::vector<Condition<Ts...> *> conditions_;
  ActionList<Ts...> actions_;
};

template<typename T> class GlobalVariableComponent : public Component {
 public:
  explicit GlobalVariableComponent();
  explicit GlobalVariableComponent(T initial_value);
  explicit GlobalVariableComponent(
      std::array<typename std::remove_extent<T>::type, std::extent<T>::value> initial_value);

  T &value();

  void setup() override;

  float get_setup_priority() const override;

  void loop() override;

  void set_restore_value(uint32_t name_hash);

 protected:
  T value_{};
  T prev_value_{};
  bool restore_value_{false};
  uint32_t name_hash_{};
  ESPPreferenceObject rtc_;
};

ESPHOME_NAMESPACE_END

#include "esphome/automation.tcc"

#endif  // ESPHOME_AUTOMATION_H
