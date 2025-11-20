#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"

namespace esphome {
namespace morse_code {

enum State : uint8_t {
  STATE_STOPPED = 0,
  STATE_INIT,
  STATE_STARTING,
  STATE_RUNNING,
  STATE_STOPPING,
};

class MorseCode : public Component {
 public:
  void set_output(output::BinaryOutput *output) { this->output_ = output; }
  uint16_t get_dit_duration() { return dit_duration_; }
  void set_dit_duration(uint16_t dit_duration) {
    this->dit_duration_ = dit_duration;
    this->dah_duration_ = dit_duration * 3;
    this->space_duration_ = dit_duration;
    this->lspace_duration_ = dit_duration * 3;
    this->wspace_duration_ = dit_duration * 7;
  }
  void start(std::string text);
  void stop();
  void dump_config() override;

  bool is_running() { return this->state_ != State::STATE_STOPPED; }
  void loop() override;

  void add_on_finished_callback(std::function<void()> callback) {
    this->on_finished_callback_.add(std::move(callback));
  }

 protected:
  void finish_();
  void set_state_(State state);

  std::string text_{""};
  size_t position_{0};
  uint16_t current_symbol_;

  bool set_output_;
  uint16_t next_output_;

  uint16_t dit_duration_{100};
  uint16_t dah_duration_{300};
  uint16_t space_duration_{100};
  uint16_t lspace_duration_{300};
  uint16_t wspace_duration_{700};

  State state_{State::STATE_STOPPED};

  output::BinaryOutput *output_;

  CallbackManager<void()> on_finished_callback_;
};

template<typename... Ts> class StartAction : public Action<Ts...> {
 public:
  StartAction(MorseCode *morse_code) : morse_code_(morse_code) {}
  TEMPLATABLE_VALUE(std::string, value)

  void play(const Ts&... x) override { this->morse_code_->start(this->value_.value(x...)); }

 protected:
  MorseCode *morse_code_;
};

template<typename... Ts> class StopAction : public Action<Ts...>, public Parented<MorseCode> {
 public:
  void play(const Ts&... x) override { this->parent_->stop(); }
};

template<typename... Ts> class IsRunningCondition : public Condition<Ts...>, public Parented<MorseCode> {
 public:
  bool check(const Ts&... x) override { return this->parent_->is_running(); }
};

class FinishedTrigger : public Trigger<> {
 public:
  explicit FinishedTrigger(MorseCode *parent) {
    parent->add_on_finished_callback([this]() { this->trigger(); });
  }
};

}  // namespace morse_code
}  // namespace esphome
