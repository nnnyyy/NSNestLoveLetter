// TestClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <conio.h>
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

class CContext : public boost::serialization::singleton<CContext> {
public:
	CContext() : m_uUserSN(0), m_bLogined(FALSE), m_nRoom(-1){}
	~CContext() {}

public:
	ULONG	m_uUserSN;
	BOOL	m_bLogined;

	LONG	m_nRoom;

	struct Player : public boost::enable_shared_from_this<Player> {
	public:
		Player() : m_bReady(FALSE), m_bGuard(FALSE), m_bDead(FALSE) {}
		ULONG m_uUserSN;
		BOOL m_bReady;
		BOOL m_bGuard;
		BOOL m_bDead;
		LONG m_nHandCardType;
		LONG m_nGroundCardType;
		typedef boost::shared_ptr<Player> pointer;
	};

	struct RoomInfo {
		LONG nSN;
		LONG nUserCnt;
		std::vector<Player::pointer> vPlayers;
		std::map<ULONG, Player::pointer> mPlayers;
		std::map<LONG, LONG> mIndex;

		void PrintInfo() {
			std::cout << "Room Sn : " << nSN << " User Cnt : " << nUserCnt << std::endl;
			for (int i = 0; i < vPlayers.size(); ++i) {
				std::cout << "[" << vPlayers[i]->m_uUserSN << "] - " << (vPlayers[i]->m_bReady ? "Ready" : "Not Ready") << std::endl;
			}
		}
	};

	boost::shared_ptr<RoomInfo> m_pRoom;

