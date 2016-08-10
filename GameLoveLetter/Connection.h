#pragma once

#define _BUFF_SIZE 128

using boost::asio::ip::tcp;

class CConnection : public boost::enable_shared_from_this<CConnection> {
private:
	tcp::socket m_Socket;
	int m_nID;

	boost::array<BYTE, _BUFF_SIZE> m_RecvBuf;
	std::string m_sMsg;

	CConnection(boost::asio::io_service& io) : m_Socket(io), m_nID(-1){}

	void handle_Accept(const boost::system::error_code& err, size_t byte_transferred);
	void handle_Read(const boost::system::error_code& err, size_t byte_transferred);

public:

	typedef boost::shared_ptr<CConnection> pointer;

	static pointer create(boost::asio::io_service& io) {
		return pointer(new CConnection(io));
	}

	tcp::socket& socket() {
		return m_Socket;
	}

	void start(int _nID);
};
