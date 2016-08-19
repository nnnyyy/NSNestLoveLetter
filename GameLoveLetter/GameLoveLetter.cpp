// GameLoveLetter.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Packet.h"
#include "User.h"
#include "Connection.h"
#include "Server.h"
#include "GameDealer.h"
#include "Room.h"
#include "MysqlMan.h"
#include "MinidumpHelp.h"
#include "LogMan.h"

MinidumpHelp g_miniDump;

class CThreadManager {
public:
	void handle_timer() {
		while (true) {
			CRoomManager::get_mutable_instance().Update();
			Sleep(10);
		}		
	}
};

int main()
{
	g_miniDump.install_self_mini_dump();

	if (!CMysqlManager::get_mutable_instance().Connect()) {
		return -1;
	}

	try {
		boost::asio::io_service io;
		boost::shared_ptr<Server> pServer = boost::shared_ptr<Server>(new Server(io));
		Server_Wrapper::get_mutable_instance().m_pServer = pServer;
		CThreadManager manager;
		boost::thread thread(boost::bind(&CThreadManager::handle_timer, &manager));		
		CLogMan::get_mutable_instance().RunThread();
		io.run();

		thread.join();
	}	
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}

