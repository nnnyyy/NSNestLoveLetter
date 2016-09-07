#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "GameData.h"
#include "Packet.h"
#include "User.h"
#include "Connection.h"
#include "Server.h"

std::map<ULONG, boost::shared_ptr<CUser> > Server_Wrapper::m_mUsers;

Server::Server(boost::asio::io_service& io) : m_Acceptor(io, tcp::endpoint(tcp::v4(),7770)), m_dwSocketSNCounter(0), m_nAcceptCnt(0) {
	WaitAccept();
}

void Server_Wrapper::Update() { m_pServer->Update(); }

void Server::WaitAccept() {
	CConnection::pointer newConn = CConnection::create(m_Acceptor.get_io_service());	

	m_Acceptor.async_accept(newConn->socket(), 
		boost::bind( &Server::handle_Accept, this, newConn, boost::asio::placeholders::error )
	);
}

void Server::handle_Accept(CConnection::pointer newConn, const boost::system::error_code& err) {
	if (!err) {
		m_LockMutex.lock();
		newConn->m_uSocketSN = m_dwSocketSNCounter;
		m_dwSocketSNCounter++;
		m_mSockets.insert(std::pair<ULONG, CConnection::pointer>(newConn->m_uSocketSN, newConn ));		
		m_LockMutex.unlock();
		//m_lSockets.push_back(newConn);

		newConn->start();
		WaitAccept();
	}
}

void Server::RemoveSocket(CConnection::pointer pSocket) {
	boost::lock_guard<boost::mutex> lock(m_LockMutex);
	m_mSockets.erase(pSocket->m_uSocketSN);
}

void Server::Update() {	
	boost::lock_guard<boost::mutex> lock(m_LockMutex);
	std::map<ULONG, CConnection::pointer>::iterator it = m_mSockets.begin();
	for (; it != m_mSockets.end(); ++it) {
		CConnection::pointer pConn = it->second;		
		pConn->Update();		
	}
}