#pragma once

class InPacket {
public:
	InPacket();
	~InPacket();
	enum { PS_HEADER, PS_DATA, PS_COMPLETE };
	enum { HEADER_SIZE = 2 };

private:

	std::vector<BYTE> m_Buf;
	LONG m_nLength;
	LONG m_nOffset;
	LONG m_nState;
	LONG m_nDataLen;

public:

	void Clear();
	LONG Append(BYTE* &pByte, size_t &byteRead);

	BYTE Decode1();
	USHORT Decode2();
	ULONG Decode4();
	UINT64 Decode8();

private:
	void AppendInner(BYTE *pByte, size_t nAppendSize);
};

class OutPacket {
public:
	enum { HEADER_SIZE = 2 };
	OutPacket(LONG nType);
	~OutPacket();

private:

	std::vector<BYTE> m_Buf;
	LONG m_nOffset;

public:

	void Encode1(BYTE n);
	void Encode2(USHORT n);
	void Encode4(ULONG n);
	void Encode8(UINT64 n);	
	void EncodeStr(std::string n);

	void MakeBuf(std::vector<BYTE> &v);
};