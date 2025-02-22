#include "morse_code.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace morse_code {

static const char *const TAG = "morse_code";

static const uint16_t SYMBOL_TABLE[] = {
  0xDDF0, 0x75D0, 0x0000, 0x575C, 0x0000, 0x7540, 0x7FD0, 0xDF40, 0xDF70, 0x0000,
  0x7740, 0xF5F0, 0xD570, 0x7770, 0xD740,
  0xFFC0, 0x7FC0, 0x5FC0, 0x57C0, 0x55C0, 0x5540, 0xD540, 0xF540, 0xFD40, 0xFF40,
  0xFD50, 0xDDD0, 0x0000, 0xD5C0, 0x0000, 0x5F50, 0x7DD0,
  0x7000, 0xD500, 0xDD00, 0xD400, 0x4000, 0x5D00, 0xF400, 0x5500, 0x5000, 0x7F00,
  0xDC00, 0x7500, 0xF000, 0xD000, 0xFC00, 0x7D00, 0xF700, 0x7400, 0x5400, 0xC000,
  0x5C00, 0x5700, 0x7C00, 0xD700, 0xDF00, 0xF500,
  0x0000, 0x0000, 0x0000, 0x0000, 0x5F70, 0x0000,
  0x7000, 0xD500, 0xDD00, 0xD400, 0x4000, 0x5D00, 0xF400, 0x5500, 0x5000, 0x7F00,
  0xDC00, 0x7500, 0xF000, 0xD000, 0xFC00, 0x7D00, 0xF700, 0x7400, 0x5400, 0xC000,
  0x5C00, 0x5700, 0x7C00, 0xD700, 0xDF00, 0xF500,
  0x0000, 0x0000, 0x0000, 0x0000
};

uint16_t char2symbol(char c) {
  if ((c == 9) || (c == 13) || (c == 32)) return 0x8000;
  if ((c < 33) || (c > 126)) return 0;
  return SYMBOL_TABLE[c - 33];
}

void MorseCode::dump_config() {
  ESP_LOGCONFIG(TAG, "Morse_Code:");
  ESP_LOGCONFIG(TAG, "  Dit duration: %i ms", this->dit_duration_);
}

void MorseCode::start(std::string text) {
  this->text_ = std::move(text);

  ESP_LOGD(TAG, "Starting to transmit: %s", this->text_.c_str());

  if (this->text_.length() == 0) {
    ESP_LOGW(TAG, "Nothing to transmit.");
    return;
  }

  this->position_ = 0;
  this->next_output_ = 0;
  this->set_output_ = false;

  char first_char = this->text_[this->position_];
  this->current_symbol_ = char2symbol(first_char);
  ESP_LOGV(TAG, "First character = %c (0x%04X)", first_char, this->current_symbol_);

  if (this->output_ != nullptr) {
    this->set_state_(State::STATE_RUNNING);
  }
}

void MorseCode::stop() {
  if (this->output_ != nullptr) {
    this->output_->set_state(false);
  }
  this->set_state_(STATE_STOPPED);
  this->current_symbol_ = 0;
  this->next_output_ = 0;
  this->set_output_ = false;
}

void MorseCode::loop() {
  if (this->state_ == State::STATE_STOPPED)
    return;

  uint16_t cur_millis = millis();

  if (cur_millis < this->next_output_)
    return;

  if (this->set_output_) {
    this->set_output_ = false;
    this->next_output_ = cur_millis + this->space_duration_;
  }
  else {
    this->next_output_ = 0;
    if (this->current_symbol_ == 0x0000) {
      this->next_output_ = cur_millis + this->lspace_duration_ - this->space_duration_;
      ESP_LOGVV(TAG, "end of symbol");
    }
    else if (this->current_symbol_ == 0x8000) {
      this->next_output_ = cur_millis + this->wspace_duration_ - this->space_duration_;
      ESP_LOGVV(TAG, "word space");
      this->current_symbol_ = 0x0000;
    }
    else if ((this->current_symbol_ & 0xC000) == 0xC000) { // = 11xx xxxx
      this->next_output_ = cur_millis + this->dah_duration_;
      this->set_output_ = true;
      ESP_LOGVV(TAG, "dah");
    }
    else if ((this->current_symbol_ & 0xC000) == 0x4000) { // 01xx xxxx
      this->next_output_ = cur_millis + this->dit_duration_;
      this->set_output_ = true;
      ESP_LOGVV(TAG, "dit");
    }
    if (this->current_symbol_ == 0x0000) {
      this->position_++;
      if (!this->text_[this->position_]) {
        this->finish_();
        return;
      }
      char next_char = this->text_[this->position_];
      this->current_symbol_ = char2symbol(next_char);
      ESP_LOGV(TAG, "Next character = %c (0x%04X)", next_char, this->current_symbol_);
    }
    else {
      this->current_symbol_ = this->current_symbol_ << 2;
    }
  }

  this->output_->set_state(this->set_output_);
}

void MorseCode::finish_() {
  if (this->output_ != nullptr) {
    this->output_->set_state(false);
  }
  this->set_state_(State::STATE_STOPPED);
  this->next_output_ = 0;
  this->current_symbol_ = 0;
  this->on_finished_callback_.call();
  ESP_LOGD(TAG, "Finished.");
}

static const LogString *state_to_string(State state) {
  switch (state) {
    case STATE_STOPPED:
      return LOG_STR("STATE_STOPPED");
    case STATE_STARTING:
      return LOG_STR("STATE_STARTING");
    case STATE_RUNNING:
      return LOG_STR("STATE_RUNNING");
    case STATE_STOPPING:
      return LOG_STR("STATE_STOPPING");
    case STATE_INIT:
      return LOG_STR("STATE_INIT");
    default:
      return LOG_STR("UNKNOWN");
  }
};

void MorseCode::set_state_(State state) {
  State old_state = this->state_;
  this->state_ = state;
  ESP_LOGD(TAG, "State changed from %s to %s", LOG_STR_ARG(state_to_string(old_state)),
           LOG_STR_ARG(state_to_string(state)));
}

}  // namespace morse_code
}  // namespace esphome