	void CreateRoom(LONG nSN) {
		boost::shared_ptr<RoomInfo> p(new RoomInfo());
		p->nSN = nSN;
		p->nUserCnt = 0;
		m_pRoom = p;
	}
};

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
					//std::cout << "Received : " << nByteTransfer << std::endl;
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
			OnLoginRet(iPacket);
			break;

		case GCP_CreateRoomRet:
			OnCreateRoomRet(iPacket);
			break;

		case GCP_EnterRoomRet:
			OnEnterRoomRet(iPacket);
			break;

		case GCP_RoomState:
			OnRoomState(iPacket);
			break;

		case GCP_LeaveRoomRet:
			OnLeaveRoomRet(iPacket);
			break;

		case GCP_GameStartRet:
			OnGameStartRet(iPacket);
			break;

		case GCP_GameLoveLetter:
			OnGameLoveLetter(iPacket);
			break;
		}
	}

	void OnLoginRet(InPacket& iPacket) {		
		LONG nRet = iPacket.Decode2();
		if (nRet == -1) {
			std::cout << "[Login] 이미 로그인 되어있습니다." << std::endl;
			return;
		}
		else if (nRet == 0) {
			std::cout << "[Login] 로그인 되었습니다." << std::endl;
			CContext::get_mutable_instance().m_bLogined = TRUE;
			CContext::get_mutable_instance().m_uUserSN = iPacket.Decode4();			
			return;
		}
		else {
			std::cout << "[Login] 알 수 없는 오류 입니다." << std::endl;
		}
	}

	void OnCreateRoomRet(InPacket& iPacket) {
		LONG nRet = iPacket.Decode2();
		if (nRet == -1) {
			std::cout << "[CreateRoom] 방 생성 오류" << std::endl;
		}
		else if (nRet == 0) {
			LONG nSN = iPacket.Decode4();
			std::cout << "[CreateRoom] 방 생성 - " << nSN << std::endl;
			CContext::get_mutable_instance().CreateRoom(nSN);
		}
	}

	void OnEnterRoomRet(InPacket& iPacket) {
		LONG nRet = iPacket.Decode2();
		if (nRet == 0) {
			LONG nSN = iPacket.Decode4();			
			CContext::get_mutable_instance().m_nRoom = nSN;
			CContext::get_mutable_instance().CreateRoom(nSN);
		}
	}

	void OnRoomState(InPacket& iPacket) {
		if (!CContext::get_mutable_instance().m_pRoom) {
			std::cout << "방에 입장해 있지 않습니다." << std::endl;
			return;
		}
		DWORD dwFlag = iPacket.Decode4();
		LONG nCnt = iPacket.Decode1();
		boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;
		pRoom->nUserCnt = nCnt;
		pRoom->vPlayers.clear();
		pRoom->mPlayers.clear();
		for (int i = 0; i < nCnt; ++i) {
			//	유저 정보들 ( 나중에는 플래그에 따라서 갱신 정보를 정한다 )	
			CContext::Player::pointer pPlayer(new CContext::Player());
			pRoom->vPlayers.push_back(pPlayer);			
			pPlayer->m_uUserSN = iPacket.Decode4();
			pPlayer->m_bReady = iPacket.Decode1();			
			pRoom->mPlayers.insert(std::pair< ULONG, CContext::Player::pointer >(pPlayer->m_uUserSN, pPlayer));
		}		

		pRoom->PrintInfo();
	}

	void OnLeaveRoomRet(InPacket& iPacket) {
		LONG nUserSN = iPacket.Decode4();
		if (CContext::get_mutable_instance().m_uUserSN == nUserSN) {
			CContext::get_mutable_instance().m_nRoom = -1;
			std::cout << "방에서 나왔습니다." << std::endl;
		}		
	}

	void OnGameStartRet(InPacket& iPacket) {
		LONG nRet = iPacket.Decode4();
		if (nRet == -1) {
			std::cout << "게임 인원이 부족합니다." << std::endl;
			return;
		}

		if (nRet == -2) {
			std::cout << "방장이 아닌 사람의 명령!" << std::endl;
			return;
		}

		if (nRet == 0) {
			std::cout << "게임 시작!" << std::endl;
			boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;			
			
			LONG nSize = iPacket.Decode4();
			for (int i = 0; i < nSize; ++i) {
				LONG nUserSN = iPacket.Decode4();
				LONG nIndex = iPacket.Decode4();
				pRoom->mIndex.insert(std::pair<LONG, LONG>(nUserSN, nIndex));
			}
			return;
		}
	}

	void OnGameLoveLetter(InPacket& iPacket) {
		LONG nSubType = iPacket.Decode2();
		switch (nSubType) {
		case GCP_LL_Status:
			OnLLStatus(iPacket);
			break;
		}
	}

	void OnLLStatus(InPacket& iPacket) {
		LONG nCurTurnIndex = iPacket.Decode4();
		LONG nUserCnt = iPacket.Decode4();
		boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;
		for (int i = 0; i < nUserCnt; ++i) {
			LONG nUserSN = iPacket.Decode4();
			BOOL bDead = iPacket.Decode4();
			BOOL bGuard = iPacket.Decode4();
			CContext::Player::pointer pPlayer = pRoom->mPlayers.at(nUserSN);
			pPlayer->m_bDead = bDead;
			pPlayer->m_bGuard = bGuard;
			LONG nGroundCardSize = iPacket.Decode4();
			if (nGroundCardSize) {
				LONG nLastGroundCardType = iPacket.Decode4();
				pPlayer->m_nGroundCardType = nLastGroundCardType;
			}
		}
		LONG nHandCardType = iPacket.Decode4();

		CContext::Player::pointer pPlayer = pRoom->mPlayers.at(CContext::get_mutable_instance().m_uUserSN);
		pPlayer->m_nHandCardType = nHandCardType;
		BOOL bMyTurn = (pRoom->mIndex.at(CContext::get_mutable_instance().m_uUserSN) == nCurTurnIndex);

		std::cout << "[LoveLetter Status] Player - " << pPlayer->m_nHandCardType << " , Turn : " << bMyTurn << std::endl;
	}

	void PrintRoomList() {
		if (m_vRoomInfo.size() <= 0) {
			std::cout << "생성된 방이 없습니다." << std::endl;
			return;
		}

		for (std::vector<RoomInfo>::iterator iter = m_vRoomInfo.begin(); iter != m_vRoomInfo.end(); ++iter) {
			std::cout << "[방 - " << (*iter).nSN << "] Count : " << (*iter).nUserCount << std::endl;
		}
	}

private:
	tcp::socket m_Socket;
	bool m_bConnect;
	InPacket packetBuf;
	boost::array<BYTE, _BUFF_SIZE> m_RecvBuf;
	struct RoomInfo {
		LONG nSN;
		LONG nUserCount;
	};
	std::vector<RoomInfo> m_vRoomInfo;
};

void PrintScreen();
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

			case 4: {
				std::cout << "게임 레디" << std::endl;
				OutPacket oPacket(CGP_GameReady);
				client.SendPacket(oPacket);
				
			}	
					break;

			case 5: {
				std::cout << "게임 시작" << std::endl;
				OutPacket oPacket(CGP_GameStart);
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