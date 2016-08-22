#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "LogMan.h"


void CLogMan::RunThread() {	
	m_bTerminate = FALSE;
	m_Semaphore = CreateSemaphore(NULL, 1, 3, NULL);
	SYSTEMTIME time;
	::GetLocalTime(&time);
	std::string sFileName = boost::str(boost::format("GameLog_%04d%02d%02d_%02d%02d%02d.log")
		% time.wYear % time.wMonth % time.wDay % time.wHour % time.wMinute % time.wSecond);
	fout.open(sFileName);
	fout << "Start" << std::endl;
	m_pThread = boost::shared_ptr<boost::thread>( new boost::thread(boost::bind(&CLogMan::Run, this)));
}

void CLogMan::Run() {
	while (!m_bTerminate) {
		::WaitForSingleObject(m_Semaphore, 3000);

		m_LockMutex.lock();
		for each (_LogInfo info in m_vLogs)
		{
			fout << info.sLog << std::endl;
		}
		m_vLogs.clear();
		m_LockMutex.unlock();
	}
}

void CLogMan::Add(std::string sLog) {
	boost::lock_guard<boost::mutex> lock(m_LockMutex);
	SYSTEMTIME time;
	::GetLocalTime(&time);
	std::string sTimePrefix = boost::str(boost::format("%04d%02d%02d %02d:%02d:%02d | ")
		% time.wYear % time.wMonth % time.wDay % time.wHour % time.wMinute % time.wSecond);	
	_LogInfo info;
	info.sLog = sTimePrefix + sLog;	
	std::cout << info.sLog << std::endl;
	m_vLogs.push_back(info);
}