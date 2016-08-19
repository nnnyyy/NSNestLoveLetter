#pragma once

class LogMan : public boost::serialization::singleton<LogMan> {
public:
	LogMan() {}
	~LogMan() {}

	void RunThread();
	void Run();

protected:
	boost::thread m_Thread;
};
