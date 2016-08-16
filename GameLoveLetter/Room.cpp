#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "PacketProtocol.h"
#include "Packet.h"
#include "User.h"
#include "Connection.h"
#include "Server.h"
#include "GameDealer.h"
#include "Room.h"

LONG CRoom::s_nSN = 0;

CRoom::CRoom(){
	m_nSN = s_nSN++;
	CGameDealerLoveLetter::pointer p(new CGameDealerLoveLetter());
	m_pDealer = p;
}

CRoom::~CRoom() {

}

void CRoom::Enter(CUser::pointer pUser) {
	if (m_vUsers.size() >= USER_MAX) {
		//	Log(ERROR, "What!?");
		return;
	}

	m_vUsers.push_back(pUser);
	m_mUsers.insert(std::pair<ULONG, CUser::pointer>( pUser->m_nUserSN, pUser ));

	if (m_vUsers.size() == 1) {
		m_pMaster = pUser;
		m_pMaster->m_bReady = TRUE;
	}
	SendEnterPacket(pUser);
}

void CRoom::SendEnterPacket(CUser::pointer pUser) {
	OutPacket oPacket(GCP_EnterRoomRet);
	oPacket.Encode2(0);
	oPacket.Encode4(GetSN());
	pUser->SendPacket(oPacket);

	BroadcastRoomState();
}

void CRoom::BroadcastRoomState(DWORD dwFlag) {
	OutPacket oPacket(GCP_RoomState);
	LONG nCnt = m_vUsers.size();
	oPacket.Encode4(dwFlag);
	oPacket.Encode1(nCnt);
	for (int i = 0; i < nCnt; ++i) {
		oPacket.Encode4(m_vUsers[i]->m_nUserSN);
		oPacket.Encode1(m_vUsers[i]->m_bReady);
	}
	BroadcastPacket(oPacket);
}

void CRoom::BroadcastPacket(OutPacket& oPacket) {
	LONG nCnt = m_vUsers.size();
	for (int i = 0; i < nCnt; ++i) {
		m_vUsers[i]->SendPacket(oPacket);
	}
}

void CRoom::Update(LONG tCur) {
	if (!m_bGameStart) {
		return;
	}

	m_pDealer->Update();
}

void CRoom::Destroy() {
	m_vUsers.clear();
	m_bGameStart = FALSE;
	m_pDealer->m_pRoom = NULL;
}

void CRoom::OnGamePacket(InPacket& iPacket, CUser::pointer pUser) {
	if (!m_bGameStart) {
		return;
	}

	m_pDealer->OnPacket(iPacket, pUser);
}

void CRoom::Start(CUser::pointer pUser) {
	if (pUser != m_pMaster) {		
		OutPacket oPacket(GCP_GameStartRet);
		oPacket.Encode4(-2);	//	방장 아님 오류
		BroadcastPacket(oPacket);
		return;
	}

	if (m_vUsers.size() < USER_MAX) {
		//	유저가 부족하다.
		OutPacket oPacket(GCP_GameStartRet);
		oPacket.Encode4(-1);	//	유저 부족 오류
		BroadcastPacket(oPacket);
		return;
	}

	for (std::vector < CUser::pointer >::iterator iter = m_vUsers.begin(); iter != m_vUsers.end(); ++iter) {
		CUser::pointer pUser = *iter;
		if (pUser != m_pMaster && !pUser->m_bReady) {
			OutPacket oPacket(GCP_GameStartRet);
			oPacket.Encode4(-3);	//	전부 레디 상태 아님 오류
			BroadcastPacket(oPacket);			
			return;
		}
				
	}

	m_pDealer->m_pRoom = shared_from_this();
	m_pDealer->InitGame();
	m_bGameStart = TRUE;

	OutPacket oPacket(GCP_GameStartRet);
	oPacket.Encode4(0);	//	정상
	//	유저별 게임 인덱스 정보 전송
	m_pDealer->EncodePlayerIndexList(oPacket);
	BroadcastPacket(oPacket);

	m_pDealer->NextTurn();
}

LONG CRoom::GetUserCount() {
	return m_vUsers.size();
}

void CRoom::RemoveUser(CUser::pointer pUser) {
	if (!pUser) return;
	m_mUsers.erase(pUser->m_nUserSN);

	BOOL bFind = FALSE;
	for (std::vector<CUser::pointer>::iterator iter = m_vUsers.begin(); iter != m_vUsers.end(); ++iter) {
		if (*iter == pUser) {
			m_vUsers.erase(iter);
			std::cout << "User Removed : " << pUser->m_nUserSN << std::endl;			
			bFind = TRUE;
			break;
		}
	}	

	if (bFind && m_vUsers.size() > 0 && m_pMaster == pUser) {
		m_pMaster = m_vUsers[0];
		m_pMaster->m_bReady = TRUE;
		//	새 방장 알림 패킷
		OutPacket oPacket(GCP_RoomState);
		DWORD dwFlag = 0x80000000;	//	새 방장 알리기
		oPacket.Encode4(dwFlag);
		oPacket.Encode4(m_pMaster->m_nUserSN);
		BroadcastPacket(oPacket);
	}

	BroadcastRoomState();
}

CRoom::pointer CRoomManager::MakeRoom() {
	CRoom::pointer pRoom(new CRoom);	
	return pRoom;
}

void CRoomManager::Register(CRoom::pointer pRoom) {
	m_vRooms.push_back(pRoom);
	m_mRooms.insert(std::pair<LONG, CRoom::pointer >(pRoom->GetSN(),pRoom));
}

CRoom::pointer CRoomManager::GetRoom(LONG nSN) {
	if (m_mRooms.find(nSN) == m_mRooms.end()) return NULL;
	return m_mRooms.at(nSN);
}

void CRoomManager::Update() {
	for (std::vector<CRoom::pointer>::iterator iter = m_vRooms.begin(); iter != m_vRooms.end(); ) {
		if ((*iter)->GetUserCount() <= 0 ) {
			(*iter)->Destroy();
			m_mRooms.erase((*iter)->GetSN());
			iter = m_vRooms.erase(iter);			
			continue;
		}
		(*iter)->Update(0);
		++iter;
	}
}

void CRoomManager::MakeRoomListPacket(OutPacket& oPacket) {
	oPacket.Encode4(m_vRooms.size());
	for (std::vector<CRoom::pointer>::iterator iter = m_vRooms.begin(); iter != m_vRooms.end(); ++iter) {
		CRoom::pointer pRoom = *iter;
		oPacket.Encode4(pRoom->GetSN());
		oPacket.Encode1(pRoom->GetUserCount());
	}
}