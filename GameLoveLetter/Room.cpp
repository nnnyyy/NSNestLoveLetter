#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "GameData.h"
#include "PacketProtocol.h"
#include "Packet.h"
#include "User.h"
#include "Connection.h"
#include "Server.h"
#include "GameDealer.h"
#include "Room.h"
#include "LogMan.h"

LONG CRoom::s_nSN = 0;

CRoom::CRoom(){
	m_nSN = s_nSN++;
	CGameDealerLoveLetter::pointer p(new CGameDealerLoveLetter());
	m_pDealer = p;
}

CRoom::~CRoom() {

}

void CRoom::Enter(CUser::pointer pUser) {
	if (GetPlayerCount() >= USER_MAX) {		
		return;
	}

	if (IsGameRunning()) {		
		return;
	}

	pUser->m_bReady = FALSE;
	m_vUsers.push_back(pUser);
	m_mUsers.insert(std::pair<ULONG, CUser::pointer>( pUser->m_nUserSN, pUser ));

	if (m_vUsers.size() == 1) {
		m_pMaster = pUser;
		m_pMaster->m_bReady = TRUE;
		LogAdd(boost::str(boost::format("[Room:%d] Created") % m_nSN));
	}
	SendEnterPacket(pUser);
}

void CRoom::SendEnterPacket(CUser::pointer pUser) {
	OutPacket oPacket(GCP_EnterRoomRet);
	oPacket.Encode2(0);
	oPacket.Encode4(GetSN());
	pUser->SendPacket(oPacket);

	BroadcastRoomState(FLAG_ROOM_MASTER | FLAG_WITHOUT_ROOM_MASTER);
}

void CRoom::BroadcastRoomState(DWORD dwFlag) {
	OutPacket oPacket(GCP_RoomState);
	oPacket.Encode4(dwFlag);
	if (dwFlag & FLAG_ROOM_MASTER) {
		oPacket.Encode4(m_pMaster->m_nUserSN);		
	}

	if (dwFlag & FLAG_WITHOUT_ROOM_MASTER) {
		LONG nCnt = m_vUsers.size();		
		oPacket.Encode1(nCnt);
		for (int i = 0; i < nCnt; ++i) {
			oPacket.Encode4(m_vUsers[i]->m_nUserSN);
			oPacket.EncodeStr(m_vUsers[i]->m_sNick);
			oPacket.Encode1(m_vUsers[i]->m_bReady);
		}
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

	RegisterCPU();	//	CPU도 참전

	if (GetPlayerCount() < USER_MIN) {
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

	LogAdd(boost::str(boost::format("[Room:%d] Game Start")%m_nSN));
	OutPacket oPacket(GCP_GameStartRet);
	oPacket.Encode4(0);	//	정상
	//	유저별 게임 인덱스 정보 전송
	m_pDealer->EncodePlayerIndexList(oPacket);
	BroadcastPacket(oPacket);

	m_pDealer->SendGameInitInfo();
	m_pDealer->Process();
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
			LogAdd(boost::str(boost::format("[Room:%d] User Removed : %s(sn:%d)") % m_nSN % pUser->m_sNick % pUser->m_nUserSN));
			bFind = TRUE;
			break;
		}
	}	

	if (IsGameRunning()) {		
		for each (CUser::pointer p in m_vUsers)
		{
			if (m_pMaster == p) {
				p->m_bReady = TRUE;
			}
			else {
				p->m_bReady = FALSE;
			}			
		}
		m_bGameStart = FALSE;
		m_pDealer->StopGame();
	}

	if (bFind && m_vUsers.size() > 0 && m_pMaster == pUser) {
		m_pMaster = m_vUsers[0];
		m_pMaster->m_bReady = TRUE;
		//	새 방장 알림 패킷
		BroadcastRoomState(FLAG_ROOM_MASTER);
		OutPacket oPacket(GCP_RoomState);
		DWORD dwFlag = 0x80000000;	//	새 방장 알리기
		
	}

	BroadcastRoomState();
}

void CRoom::ResetReady() {
	m_bGameStart = FALSE;
	for each (CUser::pointer pUser in m_vUsers)
	{
		if (pUser == m_pMaster) {
			m_pMaster->m_bReady = TRUE;
		}
		else {
			pUser->m_bReady = FALSE;
		}
	}
}

void CRoom::RegisterCPU() {
	LONG nUserCnt = m_vUsers.size();
	LONG nCPUCnt = USER_MAX - nUserCnt;
	m_vCPUs.clear();

	for (int i = 0; i < nCPUCnt; ++i) {
		CPUInfo info;
		info.sName = boost::str(boost::format("CPU %d") % i);
		m_vCPUs.push_back(info);
	}
}

LONG CRoom::GetPlayerCount() const {
	LONG nUserCnt = m_vUsers.size();
	LONG nCPUCnt = m_vCPUs.size();
	return nUserCnt + nCPUCnt;
}

CRoom::pointer CRoomManager::MakeRoom() {
	CRoom::pointer pRoom(new CRoom);	
	return pRoom;
}

void CRoomManager::Register(CRoom::pointer pRoom) {
	boost::lock_guard<boost::mutex> lock(m_LockMutex);
	m_vRooms.push_back(pRoom);
	m_mRooms.insert(std::pair<LONG, CRoom::pointer >(pRoom->GetSN(),pRoom));
}

CRoom::pointer CRoomManager::GetRoom(LONG nSN) {
	boost::lock_guard<boost::mutex> lock(m_LockMutex);
	if (m_mRooms.find(nSN) == m_mRooms.end()) return NULL;
	return m_mRooms.at(nSN);
}

void CRoomManager::Update() {
	for (std::vector<CRoom::pointer>::iterator iter = m_vRooms.begin(); iter != m_vRooms.end(); ) {
		if ((*iter)->GetUserCount() <= 0 ) {
			(*iter)->Destroy();
			LogAdd(boost::str(boost::format("[RoomManager] Destroy Room : %d") % (*iter)->GetSN()));
			m_LockMutex.lock();
			m_mRooms.erase((*iter)->GetSN());			
			iter = m_vRooms.erase(iter);
			m_LockMutex.unlock();
			continue;
		}
		(*iter)->Update(0);
		++iter;
	}
}

void CRoomManager::MakeRoomListPacket(OutPacket& oPacket) {
	boost::lock_guard<boost::mutex> lock(m_LockMutex);
	oPacket.Encode4(m_vRooms.size());
	for (std::vector<CRoom::pointer>::iterator iter = m_vRooms.begin(); iter != m_vRooms.end(); ++iter) {
		CRoom::pointer pRoom = *iter;
		if (pRoom->IsGameRunning()) {
			continue;
		}
		oPacket.Encode4(pRoom->GetSN());
		oPacket.Encode1(pRoom->GetUserCount());
	}
}