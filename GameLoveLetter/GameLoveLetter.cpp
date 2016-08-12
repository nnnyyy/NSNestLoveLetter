// GameLoveLetter.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Packet.h"
#include "Connection.h"
#include "Server.h"
#include <mysql.h>

int main()
{
	MYSQL *connection = NULL;
	MYSQL conn;
	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;

	if (mysql_init(&conn) == NULL) {
		return -1;
	}

	connection = mysql_real_connect(&conn, "localhost", "root", "s1980819", "game_love_letter", 3306, (const char*)NULL, 0);
	if (connection == NULL) {
		std::cout << mysql_errno(&conn) << "���� : " << mysql_error(&conn) << std::endl;
		return -1;
	}

	try {
		boost::asio::io_service io;
		boost::shared_ptr<Server> pServer = boost::shared_ptr<Server>(new Server(io));
		Server_Wrapper::get_mutable_instance().m_pServer = pServer;
		io.run();
	}	
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}

