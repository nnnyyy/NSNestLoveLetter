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

void CUser::OnRoomListRequest(InPacket &iPacket) {
	OutPacket oPacket(GCP_RoomListRet);
	CRoomManager::get_mutable_instance().MakeRoomListPacket(oPacket);
	SendPacket(oPacket);
}

void CUser::OnCreateRoom(InPacket &iPacket) {
	if (m_pRoom) {
		// Err		
		CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
		pRoom->RemoveUser(shared_from_this());
		m_pRoom = NULL;
		OutPacket oPacket(GCP_CreateRoomRet);
		oPacket.Encode2(-1);		
		SendPacket(oPacket);
		return;
	}
	CRoom::pointer pRoom = CRoomManager::MakeRoom();
	m_pRoom = pRoom;

	OutPacket oPacket(GCP_CreateRoomRet);
	oPacket.Encode2(0);
	oPacket.Encode4(pRoom->GetSN());
	SendPacket(oPacket);

	pRoom->Enter(shared_from_this());
	CRoomManager::get_mutable_instance().Register(pRoom);

	
}

void CUser::OnEnterRoom(InPacket &iPacket) {	
	if (m_pRoom) {		
		CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
		pRoom->RemoveUser(shared_from_this());
		m_pRoom = NULL;
		// Err
		// SendFailPacket();
		return;
	}

	LONG nRoomSN = iPacket.Decode4();
	CRoom::pointer pRoom = CRoomManager::get_mutable_instance().GetRoom(nRoomSN);
	if (!pRoom) {
		// SendFailPacket();
		return;
	}

	m_pRoom = pRoom;
	pRoom->Enter(shared_from_this());
}

void CUser::OnLeaveRoom(InPacket &iPacket) {
	if (!m_pRoom) {
		//	이미 나갔다
		return;
	}
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	pRoom->RemoveUser(shared_from_this());
	//	SendRetPacket
	OutPacket oPacket(GCP_LeaveRoomRet);
	oPacket.Encode4(m_nUserSN);
	pRoom->BroadcastPacket(oPacket);	
	m_pRoom = NULL;	

	OutPacket oPacket2(GCP_LeaveRoomRet);
	oPacket2.Encode4(m_nUserSN);
	SendPacket(oPacket2);
}

void CUser::OnGameStart(InPacket &iPacket) {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (!pRoom) {
		return;
	}
	
	pRoom->Start(shared_from_this());
}

void CUser::OnGameReady(InPacket &iPacket) {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (!pRoom) {
		return;
	}

	if (pRoom->GetMaster() == shared_from_this()) {
		//	방장은 레디 상태를 변경할 수 없다.
		pRoom->BroadcastRoomState();
		return;
	}
	
	m_bReady = !m_bReady;
	pRoom->BroadcastRoomState();
}