#pragma once

template<class T>
class shared_const_buffer
{
public:
	// Construct from a `shared_ptr`
	explicit shared_const_buffer(boost::shared_ptr<std::vector<T>> const& p_data)
		: data_(p_data),
		buffer_(boost::asio::buffer(*data_))
	{}

	// Implement the ConstBufferSequence requirements.
	using value_type = boost::asio::const_buffer;
	using const_iterator = boost::asio::const_buffer const*;

	boost::asio::const_buffer const* begin() const { return &buffer_; }
	boost::asio::const_buffer const* end() const { return &buffer_ + 1; }

private:
	boost::shared_ptr<std::vector<T>> data_;
	boost::asio::const_buffer buffer_;
};

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
	std::string DecodeStr();

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
	void EncodeStr(std::string s);

	void MakeBuf(boost::shared_ptr< std::vector<BYTE> > vData);
};