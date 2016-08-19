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

#define LOCAL_DB_CONNECT

BOOL CMysqlManager::Connect() {

	try {
		sql::Driver *driver;
		driver = get_driver_instance();
#if defined(LOCAL_DB_CONNECT)
		conn = boost::shared_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1:3306", MYSQL_ID, MYSQL_PW));
#else
		conn = boost::shared_ptr<sql::Connection>(driver->connect("tcp://52.79.205.198:3306", MYSQL_ID, MYSQL_PW));
#endif
		
		conn->setSchema(MYSQL_DATABASE);
	}
	catch (sql::SQLException& e) {
		std::cout << "MySql Exception : " << e.getErrorCode() << std::endl;
	}
	

	return TRUE;
}


LONG CMysqlManager::Login(std::string sID, std::string sPW, LONG& nSN, std::string &sNick) {
	
	LONG nRet;
	try {
		stmt.reset(conn->createStatement());
		pstmt.reset(conn->prepareStatement("CALL Login(?,?,@nRet,@nSN, @sNick)"));
		pstmt->setString(1, sID);
		pstmt->setString(2, sPW);
		pstmt->execute();

		rs.reset(stmt->executeQuery("SELECT @nRet AS nRet, @nSN AS nSN, @sNick AS sNick"));
		while (rs->next()) {
			nRet = rs->getInt("nRet");
			nSN = rs->getInt("nSN");
			sNick = rs->getString("sNick");
		}
	}
	catch (sql::SQLException& e) {
		std::cout << "MySql Exception : " << e.getErrorCode() << std::endl;
		if(e.getErrorCode() == 2006) Connect();
		return -99;
	}

	return nRet;
}

LONG CMysqlManager::Logout(LONG nSN) {
	try{
		stmt.reset(conn->createStatement());
		pstmt.reset(conn->prepareStatement("CALL SetLoginStatus(?,0)"));
		pstmt->setInt(1, nSN);
		pstmt->execute();
	}
	catch (sql::SQLException& e) {
		std::cout << e.getErrorCode() << std::endl;
		if (e.getErrorCode() == 2006) Connect();
		return -99;
	}	

	return 0;
}