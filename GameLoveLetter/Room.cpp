#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Packet.h"
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
	OutPacket oPacket(1);
	oPacket.Encode1(0);
	oPacket.Encode2(pUser->GetCharacterID());
	BroadcastPacket(oPacket);
}

void CRoom::BroadcastPacket(OutPacket& oPacket) {
	LONG nCnt = m_vUsers.size();
	for (int i = 0; i < nCnt; ++i) {
		m_vUsers[i]->SendPacket(oPacket);
	}
}