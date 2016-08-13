#pragma once
#include "Connection.h"

class CUser : public boost::enable_shared_from_this<CUser> {

public:
	CUser();
	~CUser();

protected:

	ULONG m_uSocketSN;		

public:
	typedef boost::shared_ptr<CUser> pointer;
	LONG GetCharacterID() const;	
	void SendPacket(OutPacket& oPacket);

	void SetConnection(ULONG uSocketSN);
};