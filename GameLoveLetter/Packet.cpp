#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "Packet.h"


InPacket::InPacket() : m_Buf(256) {
	Clear();
}

InPacket::~InPacket() {

}

void InPacket::Clear() {
	m_nLength = 0;
	m_nOffset = 0;
}

LONG InPacket::Append(BYTE* &pByte, size_t byteRead) {
	std::memcpy(&m_Buf[0], pByte, 2);
	pByte += 2;
	USHORT n = *reinterpret_cast<const USHORT*>(&m_Buf[0]);
	std::cout << "Data Size : " << n << std::endl;

	return m_nState;
}