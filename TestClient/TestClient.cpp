// TestClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define _BUFF_SIZE 64 
#define _MY_IP "127.0.0.1" 

using boost::asio::ip::tcp;

class CProtocol {
public:
	CProtocol(boost::asio::io_service& io) : m_Socket(io) {
	}

	~CProtocol() {}

	void Connect() {
		tcp::resolver resolver(m_Socket.get_io_service());
		tcp::resolver::query query(_MY_IP, "7770");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;

		boost::system::error_code err = boost::asio::error::host_not_found;

		while (err && endpoint_iterator != end) {
			m_Socket.close();
			m_Socket.connect(*endpoint_iterator++, err);
		}

		if (err) {
			throw boost::system::system_error(err);
		}

		m_bConnect = true;

		boost::array<CHAR, _BUFF_SIZE> buf;
		size_t len = m_Socket.read_some(boost::asio::buffer(buf), err);
		if (err == boost::asio::error::eof) {
			m_bConnect = false;
			return;
		}
		else if (err) {
			throw boost::system::system_error(err);
		}

		std::cout.write(buf.data(), len);
	}

	bool IsRun() { return m_bConnect;}
	bool IsSocketOpen() {
		if (!m_Socket.is_open() && m_bConnect) {
			m_bConnect = false;
			return false;
		}

		return true;
	}

	void handle_receive() {

	}

	void handle_send() {
		LONG packetSN = 0;
		while (m_bConnect)
		{
			if (!IsSocketOpen())
				break;

			try
			{
				boost::array<BYTE, _BUFF_SIZE> buf = { 0 };
				USHORT sPacketHeader = 62;
				*reinterpret_cast<USHORT*>(&buf[0]) = sPacketHeader;

				for (int i = 2; i < buf.size(); ++i) {
					buf[i] = packetSN;
				}
				boost::system::error_code error;
				int len = boost::asio::write(m_Socket, boost::asio::buffer(buf, buf.size()), error);
				if (len > 0) {
					std::cout << "writed" << std::endl;
					packetSN++;
				}
					
			}
			catch (std::exception& e)
			{
				m_bConnect = false;
				std::cerr << e.what() << std::endl;
			}

			Sleep(2000);
		}
	}

private:
	tcp::socket m_Socket;
	bool m_bConnect;
};

int main()
{
	try {
		boost::asio::io_service io;
		boost::asio::io_service::work work(io);

		CProtocol client(io);
		client.Connect();
		boost::thread Send(boost::bind( &CProtocol::handle_send, &client ));
		io.run();	//	하는 일이 있을 때만 Blocking. work(io)로 block 처리.

		Send.join();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}

