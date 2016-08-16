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
#define _AWS_IP "52.79.205.198" 
#define _PORT "7770"

class CProtocol;
void PrintMenu();
void PrintActionLog();
void ProcMenu(CProtocol& client, LONG n);
void ProcGameMenu(CProtocol& client, LONG n);
void ProcCardCommand(CProtocol& client, LONG n);

std::vector< std::string> g_vMessages;
void AddMessage(std::string sMsg) {
	g_vMessages.push_back(sMsg);
}

using boost::asio::ip::tcp;

std::vector<BYTE> buf;
boost::mutex g_mutex;

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
		Player() : m_bReady(FALSE), m_bGuard(FALSE), m_bDead(FALSE), m_nIndex(-1) {}
		ULONG m_uUserSN;
		BOOL m_bReady;
		BOOL m_bGuard;
		BOOL m_bDead;
		LONG m_nIndex;
		std::vector<LONG> m_vHandCardType;
		std::vector<LONG> m_vGroundCardType;
		typedef boost::shared_ptr<Player> pointer;
	};

	struct RoomInfo {
		RoomInfo() : m_bGameStart(FALSE) {}
		LONG nSN;
		LONG nUserCnt;
		LONG nCurTurnIndex;
		std::vector<Player::pointer> vPlayers;
		Player::pointer pLocalPlayer;
		std::map<ULONG, Player::pointer> mPlayers;
		std::map<LONG, LONG> mSNToIndex;	//	UserSN to Idx
		std::map<LONG, LONG> mIndexToSN;	//	Idx to UserSN  
		BOOL m_bGameStart;

		BOOL IsMyTurn() {
			if (!pLocalPlayer) return FALSE;
			return (nCurTurnIndex == pLocalPlayer->m_nIndex);
		}

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
			if (IsMyTurn()) {
				std::cout << "[턴]";
			}
			std::cout << "[" << pLocalPlayer->m_nIndex << "]";
			if (pLocalPlayer->m_vHandCardType.size() >= 2) {
				std::cout << (pLocalPlayer->m_bDead ? "#Dead#" : "") << "[My Hand Card] " << pLocalPlayer->m_vHandCardType[0] << " ," << pLocalPlayer->m_vHandCardType[1] << std::endl;
			}
			else {
				std::cout << (pLocalPlayer->m_bDead ? "#Dead#" : "") << "[My Hand Card] " << pLocalPlayer->m_vHandCardType[0] << std::endl;
			}
			
			for (std::vector<Player::pointer>::iterator iter = vPlayers.begin(); iter != vPlayers.end(); ++iter) {
				Player::pointer player = *iter;					
				if (player == pLocalPlayer) continue;
				std::cout << (player->m_bDead ? "#Dead#" : "");
				std::cout << "[" << player->m_nIndex << "]";
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
		//tcp::resolver::query query(_AWS_IP, _PORT);	//	실제 서비스 할 서버
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
			std::cout << "[Login] 이미 로그인 되어있습니다." << std::endl;
			return;
		}
		else if (nRet == 0) {
			std::cout << "[Login] 로그인 되었습니다." << std::endl;
			CContext::get_mutable_instance().m_bLogined = TRUE;
			CContext::get_mutable_instance().m_uUserSN = iPacket.Decode4();	

			OutPacket oPacket(CGP_RoomListRequest);
			SendPacket(oPacket);

			return;
		}
		else {
			std::cout << "[Login] 알 수 없는 오류 입니다." << std::endl;
		}
	}

	void OnRoomListRet(InPacket& iPacket) {
		std::cout << "## 게임 방 리스트 ##" << std::endl;
		LONG nRoomCnt = iPacket.Decode4();
		for (int i = 0; i < nRoomCnt; ++i) {
			LONG nRoomSN = iPacket.Decode4();
			LONG nUserCnt = iPacket.Decode4();
			std::cout << "[방번호 : " << nRoomSN << "] - 인원 : " << nUserCnt << std::endl;
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

		if (dwFlag & 0x80000000) {
			ULONG uUserSN = iPacket.Decode4();
			std::cout << "새로운 방장 : " << uUserSN << std::endl;
			return;
		}

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
			CContext::get_mutable_instance().m_pRoom = NULL;
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
			
			pRoom->m_bGameStart = TRUE;
			LONG nSize = iPacket.Decode4();
			for (int i = 0; i < nSize; ++i) {
				LONG nUserSN = iPacket.Decode4();
				LONG nIndex = iPacket.Decode4();
				pRoom->mSNToIndex.insert(std::pair<LONG, LONG>(nUserSN, nIndex));
				pRoom->mIndexToSN.insert(std::pair<LONG, LONG>(nIndex, nUserSN));
				pRoom->mPlayers.at(nUserSN)->m_nIndex = nIndex;
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

		case GCP_LL_ActionRet:
			OnActionRet(iPacket);
			break;
		}
	}

	void OnLLStatus(InPacket& iPacket) {
		LONG nCurTurnIndex = iPacket.Decode4();
		LONG nUserCnt = iPacket.Decode4();
		boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;
		pRoom->nCurTurnIndex = nCurTurnIndex;
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

	void OnActionRet(InPacket& iPacket) {
		boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;
		LONG nCardType = iPacket.Decode4();
		switch (nCardType) {
		case 1:
			{
				//	경비병 사용 결과
				BOOL bSucceed = iPacket.Decode4();
				if (bSucceed) {
					LONG nIdxDead = iPacket.Decode4();
					LONG nUserSNDead = pRoom->mIndexToSN[nIdxDead];
					AddMessage("경비병 사용 성공");
				}
				else {
					AddMessage("경비병 사용 실패");
				}
			}
			break;		
		case 2:
			{
				//	신하 사용 결과
				BOOL bSucceed = iPacket.Decode4();
				if (bSucceed) {
					LONG nCardType = iPacket.Decode4();	
					std::string sMsg = boost::str(boost::format("상대방은 %d 카드 입니다.") % nCardType);
					AddMessage(sMsg);
				}
			}
			break;
		}				
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
	boost::lock_guard<boost::mutex> lock(g_mutex);
	system("cls");	
	if (CContext::get_mutable_instance().IsGameRunning()) {
		PrintActionLog();
		CContext::get_mutable_instance().m_pRoom->PrintMyCardAndAllGroundCard();
		std::cout << "[메뉴] 0-카드 사용" << std::endl;
	}
	else {
		if (!CContext::get_mutable_instance().m_bLogined) {
			std::cout << "[메뉴] 0-로그인" << std::endl;
			return;
		}

		if (CContext::get_mutable_instance().m_pRoom) {
			CContext::get_mutable_instance().m_pRoom->PrintInfo();
			std::cout << "[메뉴] 3-방퇴장, 4-게임레디, 5-게임시작" << std::endl;
		}
		else {
			std::cout << "[메뉴] 1-방만들기, 2-방입장" << std::endl;
		}
	}
}

void PrintActionLog() {
	if (g_vMessages.size() < 3) {
		for each (std::string s in g_vMessages)
		{
			std::cout << s << std::endl;
		}
	}
	else {
		for (int i = g_vMessages.size() - 1; (g_vMessages.size() - 3) > i; --i) {
			std::cout << g_vMessages[i] << std::endl;
		}
	}

	std::cout << "" << std::endl;
}

void ProcGameMenu(CProtocol& client, LONG n) {
	char line[128 + 1];
	switch (n) {
		case 0: {			
			std::cout << "어떤 카드를 내시겠소 : ";
			std::cin.getline(line, 128);			
			int n2 = atoi(line);
			ProcCardCommand(client, n2);			
		}
				break;		
	}
}

void ProcCardCommand(CProtocol& client, LONG n) {	
	char line[128 + 1];
	CContext::Player::pointer pLocalPlayer = CContext::get_mutable_instance().m_pRoom->pLocalPlayer;
	std::vector<LONG>::iterator iter = pLocalPlayer->m_vHandCardType.begin();
	BOOL bFind = FALSE;
	for (; iter != pLocalPlayer->m_vHandCardType.end(); ++iter) {
		if (*iter == n) {
			bFind = TRUE;
			break;
		}
	}
	if (!bFind) {		
		return;
	}

	if (n == 1) {
		//	경비병 카드 사용 ( 누구에게 무엇이냐고 물어봐야함 )
		LONG nWho;
		LONG nWhat;
		std::cout << "[경비] 누구에게? : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);
		std::cout << "[경비] 무엇인 것 같소 : ";
		std::cin.getline(line, 128);
		nWhat = atoi(line);
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_GuardCheck);
		oPacket.Encode4(nWho);
		oPacket.Encode4(nWhat);
		client.SendPacket(oPacket);
	}

	if (n == 2) {
		//	왕실신하 카드 사용 ( 누가 무엇이냐고 물어봐야함 )
		LONG nWho;		
		std::cout << "[신하] 누구에게 물어보겠소? : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);		
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_RoyalSubject);
		oPacket.Encode4(nWho);		
		client.SendPacket(oPacket);
	}

	if (n == 3) {
		//	험담가 사용 ( 누구와 싸울 것이냐고 물어 봄 )
		LONG nWho;
		std::cout << "[험담가] 누구와 싸우겠소? : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Gossip);
		oPacket.Encode4(nWho);
		client.SendPacket(oPacket);
	}

	if (n == 4) {
		//	동료 사용						
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Companion);
		client.SendPacket(oPacket);
	}

	if (n == 5) {
		//	영웅 사용 ( 누구에게 쓰겠소 )
		LONG nWho;
		std::cout << "[영웅] 누구의 카드를 버리게 하겠소?(자신 포함) : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Hero);
		oPacket.Encode4(nWho);
		client.SendPacket(oPacket);
	}

	if (n == 6) {
		//	마법사 사용 ( 누구에게 쓰겠소 )
		LONG nWho;
		std::cout << "[마법사] 누구와 카드를 교환하겠소? : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Wizard);
		oPacket.Encode4(nWho);
		client.SendPacket(oPacket);
	}

	if (n == 7) {
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Lady);
		client.SendPacket(oPacket);
	}

	if (n == 8) {
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Princess);
		client.SendPacket(oPacket);
	}
}

void ProcMenu(CProtocol& client, LONG n) {
	char line[128 + 1];
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
	}
}