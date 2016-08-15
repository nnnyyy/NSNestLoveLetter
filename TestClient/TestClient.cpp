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
#include "Packet.h"
#include "../GameLoveLetter/PacketProtocol.h"

#define _BUFF_SIZE 128 
#define _MY_IP "127.0.0.1" 

using boost::asio::ip::tcp;

std::vector<BYTE> buf;

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
		while (m_bConnect) {
			if (!IsSocketOpen())
				break;
			boost::system::error_code err = boost::asio::error::host_not_found;

			try {								
				size_t nByteTransfer = m_Socket.read_some(boost::asio::buffer(m_RecvBuf), err);
				if (!err) {
					std::cout << "Received : " << nByteTransfer << std::endl;
					if (m_RecvBuf.size()) {
						BYTE *pBuf = &m_RecvBuf[0];

						while (pBuf) {
							LONG nState = packetBuf.Append(pBuf, nByteTransfer);
							if (nState == InPacket::PS_COMPLETE) {
								ProcessPacket(packetBuf);
								packetBuf.Clear();
							}
						}
					}
				}
				else {
					std::cout << "Close() - " << err.message() << std::endl;
					m_bConnect = false;
				}				
			}
			catch (std::exception e) {
				m_bConnect = false;
				std::cerr << e.what() << std::endl;
			}

			Sleep(1);
		}
	}

	void handle_send() {
		LONG packetSN = 0;
		while (m_bConnect)
		{
			if (!IsSocketOpen())
				break;

			try
			{
				switch (packetSN) {
				case 0: 
				{
					OutPacket oPacket(CGP_Login);
					SendPacket(oPacket);
				}					
				break;

				case 1:
				{
					OutPacket oPacket(CGP_CreateRoom);
					SendPacket(oPacket);
				}
				break;
				}
				
				packetSN++;
			}
			catch (std::exception& e)
			{
				m_bConnect = false;
				std::cerr << e.what() << std::endl;
			}

			Sleep(10000);
		}
	}

	void handle_Write(const boost::system::error_code& err, size_t byte_transferred) {

	}

	void SendPacket(OutPacket& oPacket) {
		oPacket.MakeBuf(buf);
		boost::asio::async_write(m_Socket, boost::asio::buffer(buf),
			boost::bind(&CProtocol::handle_Write, this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
				));
	}

	void ProcessPacket(InPacket& iPacket) {
		LONG nType = iPacket.Decode2();
		switch (nType) {
		case GCP_LoginRet:
			std::cout << "Logined" << std::endl;
			break;

		case GCP_RoomInfoRet:
			std::cout << "RoomInfoRet" << std::endl;
			break;
		}
	}

private:
	tcp::socket m_Socket;
	bool m_bConnect;
	InPacket packetBuf;
	boost::array<BYTE, _BUFF_SIZE> m_RecvBuf;
};

int main()
{
	try {
		boost::asio::io_service io;
		boost::asio::io_service::work work(io);

		CProtocol client(io);
		client.Connect();
		//boost::thread Send(boost::bind( &CProtocol::handle_send, &client ));
		boost::thread Receive(boost::bind(&CProtocol::handle_receive, &client));
		boost::thread mainIO(boost::bind(&boost::asio::io_service::run, &io));
		//io.run();	//	하는 일이 있을 때만 Blocking. work(io)로 block 처리.

		char line[128 + 1];
		while (std::cin.getline(line, 128))
		{
			int n = atoi(line);
			switch (n) {
			case 0: {
				std::cout << "로그인 시도" << std::endl;
				OutPacket oPacket(CGP_Login);
				client.SendPacket(oPacket);
			}	
				break;
			case 1: {
				std::cout << "방 만들기 시도" << std::endl;
				OutPacket oPacket(CGP_CreateRoom);
				client.SendPacket(oPacket);			
				//	방 생성
			}
				break;

			case 2: {				
				std::cout << "방 입장 시도" << std::endl;
				std::cout << "몇번방 접속 : " << std::endl;
				std::cin.getline(line, 128);
				int n2 = atoi(line);
				OutPacket oPacket(CGP_EnterRoom);
				oPacket.Encode4(n2);
				client.SendPacket(oPacket);
			}
				break;

			case 3: {
				std::cout << "방 떠나기 시도" << std::endl;
				OutPacket oPacket(CGP_LeaveRoom);				
				client.SendPacket(oPacket);
			}
					break;
			default:
				std::cout << "잘못된 명령어" << std::endl;
				continue;
			}

		}

		//Send.join();
		Receive.join();

		mainIO.join();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}

