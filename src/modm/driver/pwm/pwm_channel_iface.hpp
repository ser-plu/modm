#pragma once

class IPwmChannel {
 public:
  virtual void Init(bool invert = false, uint32_t pwm_freq = kDefaultPwmFreq) = 0;
  virtual void SetDutyCycle(float duty_cycle) = 0;
  virtual void Stop() = 0;
  virtual void Start() = 0;
  virtual void SetInverted(bool invert) = 0;

  bool isInverted() const { return inverted_; }

  bool isRunning() const { return running_; }

  static constexpr uint16_t kDefaultPwmFreq = 10000;

 protected:
  bool inverted_ = false;
  bool running_ = false;
};

static constexpr uint16_t kMaxPwmFreqSupported = 24000;
static_assert(IPwmChannel::kDefaultPwmFreq < kMaxPwmFreqSupported,
              "Hardware does not support these frequencies.");
