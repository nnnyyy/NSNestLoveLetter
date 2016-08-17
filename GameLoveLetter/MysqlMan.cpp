#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <algorithm>
#include <random>
#include <boost/format.hpp>
#include "PacketProtocol.h"
#include "Packet.h"
#include "User.h"
#include "MysqlMan.h"

#define MYSQL_SERVER_IP "localhost"
#define MYSQL_ID "root"
#define MYSQL_PW "s1980819"
#define MYSQL_DATABASE "game_love_letter"
#define MYSQL_LL_PORT (3306)

BOOL CMysqlManager::Connect() {
	if (mysql_init(&conn) == NULL) {
		return FALSE;
	}

	connection = mysql_real_connect(&conn, MYSQL_SERVER_IP, MYSQL_ID, MYSQL_PW, MYSQL_DATABASE, MYSQL_PORT, (const char*)NULL, 0);
	if (connection == NULL) {
		std::cout << mysql_errno(&conn) << "Error : " << mysql_error(&conn) << std::endl;
		return FALSE;
	}

	std::cout << "[Mysql] Connected" << std::endl;
	m_bConnect = TRUE;

	return TRUE;
}

BOOL CMysqlManager::Query(std::string sQuery) {
	if (!IsConnect()) {
		return FALSE;
	}

	if (mysql_query(&conn, sQuery.c_str())) {
		return FALSE;
	}

	sql_result = mysql_store_result(&conn);
	if (sql_result == NULL) {
		if (mysql_field_count(&conn) == NULL) {
			
		}
		else {
			std::cout << "Error : " << mysql_error(&conn) << std::endl;
		}
		return FALSE;
	}
	return TRUE;
}


LONG CMysqlManager::Login(std::string sID, std::string sPW, LONG& nSN) {
	std::string sQuery = boost::str(boost::format("SELECT SN FROM account WHERE ID='%s' AND PW= PASSWORD('%s')") % sID % sPW);
	if (!Query(sQuery)) {
		return -1;
	}

	sql_row = mysql_fetch_row(sql_result);
	if (sql_row) {
		nSN = atoi(sql_row[0]);
		return 0;
	}

	return -1;
}