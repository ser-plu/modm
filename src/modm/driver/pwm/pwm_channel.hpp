#pragma once

#include "modm/platform.hpp"
#include "pwm_channel_iface.hpp"
#include "system_clock.hpp"

template <typename Timer, typename Channel, typename SystemClock>
class PwmChannel : public IPwmChannel {
 public:
  void Init(bool invert = false, uint32_t pwm_freq = IPwmChannel::kDefaultPwmFreq) final {
    if (!Timer::isEnabled()) {
      InitTimer(pwm_freq);
    }
    inverted_ = invert;

    Timer::template connect<Channel>();
    Start();
  }

  void SetDutyCycle(float duty_cycle) final {
    if (duty_cycle > 1.f) {
      duty_cycle = 1.f;
    }

    Timer::template setCompareValue<Channel>(Timer::getOverflow() * duty_cycle);
  }

  void Stop() final {
    auto pwm_mode =
        inverted_ ? Timer::OutputCompareMode::ForceActive : Timer::OutputCompareMode::ForceInactive;
    ConfigureChannel(pwm_mode);
    Timer::pause();
    running_ = false;
  }

  void Start() final {
    auto pwm_mode = inverted_ ? Timer::OutputCompareMode::Pwm2 : Timer::OutputCompareMode::Pwm;
    ConfigureChannel(pwm_mode);
    Timer::start();
    running_ = true;
  }

  void SetInverted(bool invert) final {
    inverted_ = invert;
    if (running_) {
      Start();
    } else {
      Stop();
    }
  }

 private:
  static bool IsTimerInitialized();
  void ConfigureChannel(Timer::OutputCompareMode pwm_mode);

  void InitTimer(uint32_t pwm_frequency) {
    Timer::enable();

    Timer::setMode(Timer::Mode::UpCounter);

    static constexpr std::chrono::microseconds kUsInSecond = 1'000'000us;
    Timer::template setPeriod<SystemClock>(kUsInSecond / pwm_frequency);

    if constexpr (Timer::hasAdvancedPwmControl()) {
      Timer::enableOutput();
    }
  }
};

template <typename Timer, typename Channel, typename SystemClock>
void PwmChannel<Timer, Channel, SystemClock>::ConfigureChannel(Timer::OutputCompareMode pwm_mode) {
  if constexpr (Timer::hasAdvancedPwmControl()) {
    auto main_pin_state = Timer::PinState::Enable;
    auto complementary_pin_state = Timer::PinState::Disable;
    if constexpr (Timer::template isComplementaryChannel<Channel>()) {
      main_pin_state = Timer::PinState::Disable;
      complementary_pin_state = Timer::PinState::Enable;
    }

    Timer::template configureOutputChannel<Channel>(pwm_mode, main_pin_state,
                                                    Timer::OutputComparePolarity::ActiveHigh,
                                                    complementary_pin_state);

  } else {
    Timer::template configureOutputChannel<Channel>(pwm_mode, 0);
  }
}
