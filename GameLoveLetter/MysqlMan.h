#pragma once
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <mysql.h>

class CMysqlManager : public boost::serialization::singleton<CMysqlManager> {
public:
	CMysqlManager() : m_bConnect(FALSE) {}
	~CMysqlManager() { }

	BOOL Connect();
	BOOL IsConnect() const { return m_bConnect; }	

	LONG Login(std::string sID, std::string sPW, LONG& nSN, std::string &sNick);
	LONG Logout(LONG nSN);
	LONG GetGameDataFromDB(LONG nSN, CGameData& _data);
	LONG SetGameDataFromDB(LONG nSN, CGameData& _data);

protected:
	boost::shared_ptr<sql::Connection> conn;
	boost::shared_ptr<sql::PreparedStatement> pstmt;
	boost::shared_ptr<sql::ResultSet> rs;
	boost::shared_ptr<sql::Statement> stmt;
	BOOL m_bConnect;
};