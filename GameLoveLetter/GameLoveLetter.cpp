// GameLoveLetter.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
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
	if (!CMysqlManager::get_mutable_instance().Connect()) {
		return -1;
	}	

	LONG nSN;
	CMysqlManager::get_mutable_instance().Login("nnnyyy", "s1980819", nSN);	

	try {
		boost::asio::io_service io;
		boost::shared_ptr<Server> pServer = boost::shared_ptr<Server>(new Server(io));
		Server_Wrapper::get_mutable_instance().m_pServer = pServer;
		CThreadManager manager;
		boost::thread thread(boost::bind(&CThreadManager::handle_timer, &manager));
		io.run();

		thread.join();
	}	
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}

