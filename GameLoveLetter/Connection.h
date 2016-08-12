#pragma once

#define _BUFF_SIZE 128

using boost::asio::ip::tcp;

class CConnection : public boost::enable_shared_from_this<CConnection> {
private:
	tcp::socket m_Socket;
public:
	ULONG m_uSocketSN;
private:
	boost::array<BYTE, _BUFF_SIZE> m_RecvBuf;	
	std::string m_sMsg;
	std::vector<BYTE> v;
	InPacket packetBuf;

	CConnection(boost::asio::io_service& io) : m_Socket(io), m_uSocketSN(-1){		
	}

	void handle_Accept(const boost::system::error_code& err, size_t byte_transferred);
	void handle_Read(const boost::system::error_code& err, size_t byte_transferred);
	void handle_Write(const boost::system::error_code& err, size_t byte_transferred);

public:

	typedef boost::shared_ptr<CConnection> pointer;

	static pointer create(boost::asio::io_service& io) {
		return pointer(new CConnection(io));
	}

	tcp::socket& socket() {
		return m_Socket;
	}

	void start();

private:

	void ProcessPacket(InPacket &iPacket);
	void SendPacket(OutPacket &oPacket);

private:
	
	std::string m_sID;
	std::string m_sPW;


	void OnLogin(InPacket &iPacket);
};
