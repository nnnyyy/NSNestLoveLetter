#pragma once
#include <mysql.h>

class CMysqlManager : public boost::serialization::singleton<CMysqlManager> {
public:
	CMysqlManager() : m_bConnect(FALSE) {}
	~CMysqlManager() { mysql_close(&conn);  }

	BOOL Connect();
	BOOL IsConnect() const { return m_bConnect; }
	BOOL Query(std::string sQuery);

	LONG Login(std::string sID, std::string sPW, LONG& nSN);

protected:
	MYSQL *connection = NULL;
	MYSQL conn;
	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	BOOL m_bConnect;
};