#pragma once

class CLogMan : public boost::serialization::singleton<CLogMan> {
public:
	CLogMan() {}
	~CLogMan() {}

	void RunThread();
	void Run();

	void Add(std::string sLog);

	struct _LogInfo {
		std::string sLog;		
	};

private:

	boost::shared_ptr<boost::thread> m_pThread;
	boost::mutex m_LockMutex;
	BOOL m_bTerminate;
	HANDLE m_Semaphore;	

	std::vector<_LogInfo> m_vLogs;
	std::ofstream fout;
};

__inline void LogAdd(std::string sLog) {	
	CLogMan::get_mutable_instance().Add(sLog);
}
