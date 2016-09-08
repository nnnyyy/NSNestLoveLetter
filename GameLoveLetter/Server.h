#pragma once

using boost::asio::ip::tcp;

class Server;
class CUser;

class Server_Wrapper : public boost::serialization::singleton<Server_Wrapper> {
public:
	Server_Wrapper() {}
	~Server_Wrapper() {}

	boost::shared_ptr<Server> m_pServer;

	static std::map<ULONG, boost::shared_ptr<CUser> > m_mUsers;

	void Update();
};

class Server {	
public:	
	Server(boost::asio::io_service& io);

public:
	std::map<ULONG, CConnection::pointer> m_mSockets;
	std::list<CConnection::pointer> m_lSockets;

private:
	tcp::acceptor m_Acceptor;
	ULONG m_dwSocketSNCounter;
	int m_nAcceptCnt;

	void WaitAccept();
	void handle_Accept(CConnection::pointer newConn, const boost::system::error_code& err);

public:

	void RemoveSocket( CConnection::pointer pSocket );
	CConnection::pointer GetSocket(ULONG _uSocketSN);
	void Update();
	boost::mutex m_LockMutex;
};