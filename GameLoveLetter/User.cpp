#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Packet.h"
#include "Connection.h"
#include "Server.h"



CUser::CUser() {

}

CUser::~CUser() {

}

LONG CUser::GetCharacterID() const {
	return 0;
}

void CUser::SendPacket(OutPacket& oPacket) {		
	CConnection::pointer pConn = Server_Wrapper::get_mutable_instance().m_pServer->m_mSockets.at(m_uSocketSN);
	pConn->SendPacket(oPacket);

}

void CUser::SetConnection(ULONG uSocketSN) {	
	m_uSocketSN = uSocketSN;	
}