#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Connection.h"
#include "Packet.h"
#include "Server.h"

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
			std::cout << "byte_transferred : " << byte_transferred << std::endl;	
			std::copy(m_RecvBuf.begin(), m_RecvBuf.end(), std::back_inserter(m_CircularBuf) );
			std::cout << "Recv : " << m_CircularBuf.size() << std::endl;		
			
			BYTE *pBuf = &m_RecvBuf[0];

			LONG nState = packetBuf.Append(pBuf, byte_transferred);
			if (nState == InPacket::PS_COMPLETE) {
				//ProcessPacket(packetBuf);
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
	}
}


void CConnection::start() {	
	time_t now = time(0);	
	m_sMsg = "Hello";
	boost::asio::async_write(m_Socket, boost::asio::buffer(m_sMsg), 
		boost::bind(&CConnection::handle_Accept, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
		));
}

void CConnection::ProcessPacket(InPacket &iPacket) {

}