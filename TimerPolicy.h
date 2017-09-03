#pragma once

class TimerPolicy
{
protected:
	TimerPolicy() = default;
public:
	const static int interval=3;
	enum class TimerStatus {
    ENABLE_TIMER,
    DISABLE_TIMER,
  };

  virtual TimerStatus GetStatus() = 0;
  virtual ~TimerPolicy() {}
};

class EnableTimerPolicy: public TimerPolicy
{

	TimerStatus GetStatus() override
	{
		return TimerStatus::ENABLE_TIMER;
	}

};

class DisableTimerPolicy: public TimerPolicy
{
	TimerStatus GetStatus() override
	{
		return TimerStatus::DISABLE_TIMER;
	}

};
