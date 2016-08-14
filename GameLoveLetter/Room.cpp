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
#include "Room.h"

LONG CRoom::s_nSN = 0;

CRoom::CRoom() {
	m_nSN = s_nSN++;
}

CRoom::~CRoom() {

}

void CRoom::Enter(CUser::pointer pUser) {
	if (m_vUsers.size() >= USER_MAX) {
		//	Log(ERROR, "What!?");
		return;
	}

	m_vUsers.push_back(pUser);
	SendEnterPacket(pUser);
}

void CRoom::SendEnterPacket(CUser::pointer pUser) {
	OutPacket oPacket(GCP_RoomInfoRet);
	LONG nCnt = m_vUsers.size();
	oPacket.Encode1(nCnt);
	for (int i = 0; i < nCnt; ++i) {
		oPacket.Encode2(pUser->GetCharacterID());
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
	std::cout << "Room : " << m_nSN << " Update" << std::endl;
}

void CRoom::Destroy() {
	m_vUsers.clear();
}

LONG CRoom::GetUserCount() {
	return m_vUsers.size();
}

void CRoom::RemoveUser(CUser::pointer pUser) {
	for (std::vector<CUser::pointer>::iterator iter = m_vUsers.begin(); iter != m_vUsers.end(); ++iter) {
		if (*iter == pUser) {
			m_vUsers.erase(iter);
			std::cout << "User Removed : " << pUser->GetCharacterID() << std::endl;
			return;
		}
	}
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