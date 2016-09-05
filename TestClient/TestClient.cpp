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

//#define LOCAL_CONNECT

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

using boost::asio::ip::tcp;

std::vector<BYTE> buf;
boost::mutex g_mutex;

std::vector< std::string> g_vMessages;
void AddMessage(std::string sMsg) {
	boost::lock_guard<boost::mutex> lock(g_mutex);
	g_vMessages.push_back(sMsg);
}

class CContext : public boost::serialization::singleton<CContext> {
public:
	CContext() : m_uUserSN(0), m_bLogined(FALSE), m_nRoom(-1){}
	~CContext() {}

public:
	ULONG	m_uUserSN;
	std::string m_sNick;
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
		std::string m_sNick;
		std::vector<LONG> m_vHandCardType;
		std::vector<LONG> m_vGroundCardType;
		typedef boost::shared_ptr<Player> pointer;
	public:
		void DropCard(LONG nCardIdx) {
			std::vector<LONG>::iterator iter = m_vHandCardType.begin();
			for (; iter != m_vHandCardType.end(); ++iter) {
				if (*iter == nCardIdx) {
					m_vHandCardType.erase(iter);
					break;
				}
			}

			m_vGroundCardType.push_back(nCardIdx);
		}

		void PutCardToHand(LONG nCardIdx) {
			m_vHandCardType.push_back(nCardIdx);
		}

		void SendCard(Player::pointer pTargetPlayer, LONG nCardIdx) {
			std::vector<LONG>::iterator iter = m_vHandCardType.begin();			
			for (; iter != m_vHandCardType.end(); ++iter) {
				if (*iter == nCardIdx) {
					m_vHandCardType.erase(iter);
					break;
				}
			}

			pTargetPlayer->PutCardToHand(nCardIdx);
		}
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

		Player::pointer GetPlayer(LONG nUserIdx) {
			LONG sn = mIndexToSN[nUserIdx];
			return mPlayers[sn];
		}

		BOOL IsMyTurn() {
			if (!pLocalPlayer) return FALSE;
			return (nCurTurnIndex == pLocalPlayer->m_nIndex);
		}

		void PrintInfo() {
			std::cout << "Me " << CContext::get_mutable_instance().m_uUserSN << std::endl;
			std::cout << "Room Sn : " << nSN << " User Cnt : " << nUserCnt << std::endl;
			for (int i = 0; i < vPlayers.size(); ++i) {
				BOOL bLocal = (CContext::get_mutable_instance().m_uUserSN == vPlayers[i]->m_uUserSN);
				if (bLocal) {
					std::cout << "Me [" << vPlayers[i]->m_sNick << "] - " << (vPlayers[i]->m_bReady ? "Ready" : "Not Ready") << std::endl;
				}	
				else {
					std::cout << "[" << vPlayers[i]->m_sNick << "] - " << (vPlayers[i]->m_bReady ? "Ready" : "Not Ready") << std::endl;
				}
			}
		}

