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
#include "MysqlMan.h"
#include "LogMan.h"

using boost::asio::ip::tcp;	
using namespace boost::chrono;

const LONG ALIVE_CHECK_TIME_MILLI = 10000;

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
		LogAdd(boost::str(boost::format("[Disconnected] SocketSN : %d, pUser : %d") % m_uSocketSN % m_pUser));
		Server_Wrapper::get_mutable_instance().m_pServer->RemoveSocket(shared_from_this());		
		if (m_pUser) {
			LogAdd(boost::str(boost::format("[Disconnected] UserSN : %d, UserName : %s") % m_pUser->m_nUserSN % m_pUser->m_sNick));
			Server_Wrapper::m_mUsers.erase(m_pUser->m_nUserSN);
			CMysqlManager::get_mutable_instance().SetGameDataToDB(m_pUser->m_nUserSN, m_pUser->gamedata);
			CMysqlManager::get_mutable_instance().Logout(m_pUser->m_nUserSN);
			m_pUser->PostDisconnect();
			m_pUser = NULL;
		}		
	}
}

void CConnection::handle_Write(const boost::system::error_code& err, size_t byte_transferred) {

}


void CConnection::start() {	
	tAliveCheckTime = system_clock::now();
	time_t now = time(0);	
	LogAdd(boost::str(boost::format("Scoket Connected : %s") 
		% m_Socket.remote_endpoint().address().to_string()));

	m_Socket.async_read_some(
		boost::asio::buffer(m_RecvBuf),
		boost::bind(&CConnection::handle_Read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
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
	case CGP_RegisterUser: OnRegister(iPacket); break;
	case CGP_AliveAck: OnAliveAck(iPacket); break;
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
		LogAdd(boost::str(boost::format("[Login] Error id: %s, Ret: %d") % sID % nRet));
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
	CMysqlManager::get_mutable_instance().GetGameDataFromDB(pUser->m_nUserSN, pUser->gamedata);
	Server_Wrapper::m_mUsers.insert(std::pair<ULONG, CUser::pointer >(pUser->m_nUserSN, pUser));
	
	OutPacket oPacket(GCP_LoginRet);
	oPacket.Encode2(0);
	oPacket.Encode4(pUser->m_nUserSN);	
	oPacket.EncodeStr(pUser->m_sNick);
	SendPacket(oPacket);	
	LogAdd(boost::str(boost::format("Logined : %s, %d, %s") % sID % pUser->m_nUserSN % pUser->m_sNick));
}

void CConnection::OnRegister(InPacket &iPacket) {
	std::string sID = iPacket.DecodeStr();
	std::string sPW = iPacket.DecodeStr();
	std::string sNick = iPacket.DecodeStr();
	LONG nRet = 0;
	if ((nRet = CMysqlManager::get_mutable_instance().RegisterUser(sID, sPW, sNick)) != 0) {
		LogAdd(boost::str(boost::format("[RegisterUser] Error id: %s, Ret: %d") % sID % nRet));
		OutPacket oPacket(GCP_RegisterUserRet);
		oPacket.Encode2(nRet);
		SendPacket(oPacket);
		return;
	}

	LogAdd(boost::str(boost::format("[RegisterUser] Succeed id: %s, Ret: %d") % sID % nRet));
	OutPacket oPacket(GCP_RegisterUserRet);
	oPacket.Encode2(nRet);
	SendPacket(oPacket);
}

void CConnection::OnAliveAck(InPacket &iPacket) {	
	bAlive = TRUE;
}

void CConnection::Update(){
	if (bDisconnecting) return;
	if (duration_cast<milliseconds>(system_clock::now() - tAliveCheckTime).count() >= ALIVE_CHECK_TIME_MILLI) {
		if (!bAlive && !bDisconnecting) {
			LogAdd(boost::str(boost::format("[AliveCheck] Disconnect: Socket SN - %d") % m_uSocketSN));
			m_Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			m_Socket.close();
			bDisconnecting = TRUE;
			return;
		}
		//	5�ʿ� �ѹ� ����̺� üũ
		OutPacket oPacket(GCP_AliveAck);		
		SendPacket(oPacket);

		tAliveCheckTime = system_clock::now();
		bAlive = FALSE;
	}
}