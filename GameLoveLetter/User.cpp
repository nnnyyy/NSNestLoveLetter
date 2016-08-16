#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Packet.h"
#include "User.h"
#include "Connection.h"
#include "Server.h"
#include "GameDealer.h"
#include "Room.h"



CUser::CUser() : m_bReady(FALSE){

}

CUser::~CUser() {

}

void CUser::SendPacket(OutPacket& oPacket) {		
	CConnection::pointer pConn = Server_Wrapper::get_mutable_instance().m_pServer->m_mSockets.at(m_uSocketSN);
	pConn->SendPacket(oPacket);

}

void CUser::SetConnection(ULONG uSocketSN) {	
	m_uSocketSN = uSocketSN;	
}

void CUser::PostDisconnect() {
	if (m_pRoom) {		
		CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
		pRoom->RemoveUser(shared_from_this());
	}

	m_pRoom = NULL;
}