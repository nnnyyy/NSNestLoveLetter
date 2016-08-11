#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Connection.h"
#include "Server.h"

Server::Server(boost::asio::io_service& io) : m_Acceptor(io, tcp::endpoint(tcp::v4(),7770)), m_dwSocketSNCounter(0), m_nAcceptCnt(0) {
	WaitAccept();
}

void Server::WaitAccept() {
	CConnection::pointer newConn = CConnection::create(m_Acceptor.get_io_service());	

	m_Acceptor.async_accept(newConn->socket(), 
		boost::bind( &Server::handle_Accept, this, newConn, boost::asio::placeholders::error )
	);
}

void Server::handle_Accept(CConnection::pointer newConn, const boost::system::error_code& err) {
	if (!err) {

		newConn->m_uSocketSN = m_dwSocketSNCounter;
		m_dwSocketSNCounter++;
		m_mSockets.insert(std::pair<ULONG, CConnection::pointer>(newConn->m_uSocketSN, newConn ));		
		m_lSockets.push_back(newConn);

		newConn->start();
		WaitAccept();
	}
}

void Server::RemoveSocket(CConnection::pointer pSocket) {
	m_mSockets.erase(pSocket->m_uSocketSN);
}