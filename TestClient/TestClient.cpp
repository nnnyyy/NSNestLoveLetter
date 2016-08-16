// TestClient.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
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
#define _AWS_IP "52.79.205.198" 
#define _PORT "7770"

class CProtocol;
void PrintMenu();
void ProcMenu(CProtocol& client, LONG n);
void ProcGameMenu(CProtocol& client, LONG n);

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
		std::vector<LONG> m_vHandCardType;
		std::vector<LONG> m_vGroundCardType;
		typedef boost::shared_ptr<Player> pointer;
	};

	struct RoomInfo {
		RoomInfo() : m_bGameStart(FALSE) {}
		LONG nSN;
		LONG nUserCnt;
		std::vector<Player::pointer> vPlayers;
		Player::pointer pLocalPlayer;
		std::map<ULONG, Player::pointer> mPlayers;
		std::map<LONG, LONG> mIndex;
		BOOL m_bGameStart;

		void PrintInfo() {
			std::cout << "Me" << CContext::get_mutable_instance().m_uUserSN << std::endl;
			std::cout << "Room Sn : " << nSN << " User Cnt : " << nUserCnt << std::endl;
			for (int i = 0; i < vPlayers.size(); ++i) {
				BOOL bLocal = (CContext::get_mutable_instance().m_uUserSN == vPlayers[i]->m_uUserSN);
				if (bLocal) {
					std::cout << "Me [" << vPlayers[i]->m_uUserSN << "] - " << (vPlayers[i]->m_bReady ? "Ready" : "Not Ready") << std::endl;
				}	
				else {
					std::cout << "[" << vPlayers[i]->m_uUserSN << "] - " << (vPlayers[i]->m_bReady ? "Ready" : "Not Ready") << std::endl;
				}
			}
		}

		void PrintMyCardAndAllGroundCard() {
			if (!pLocalPlayer) return;
			if (pLocalPlayer->m_vHandCardType.size() >= 2) {
				std::cout << "[My Hand Card] " << pLocalPlayer->m_vHandCardType[0] << " ," << pLocalPlayer->m_vHandCardType[1] << std::endl;
			}
			else {
				std::cout << "[My Hand Card] " << pLocalPlayer->m_vHandCardType[0] << std::endl;
			}
			
			for (std::vector<Player::pointer>::iterator iter = vPlayers.begin(); iter != vPlayers.end(); ++iter) {
				Player::pointer player = *iter;					
				if (player == pLocalPlayer) continue;
				std::cout << "[" << player->m_uUserSN << "] ";
				if (player->m_vGroundCardType.size()) {
					LONG nCardType = player->m_vGroundCardType.back();
					std::cout << "Last Ground Card : " << nCardType << std::endl;
				}
				else {
					std::cout << std::endl;
				}
				
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

	BOOL IsGameRunning() {	
		if(!m_pRoom) return FALSE;

		return m_pRoom->m_bGameStart;
	}
};

class CProtocol {
public:
	CProtocol(boost::asio::io_service& io) : m_Socket(io) {
	}

	~CProtocol() {}

	void Connect() {
		tcp::resolver resolver(m_Socket.get_io_service());
		tcp::resolver::query query(_MY_IP, _PORT);
		//tcp::resolver::query query(_AWS_IP, _PORT);	//	���� ���� �� ����
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

		case GCP_RoomListRet:
			OnRoomListRet(iPacket);
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
		PrintMenu();
	}

	void OnLoginRet(InPacket& iPacket) {		
		LONG nRet = iPacket.Decode2();
		if (nRet == -1) {
			std::cout << "[Login] �̹� �α��� �Ǿ��ֽ��ϴ�." << std::endl;
			return;
		}
		else if (nRet == 0) {
			std::cout << "[Login] �α��� �Ǿ����ϴ�." << std::endl;
			CContext::get_mutable_instance().m_bLogined = TRUE;
			CContext::get_mutable_instance().m_uUserSN = iPacket.Decode4();	

			OutPacket oPacket(CGP_RoomListRequest);
			SendPacket(oPacket);

			return;
		}
		else {
			std::cout << "[Login] �� �� ���� ���� �Դϴ�." << std::endl;
		}
	}

	void OnRoomListRet(InPacket& iPacket) {
		std::cout << "## ���� �� ����Ʈ ##" << std::endl;
		LONG nRoomCnt = iPacket.Decode4();
		for (int i = 0; i < nRoomCnt; ++i) {
			LONG nRoomSN = iPacket.Decode4();
			LONG nUserCnt = iPacket.Decode4();
			std::cout << "[���ȣ : " << nRoomSN << "] - �ο� : " << nUserCnt << std::endl;
		}
	}

	void OnCreateRoomRet(InPacket& iPacket) {
		LONG nRet = iPacket.Decode2();
		if (nRet == -1) {
			std::cout << "[CreateRoom] �� ���� ����" << std::endl;
		}
		else if (nRet == 0) {
			LONG nSN = iPacket.Decode4();
			std::cout << "[CreateRoom] �� ���� - " << nSN << std::endl;
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
			std::cout << "�濡 ������ ���� �ʽ��ϴ�." << std::endl;
			return;
		}
		DWORD dwFlag = iPacket.Decode4();

		if (dwFlag & 0x80000000) {
			ULONG uUserSN = iPacket.Decode4();
			std::cout << "���ο� ���� : " << uUserSN << std::endl;
			return;
		}

		LONG nCnt = iPacket.Decode1();
		boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;
		pRoom->nUserCnt = nCnt;
		pRoom->vPlayers.clear();
		pRoom->mPlayers.clear();
		for (int i = 0; i < nCnt; ++i) {
			//	���� ������ ( ���߿��� �÷��׿� ���� ���� ������ ���Ѵ� )	
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
			std::cout << "�濡�� ���Խ��ϴ�." << std::endl;
		}		
	}

	void OnGameStartRet(InPacket& iPacket) {
		LONG nRet = iPacket.Decode4();
		if (nRet == -1) {
			std::cout << "���� �ο��� �����մϴ�." << std::endl;
			return;
		}

		if (nRet == -2) {
			std::cout << "������ �ƴ� ����� ���!" << std::endl;
			return;
		}

		if (nRet == 0) {
			std::cout << "���� ����!" << std::endl;
			boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;			
			
			pRoom->m_bGameStart = TRUE;
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
			pPlayer->m_vGroundCardType.clear();
			if (nGroundCardSize) {				
				for (int i = 0; i < nGroundCardSize; ++i) {
					LONG nCardType = iPacket.Decode4();
					pPlayer->m_vGroundCardType.push_back(nCardType);
				}				
			}
		}

		CContext::Player::pointer pPlayer = pRoom->mPlayers.at(CContext::get_mutable_instance().m_uUserSN);
		pPlayer->m_vHandCardType.clear();
		BOOL bMyTurn = iPacket.Decode4();
		LONG nHandCardType = iPacket.Decode4();
		pPlayer->m_vHandCardType.push_back(nHandCardType);
		LONG nNewHandCardType;
		if (bMyTurn) {
			nNewHandCardType = iPacket.Decode4();
			pPlayer->m_vHandCardType.push_back(nNewHandCardType);
		}
		
		pRoom->pLocalPlayer = pPlayer;		
	}

	void PrintRoomList() {
		if (m_vRoomInfo.size() <= 0) {
			std::cout << "������ ���� �����ϴ�." << std::endl;
			return;
		}

		for (std::vector<RoomInfo>::iterator iter = m_vRoomInfo.begin(); iter != m_vRoomInfo.end(); ++iter) {
			std::cout << "[�� - " << (*iter).nSN << "] Count : " << (*iter).nUserCount << std::endl;
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
		//io.run();	//	�ϴ� ���� ���� ���� Blocking. work(io)�� block ó��.

		PrintMenu();
		char line[128 + 1];
		while (std::cin.getline(line, 128))
		{
			int n = atoi(line);
			if (CContext::get_mutable_instance().IsGameRunning()) {
				ProcGameMenu(client, n);
			}
			else {
				ProcMenu(client, n);
			}			

			PrintMenu();

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

void PrintMenu() {
	system("cls");
	if (CContext::get_mutable_instance().IsGameRunning()) {
		CContext::get_mutable_instance().m_pRoom->PrintMyCardAndAllGroundCard();
		std::cout << "[�޴�] 0-ī�� Ȯ�� 1-ī�� ���" << std::endl;
	}
	else {
		if (!CContext::get_mutable_instance().m_bLogined) {
			std::cout << "[�޴�] 0-�α���" << std::endl;
			return;
		}

		if (CContext::get_mutable_instance().m_pRoom) {
			CContext::get_mutable_instance().m_pRoom->PrintInfo();
			std::cout << "[�޴�] 3-������, 4-���ӷ���, 5-���ӽ���" << std::endl;
		}
		else {
			std::cout << "[�޴�] 1-�游���, 2-������" << std::endl;
		}
	}
}

void ProcGameMenu(CProtocol& client, LONG n) {
	switch (n) {
		case 0: {
			std::cout << "Game Menu0" << std::endl;
		}
				break;

		case 1: {
			std::cout << "Game Menu1" << std::endl;
		}
				break;
	}
}

void ProcMenu(CProtocol& client, LONG n) {
	char line[128 + 1];
	switch (n) {
	case 0: {
		std::cout << "�α��� �õ�" << std::endl;
		OutPacket oPacket(CGP_Login);
		client.SendPacket(oPacket);
	}
			break;
	case 1: {
		std::cout << "�� ����� �õ�" << std::endl;
		OutPacket oPacket(CGP_CreateRoom);
		client.SendPacket(oPacket);
		//	�� ����
	}
			break;

	case 2: {
		std::cout << "�� ���� �õ�" << std::endl;
		std::cout << "����� ���� : " << std::endl;
		std::cin.getline(line, 128);
		int n2 = atoi(line);
		OutPacket oPacket(CGP_EnterRoom);
		oPacket.Encode4(n2);
		client.SendPacket(oPacket);
	}
			break;

	case 3: {
		std::cout << "�� ������ �õ�" << std::endl;
		OutPacket oPacket(CGP_LeaveRoom);
		client.SendPacket(oPacket);
	}
			break;

	case 4: {
		std::cout << "���� ����" << std::endl;
		OutPacket oPacket(CGP_GameReady);
		client.SendPacket(oPacket);

	}
			break;

	case 5: {
		std::cout << "���� ����" << std::endl;
		OutPacket oPacket(CGP_GameStart);
		client.SendPacket(oPacket);
	}
			break;

	default:
		std::cout << "�߸��� ��ɾ�" << std::endl;
	}
}