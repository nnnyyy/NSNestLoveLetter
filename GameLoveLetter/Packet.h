#pragma once

class InPacket {
public:
	InPacket();
	~InPacket();
	enum { PS_HEADER, PS_DATA, PS_COMPLETE };

private:

	std::vector<BYTE> m_Buf;
	LONG m_nLength;
	LONG m_nOffset;
	LONG m_nState;

public:

	void Clear();
	LONG Append(BYTE* &pByte, size_t byteRead);
};