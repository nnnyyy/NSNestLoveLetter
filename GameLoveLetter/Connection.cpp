#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "PacketProtocol.h"
#include "Packet.h"
#include "User.h"
#include "Connection.h"
#include "User.h"
#include "Server.h"
#include "GameDealer.h"
#include "Room.h"
#include "MysqlMan.h"

using boost::asio::ip::tcp;	

void CConnection::handle_Accept(const boost::system::error_code& err, size_t byte_transferred) {	
	m_Socket.async_read_some(
		boost::asio::buffer(m_RecvBuf),
		boost::bind(&CConnection::handle_Read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred 
			));	
}

void CConnection::handle_Read(const boost::system::error_code& err, size_t byte_transferred) {	
	if (!err) {
		if (m_RecvBuf.size()) {			
			BYTE *pBuf = &m_RecvBuf[0];

			while (pBuf) {
				LONG nState = packetBuf.Append(pBuf, byte_transferred);
				if (nState == InPacket::PS_COMPLETE) {
					ProcessPacket(packetBuf);
					packetBuf.Clear();
				}
			}
		}

		m_Socket.async_read_some(
			boost::asio::buffer(m_RecvBuf),
			boost::bind(&CConnection::handle_Read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
				));
	}
	else {		
		std::cout << m_uSocketSN << " - Disconnect(Write) : " << err.message() << std::endl;
		Server_Wrapper::get_mutable_instance().m_pServer->RemoveSocket(shared_from_this());		
		if (m_pUser) {
			Server_Wrapper::m_mUsers.erase(m_pUser->m_nUserSN);
			CMysqlManager::get_mutable_instance().Logout(m_pUser->m_nUserSN);
			m_pUser->PostDisconnect();
			m_pUser = NULL;
		}		
	}
}

void CConnection::handle_Write(const boost::system::error_code& err, size_t byte_transferred) {

}


void CConnection::start() {	
	time_t now = time(0);	
	m_sMsg = "Hello";
	std::cout << "Scoket Connected : " << m_Socket.remote_endpoint().address().to_string() << std::endl;
	boost::asio::async_write(m_Socket, boost::asio::buffer(m_sMsg), 
		boost::bind(&CConnection::handle_Accept, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
		));
}

void CConnection::ProcessPacket(InPacket &iPacket) {
	LONG nType = iPacket.Decode2();
	if (nType >= CGP_User_Start && nType < CGP_User_End) {
		ProcessUserPacket(nType, iPacket);
		return;
	}

	switch (nType) {
	case CGP_Login: OnLogin(iPacket); break;
	default:
		return;
	}
}

void CConnection::ProcessUserPacket(LONG nType, InPacket &iPacket) {
	if (!m_pUser) {
		// Err
		// SendErrorPacket();
		return;
	}

	if (nType >= CGP_GamePacket_Start && nType <= CGP_GamePacket_End) {
		CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pUser->GetRoom());
		if (!pRoom) {
			// Err
			// SendErrorPacket();
			return;
		}		
		pRoom->OnGamePacket(iPacket, m_pUser);
		return;
	}

	switch (nType) {
	case CGP_RoomListRequest: m_pUser->OnRoomListRequest(iPacket); break;
	case CGP_CreateRoom: m_pUser->OnCreateRoom(iPacket); break;
	case CGP_EnterRoom: m_pUser->OnEnterRoom(iPacket); break;
	case CGP_LeaveRoom: m_pUser->OnLeaveRoom(iPacket); break;
	case CGP_GameStart: m_pUser->OnGameStart(iPacket); break;
	case CGP_GameReady: m_pUser->OnGameReady(iPacket); break;		
	}
}

void CConnection::SendPacket(OutPacket &oPacket) {
	boost::shared_ptr< std::vector<BYTE> > p(new std::vector<BYTE>());
	oPacket.MakeBuf(p);	
	shared_const_buffer<BYTE> buffer(p);
	boost::asio::async_write(m_Socket, buffer,
		boost::bind(&CConnection::handle_Write, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
		));
}


void CConnection::OnLogin(InPacket &iPacket) {	
	if (m_pUser) {
		//Disconnect();
		OutPacket oPacket(GCP_LoginRet);
		oPacket.Encode2(-1);
		SendPacket(oPacket);
		return;
	}

	std::string sID = iPacket.DecodeStr();
	std::string sPW = iPacket.DecodeStr();
	LONG nSN = -1, nRet = 0;
	std::string sNick;
	if ((nRet = CMysqlManager::get_mutable_instance().Login(sID, sPW, nSN, sNick)) != 0) {
		OutPacket oPacket(GCP_LoginRet);
		oPacket.Encode2(nRet);
		SendPacket(oPacket);
		return;
	}

	boost::shared_ptr<CUser> pUser(new CUser());
	pUser->SetConnection(m_uSocketSN);
	pUser->m_nUserSN = nSN;
	pUser->m_sNick = sNick;
	m_pUser = pUser;
	Server_Wrapper::m_mUsers.insert(std::pair<ULONG, CUser::pointer >(pUser->m_nUserSN, pUser));
	
	OutPacket oPacket(GCP_LoginRet);
	oPacket.Encode2(0);
	oPacket.Encode4(pUser->m_nUserSN);	
	oPacket.EncodeStr(pUser->m_sNick);
	SendPacket(oPacket);

	std::cout << "Logined : " << sID << ", " << pUser->m_nUserSN << ", " << pUser->m_sNick << std::endl;
}