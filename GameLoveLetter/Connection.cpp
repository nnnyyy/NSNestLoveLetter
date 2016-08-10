#include "stdafx.h"
#include <iostream>
#include <string>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "Connection.h"

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
			std::cout << "Recv! " << std::endl;			
		}

		m_Socket.async_read_some(
			boost::asio::buffer(m_RecvBuf),
			boost::bind(&CConnection::handle_Read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
				));
	}
	else {
		std::cout << m_nID << "Disconnect(Write) : " << err.message() << std::endl;
	}
}


void CConnection::start(int _nID) {
	m_nID = _nID;
	time_t now = time(0);	
	m_sMsg = "";

	boost::asio::async_write(m_Socket, boost::asio::buffer(m_sMsg), 
		boost::bind(&CConnection::handle_Accept, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
		));
}