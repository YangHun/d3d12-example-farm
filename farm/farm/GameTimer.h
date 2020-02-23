#pragma once

class GameTimer
{
public:
	GameTimer();

	float Time() const;
	float DeltaTime() const { return (float)m_deltaTime; }

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double m_secondsPerCount;
	double m_deltaTime;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_currentTime;

	bool m_stopped;
};