		void PrintMyCardAndAllGroundCard() {
			if (!pLocalPlayer) return;
			std::cout << "--------------------------------------" << std::endl;
			if (IsMyTurn()) {
				std::cout << "####���� ���Դϴ�####" << std::endl;
			}
			std::cout << "[" << pLocalPlayer->m_nIndex << "]";
			if (pLocalPlayer->m_vHandCardType.size() >= 2) {
				std::cout << (pLocalPlayer->m_bGuard ? "<Shield>" : "") << (pLocalPlayer->m_bDead ? "#Dead#" : "") << "[My Hand Card] " << pLocalPlayer->m_vHandCardType[0] << " ," << pLocalPlayer->m_vHandCardType[1] << std::endl;
			}
			else {
				std::cout << (pLocalPlayer->m_bGuard ? "<Shield>" : "") << "[My Hand Card] " << (pLocalPlayer->m_bDead ? -1 : pLocalPlayer->m_vHandCardType[0]) << std::endl;
			}
			if (pLocalPlayer->m_vGroundCardType.size()) {
				std::cout << "[�ٴ� ī��] ";
				for each (LONG _nCardType in pLocalPlayer->m_vGroundCardType)
				{
					std::cout << " " << _nCardType << " ";
				}

				std::cout << std::endl;
			}
			std::cout << "--------------------------------------" << std::endl;
			
			for (std::vector<Player::pointer>::iterator iter = vPlayers.begin(); iter != vPlayers.end(); ++iter) {
				Player::pointer player = *iter;					
				if (player == pLocalPlayer) continue;
				std::cout << (player->m_bGuard ? "<Shield> " : "");
				std::cout << (player->m_bDead ? "#Dead# " : "");
				std::cout << "[" << player->m_nIndex << "]";
				std::cout << "[" << player->m_sNick << "] ";
				if (player->m_vGroundCardType.size()) {
					std::cout << "[�ٴ� ī��] ";
					for each (LONG _nCardType in player->m_vGroundCardType)
					{
						std::cout << " " << _nCardType << " ";
					}
					std::cout << std::endl;
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
#if defined(LOCAL_CONNECT)
		tcp::resolver::query query(_MY_IP, _PORT);
#else
		tcp::resolver::query query(_AWS_IP, _PORT);	//	���� ���� �� ����
#endif
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

		/*boost::array<CHAR, _BUFF_SIZE> buf;
		size_t len = m_Socket.read_some(boost::asio::buffer(buf), err);
		if (err == boost::asio::error::eof) {
			m_bConnect = false;
			return;
		}
		else if (err) {
			throw boost::system::system_error(err);
		}

		std::cout.write(buf.data(), len);*/
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
		boost::shared_ptr< std::vector<BYTE> > p(new std::vector<BYTE>());
		oPacket.MakeBuf(p);
		shared_const_buffer<BYTE> buffer(p);
		boost::asio::async_write(m_Socket, buffer,
			boost::bind(&CProtocol::handle_Write, this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
				));
	}

	void ProcessPacket(InPacket& iPacket) {
		LONG nType = iPacket.Decode2();
		switch (nType) {
		case GCP_LoginRet:
			OnLoginRet(iPacket);
			break;

		case GCP_AliveAck:
		{
			OutPacket oPacket(CGP_AliveAck);
			SendPacket(oPacket);
			return;
		}			
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

		case GCP_RegisterUserRet:
			OnRegisterUserRet(iPacket);
			break;
		}
		PrintMenu();
	}

	void OnLoginRet(InPacket& iPacket) {		
		LONG nRet = iPacket.Decode2();
		if (nRet == -1) {
			std::cout << "[Login] �α��� ����" << std::endl;
			return;
		}
		else if (nRet == 0) {
			std::cout << "[Login] �α��� �Ǿ����ϴ�." << std::endl;
			CContext::get_mutable_instance().m_bLogined = TRUE;
			CContext::get_mutable_instance().m_uUserSN = iPacket.Decode4();	
			CContext::get_mutable_instance().m_sNick = iPacket.DecodeStr();

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
			m_vRoomInfo.clear();
			RoomInfo info;
			info.nSN = nRoomSN;
			info.nUserCount = nUserCnt;
			m_vRoomInfo.push_back(info);
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
		}

		boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;
		if (dwFlag & 0x7fffffff) {
			LONG nCnt = iPacket.Decode1();			
			pRoom->nUserCnt = nCnt;
			pRoom->vPlayers.clear();
			pRoom->mPlayers.clear();
			for (int i = 0; i < nCnt; ++i) {
				//	���� ������ ( ���߿��� �÷��׿� ���� ���� ������ ���Ѵ� )	
				CContext::Player::pointer pPlayer(new CContext::Player());
				pRoom->vPlayers.push_back(pPlayer);
				pPlayer->m_uUserSN = iPacket.Decode4();
				pPlayer->m_sNick = iPacket.DecodeStr();
				pPlayer->m_bReady = iPacket.Decode1();
				pRoom->mPlayers.insert(std::pair< ULONG, CContext::Player::pointer >(pPlayer->m_uUserSN, pPlayer));
			}
		}		

		pRoom->PrintInfo();
	}

	void OnLeaveRoomRet(InPacket& iPacket) {
		LONG nUserSN = iPacket.Decode4();
		if (CContext::get_mutable_instance().m_uUserSN == nUserSN) {
			CContext::get_mutable_instance().m_nRoom = -1;
			CContext::get_mutable_instance().m_pRoom = NULL;
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

		case GCP_LL_InitStatus:
			OnLLInitStatus(iPacket);
			break;

		case GCP_LL_Status:
			OnLLStatus(iPacket);
			break;

		case GCP_LL_ActionRet:
			OnActionRet(iPacket);
			break;

		case GCP_LL_RoundResult:
			OnRoundRet(iPacket);
			break;

		case GCP_LL_FinalResult:
			OnFinalRoundRet(iPacket);
			break;

		case GCP_LL_Aborted:
			OnGameAborted(iPacket);
			break;

		case GCP_LL_Emotion:
			std::cout << "\a" << std::endl;
			break;
		}
	}

	void OnRegisterUserRet(InPacket& iPacket) {
		LONG nRet = iPacket.Decode2();

		if (nRet == 0) {
			AddMessage("������ �Ǿ����ϴ�");
		}
		else {
			AddMessage(boost::str(boost::format("���� ��ϵ��� �ʾҽ��ϴ�. %d") % nRet));
		}
	}

	void OnLLInitStatus(InPacket& iPacket) {
		LONG nCurTurnIndex = iPacket.Decode4();
		LONG nCurTurnGetCard = iPacket.Decode4();
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
		LONG nHandCardType = iPacket.Decode4();
		pPlayer->m_vHandCardType.push_back(nHandCardType);
		pRoom->pLocalPlayer = pPlayer;
	}

	void OnLLStatus(InPacket& iPacket) {
		boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;
		LONG nCurTurnIndex = iPacket.Decode4();
		LONG nCurTurnGetCard = iPacket.Decode4();		
		pRoom->nCurTurnIndex = nCurTurnIndex;
		pRoom->GetPlayer(nCurTurnIndex)->m_bGuard = FALSE;
		pRoom->GetPlayer(nCurTurnIndex)->PutCardToHand(nCurTurnGetCard);
	}

	void OnActionRet(InPacket& iPacket) {
		boost::shared_ptr<CContext::RoomInfo> pRoom = CContext::get_mutable_instance().m_pRoom;
		LONG nCardType = iPacket.Decode4();
		switch (nCardType) {
		case 1:
			{
				//	��� ��� ���
				LONG nSrcIdx = iPacket.Decode4();
				LONG nTargetIdx = iPacket.Decode4();
				LONG nCardIdx = iPacket.Decode4();
				BOOL bSucceed = iPacket.Decode4();				
				pRoom->GetPlayer(nSrcIdx)->DropCard(1);
				if (bSucceed) {	
					pRoom->GetPlayer(nTargetIdx)->DropCard(nCardIdx);
					pRoom->GetPlayer(nTargetIdx)->m_bDead = true;					
					AddMessage("��� ��� ����");
				}
				else {					
					std::string sMsg = boost::str(boost::format("[���] %d�÷��̾ %d �� �ƴմϴ�.") % nTargetIdx % nCardIdx);
					AddMessage(sMsg);					
				}
			}
			break;		
		case 2:
			{
				//	���� ��� ���
				LONG nSrcIdx = iPacket.Decode4();
				LONG nTargetIdx = iPacket.Decode4();
				BOOL bMyTurn = iPacket.Decode4();
				pRoom->GetPlayer(nSrcIdx)->DropCard(2);
				if (bMyTurn) {
					LONG nCardType = iPacket.Decode4();
					std::string sMsg = boost::str(boost::format("[����] ������ %d ī�� �Դϴ�.") % nCardType);
					AddMessage(sMsg);
				}
				else {
					//	������ ������� �ƹ� �ൿ���� �ʴ´�.
				}
			}
			break;
		case 3:
		{
			//	��㰡 ��� ���
			LONG nSrcIdx = iPacket.Decode4();
			LONG nTargetIdx = iPacket.Decode4();
			LONG nRet = iPacket.Decode4();
			LONG nDeadCard = -1;
			pRoom->GetPlayer(nSrcIdx)->DropCard(3);
			if (nRet != 0) {
				nDeadCard = iPacket.Decode4();
			}
			if (nRet == 1) {	//	Src �� �¸�				
				pRoom->GetPlayer(nTargetIdx)->DropCard(nDeadCard);
				pRoom->GetPlayer(nTargetIdx)->m_bDead = TRUE;
				std::string sMsg = boost::str(boost::format("[��㰡] player[%d] �� player[%d] �� �̰���ϴ�.") % nSrcIdx % nTargetIdx);
				AddMessage(sMsg);				
			}
			else if (nRet == -1) {	//	Target �� �¸�
				pRoom->GetPlayer(nSrcIdx)->DropCard(nDeadCard);
				pRoom->GetPlayer(nSrcIdx)->m_bDead = TRUE;
				std::string sMsg = boost::str(boost::format("[��㰡] player[%d] �� player[%d] �� �̰���ϴ�.") % nTargetIdx % nSrcIdx);
				AddMessage(sMsg);
			}
			else {
				std::string sMsg = boost::str(boost::format("[��㰡] player[%d] �� player[%d]�� �ƹ� �ϵ� �������ϴ�.") % nSrcIdx % nTargetIdx);
				AddMessage(sMsg);
			}
		}
		break;
		case 4:
		{
			//	����Ͼ� ��� ���
			LONG nSrcIdx = iPacket.Decode4();
			pRoom->GetPlayer(nSrcIdx)->DropCard(4);
			pRoom->GetPlayer(nSrcIdx)->m_bGuard = TRUE;
			std::string sMsg = boost::str(boost::format("[����Ͼ�] player[%d] �� �� �� ���� ��ȣ�� �ɾ����ϴ�.") % nSrcIdx);
			AddMessage(sMsg);
		}
		break;
		case 5:
		{
			//	���� ��� ���
			LONG nSrcIdx = iPacket.Decode4();
			LONG nTargetIdx = iPacket.Decode4();
			LONG nDropCardIdx = iPacket.Decode4();
			BOOL bTargetPlayer = iPacket.Decode4();
			LONG nNewCard = bTargetPlayer ? iPacket.Decode4() : -1;
			pRoom->GetPlayer(nSrcIdx)->DropCard(5);
			std::string sMsg = boost::str(boost::format("[����] player[%d]�� player[%d]���� ����߽��ϴ�.") % nSrcIdx % nTargetIdx);
			AddMessage(sMsg);
			if (nDropCardIdx == 8) {
				std::string sMsg = boost::str(boost::format("[����] player[%d]�� ���� ī�带 ����Ͽ� �׾����ϴ�.") % nTargetIdx);
				pRoom->GetPlayer(nTargetIdx)->DropCard(8);
				pRoom->GetPlayer(nTargetIdx)->m_bDead = TRUE;
				AddMessage(sMsg);
			}
			else {
				pRoom->GetPlayer(nTargetIdx)->DropCard(nDropCardIdx);
				pRoom->GetPlayer(nTargetIdx)->PutCardToHand(nNewCard);
			}
		}
		break;
		case 6:
		{			
			LONG nSrcIdx = iPacket.Decode4();
			LONG nTargetIdx = iPacket.Decode4();
			BOOL bSrcOrTarget = iPacket.Decode4();
			pRoom->GetPlayer(nSrcIdx)->DropCard(6);
			std::string sMsg = boost::str(boost::format("[���ڵ�] player[%d]�� player[%d]���� ����߽��ϴ�.") % nSrcIdx % nTargetIdx);
			AddMessage(sMsg);
			if (bSrcOrTarget) {
				LONG nSrcToTargetCardIdx = iPacket.Decode4();
				LONG nTargetToSrcCardIdx = iPacket.Decode4();
				pRoom->GetPlayer(nSrcIdx)->SendCard(pRoom->GetPlayer(nTargetIdx), nSrcToTargetCardIdx);
				pRoom->GetPlayer(nTargetIdx)->SendCard(pRoom->GetPlayer(nSrcIdx), nTargetToSrcCardIdx);				
			}
			else {
			}
		}
		break;

		case 7:
		{
			LONG nSrcIdx = iPacket.Decode4();
			pRoom->GetPlayer(nSrcIdx)->DropCard(7);
			std::string sMsg = boost::str(boost::format("[�պ�] player[%d]�� �պ� �½��ϴ�.") % nSrcIdx);
			AddMessage(sMsg);
		}
		break;
		}
	}	

	void OnRoundRet(InPacket& iPacket) {
		LONG nReason = iPacket.Decode4();
		LONG nWinnerIdx = iPacket.Decode4();
		//	���ڴ� �̰ܼ�
		system("cls");
		std::string sMsg = boost::str(boost::format("[���� ���] player[%d]�� �¸��Ͽ����ϴ�.") % nWinnerIdx);
		std::cout << sMsg << std::endl;
		Sleep(3000);
		PrintMenu();
	}

	void OnFinalRoundRet(InPacket& iPacket) {
		CContext::get_mutable_instance().m_pRoom->m_bGameStart = FALSE;
		LONG nWinnerIdx = iPacket.Decode4();
		//	���ڴ� �̰ܼ�
		system("cls");
		std::string sMsg = boost::str(boost::format("[���� ���] player[%d]�� �¸��Ͽ����ϴ�.") % nWinnerIdx);
		std::cout << sMsg << std::endl;
		Sleep(3000);
		PrintMenu();
	}

	void OnGameAborted(InPacket& iPacket) {
		CContext::get_mutable_instance().m_pRoom->m_bGameStart = FALSE;
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
	boost::lock_guard<boost::mutex> lock(g_mutex);
	system("cls");	
	if (CContext::get_mutable_instance().IsGameRunning()) {
		PrintActionLog();
		CContext::get_mutable_instance().m_pRoom->PrintMyCardAndAllGroundCard();
		std::cout << "[�޴�] 0-ī�� ���" << std::endl;
	}
	else {
		if (!CContext::get_mutable_instance().m_bLogined) {
			PrintActionLog();
			std::cout << "[�޴�] 0-�α��� 9-���" << std::endl;
			return;
		}		

		if (CContext::get_mutable_instance().m_pRoom) {
			CContext::get_mutable_instance().m_pRoom->PrintInfo();
			std::cout << "[�޴�] 3-������, 4-���ӷ���, 5-���ӽ���" << std::endl;
		}
		else {
			std::cout << "[�޴�] 1-�游���, 2-������, 6-������ ��û" << std::endl;
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
		for (int i = 2; i >= 0; --i) {
			
		}
		for (LONG i = g_vMessages.size() - 1; (LONG)(g_vMessages.size() - 3) < i; --i) {
			std::cout << g_vMessages[i] << std::endl;
		}
	}

	std::cout << "" << std::endl;
}

void ProcGameMenu(CProtocol& client, LONG n) {
	char line[128 + 1];
	switch (n) {
		case 0: {			
			std::cout << "� ī�带 ���ðڼ� : ";
			std::cin.getline(line, 128);			
			int n2 = atoi(line);
			ProcCardCommand(client, n2);			
		}
				break;	

		case 1: {
			// �Ŀ���
			OutPacket oPacket(CGP_GameLoveLetter);
			oPacket.Encode2(CGP_LL_Emotion);
			oPacket.Encode4(CContext::get_mutable_instance().m_pRoom->pLocalPlayer->m_nIndex);
			client.SendPacket(oPacket);			
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
		//	��� ī�� ��� ( �������� �����̳İ� ��������� )
		LONG nWho;
		LONG nWhat;
		std::cout << "[���] ��������? : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);
		std::cout << "[���] ������ �� ���� : ";
		std::cin.getline(line, 128);
		nWhat = atoi(line);
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_GuardCheck);
		oPacket.Encode4(nWho);
		oPacket.Encode4(nWhat);
		client.SendPacket(oPacket);
	}

	if (n == 2) {
		//	�սǽ��� ī�� ��� ( ���� �����̳İ� ��������� )
		LONG nWho;		
		std::cout << "[����] �������� ����ڼ�? : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);		
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_RoyalSubject);
		oPacket.Encode4(nWho);		
		client.SendPacket(oPacket);
	}

	if (n == 3) {
		//	��㰡 ��� ( ������ �ο� ���̳İ� ���� �� )
		LONG nWho;
		std::cout << "[��㰡] ������ �ο�ڼ�? : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Gossip);
		oPacket.Encode4(nWho);
		client.SendPacket(oPacket);
	}

	if (n == 4) {
		//	���� ���						
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Companion);
		client.SendPacket(oPacket);
	}

	if (n == 5) {
		//	���� ��� ( �������� ���ڼ� )
		LONG nWho;
		std::cout << "[����] ������ ī�带 ������ �ϰڼ�?(�ڽ� ����) : ";
		std::cin.getline(line, 128);
		nWho = atoi(line);
		OutPacket oPacket(CGP_GameLoveLetter);
		oPacket.Encode2(CGP_LL_Hero);
		oPacket.Encode4(nWho);
		client.SendPacket(oPacket);
	}

	if (n == 6) {
		//	������ ��� ( �������� ���ڼ� )
		LONG nWho;
		std::cout << "[������] ������ ī�带 ��ȯ�ϰڼ�? : ";
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
		std::cout << "�α��� �õ�" << std::endl;
		OutPacket oPacket(CGP_Login);		
		std::cout << "ID : ";
		std::cin.getline(line, 128);
		std::string sID(line);
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		DWORD mode = 0;
		GetConsoleMode(hStdin, &mode);
		SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
		std::cout << "PW : ";
		std::cin.getline(line, 128);
		SetConsoleMode(hStdin, mode);
		std::string sPW(line);
		oPacket.EncodeStr(sID);
		oPacket.EncodeStr(sPW);
		client.SendPacket(oPacket);
	}
			break;
	case 1: {
		OutPacket oPacket(CGP_CreateRoom);
		client.SendPacket(oPacket);
		//	�� ����
	}
			break;

	case 2: {
		std::cout << "�� ���� �õ�" << std::endl;
		std::cout << "����� ���� : ";
		std::cin.getline(line, 128);
		int n2 = atoi(line);
		OutPacket oPacket(CGP_EnterRoom);
		oPacket.Encode4(n2);
		client.SendPacket(oPacket);
	}
			break;

	case 3: {
		OutPacket oPacket(CGP_LeaveRoom);
		client.SendPacket(oPacket);
	}
			break;

	case 4: {
		OutPacket oPacket(CGP_GameReady);
		client.SendPacket(oPacket);

	}
			break;

	case 5: {
		OutPacket oPacket(CGP_GameStart);
		client.SendPacket(oPacket);
	}
			break;

	case 6: {
		OutPacket oPacket(CGP_RoomListRequest);
		client.SendPacket(oPacket);
	}
			break;

	case 9:
	{
		std::cout << "���� ���" << std::endl;
		std::cout << "id : ";
		std::cin.getline(line, 128);
		std::string sID(line);
		std::cout << "pw : ";
		std::cin.getline(line, 128);
		std::string sPW(line);
		std::cout << "nick : ";
		std::cin.getline(line, 128);
		std::string sNick(line);
		OutPacket oPacket(CGP_RegisterUser);
		oPacket.EncodeStr(sID);
		oPacket.EncodeStr(sPW);
		oPacket.EncodeStr(sNick);
		client.SendPacket(oPacket);
	}
	break;

	default:
		std::cout << "�߸��� ��ɾ�" << std::endl;
	}
}