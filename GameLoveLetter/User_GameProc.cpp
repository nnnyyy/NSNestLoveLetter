#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Packet.h"
#include "User.h"
#include "Connection.h"
#include "Server.h"
#include "Room.h"

void CUser::OnCreateRoom(InPacket &iPacket) {
	if (m_pRoom) {
		// Err		
		CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
		pRoom->RemoveUser(shared_from_this());
		m_pRoom = NULL;
		return;
	}
	CRoom::pointer pRoom = CRoomManager::MakeRoom();
	m_pRoom = pRoom;
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
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	pRoom->RemoveUser(shared_from_this());
	m_pRoom = NULL;
	//	SendRetPacket
}

void CUser::OnGameStart(InPacket &iPacket) {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (!pRoom) {
		return;
	}
	//pRoom->Start();
}

void CUser::OnGameReady(InPacket &iPacket) {
}