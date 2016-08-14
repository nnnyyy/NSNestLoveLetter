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
	m_nDataLen = 0;
	m_nState = PS_HEADER;
}

LONG InPacket::Append(BYTE* &pByte, size_t &byteRead) {

	BYTE *pSrc = pByte;
	size_t nAppendTotal = 0;
	if (m_nState == PS_HEADER) {		
		if (m_nLength < HEADER_SIZE) {
			size_t nAppend = std::min(byteRead, size_t(HEADER_SIZE - m_nLength));
			AppendInner(pSrc, nAppend);
			nAppendTotal += nAppend;

			if (m_nLength >= HEADER_SIZE) {				
				m_nDataLen = Decode2();
			}

			if ((byteRead -= nAppend) == 0) {
				pByte = 0;
				return m_nState;
			}

			pSrc += nAppend;			
		}

		m_nState = PS_DATA;
	}

	size_t nAppend = std::min(byteRead, size_t(HEADER_SIZE + m_nDataLen - m_nLength));
	AppendInner(pSrc, nAppend);
	nAppendTotal += nAppend;

	if (m_nLength >= (HEADER_SIZE + m_nDataLen)) { m_nState = PS_COMPLETE; }
	if ((byteRead -= nAppend) == 0) {
		pByte = 0;
	}
	else {
		pByte += nAppendTotal;
	}

	return m_nState;
}

void InPacket::AppendInner(BYTE *pByte, size_t nAppendSize) {
	std::memcpy((&m_Buf[0]) + m_nLength, pByte, nAppendSize);
	m_nLength += nAppendSize;
}

BYTE InPacket::Decode1() {
	BYTE n = *reinterpret_cast<const BYTE*>(&m_Buf[0] + m_nOffset);
	m_nOffset += sizeof(BYTE);
	return n;
}

USHORT InPacket::Decode2() {
	USHORT n = *reinterpret_cast<const USHORT*>(&m_Buf[0] + m_nOffset);
	m_nOffset += sizeof(USHORT);
	return n;
}

ULONG InPacket::Decode4() {
	ULONG n = *reinterpret_cast<const ULONG*>(&m_Buf[0] + m_nOffset);
	m_nOffset += sizeof(ULONG);
	return n;
}

UINT64 InPacket::Decode8() {
	UINT64 n = *reinterpret_cast<const UINT64*>(&m_Buf[0] + m_nOffset);
	m_nOffset += sizeof(UINT64);
	return n;
}


OutPacket::OutPacket(LONG nType) : m_Buf(256)  {
	Encode2(nType);
}

OutPacket::~OutPacket() {

}

void OutPacket::Encode1(BYTE n) {
	BYTE *pSrc = (&m_Buf[0] + m_nOffset);
	*reinterpret_cast<BYTE*>(pSrc) = n;
	m_nOffset += sizeof(BYTE);
}

void OutPacket::Encode2(USHORT n) {
	BYTE *pSrc = (&m_Buf[0] + m_nOffset);
	*reinterpret_cast<USHORT*>(pSrc) = n;
	m_nOffset += sizeof(USHORT);
}

void OutPacket::Encode4(ULONG n) {
	BYTE *pSrc = (&m_Buf[0] + m_nOffset);
	*reinterpret_cast<ULONG*>(pSrc) = n;
	m_nOffset += sizeof(ULONG);
}

void OutPacket::Encode8(UINT64 n) {
	BYTE *pSrc = (&m_Buf[0] + m_nOffset);
	*reinterpret_cast<UINT64*>(pSrc) = n;
	m_nOffset += sizeof(UINT64);
}

void OutPacket::MakeBuf(std::vector<BYTE> &v) {
	v.resize(HEADER_SIZE + m_nOffset);
	*reinterpret_cast<USHORT*>(&v[0]) = m_nOffset;
	std::copy(m_Buf.begin(), m_Buf.begin() + m_nOffset, v.begin() + HEADER_SIZE);
}