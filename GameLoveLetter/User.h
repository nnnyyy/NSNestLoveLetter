#pragma once

class CUser : public boost::enable_shared_from_this<CUser> {

public:
	CUser();
	~CUser();

protected:

	ULONG m_uSocketSN;		

public:
	typedef boost::shared_ptr<CUser> pointer;
	typedef boost::weak_ptr<CUser> wpointer;
	LONG GetCharacterID() const;	
	void SendPacket(OutPacket& oPacket);

	void SetConnection(ULONG uSocketSN);
	void PostDisconnect();

	void OnCreateRoom(InPacket &iPacket);
	void OnEnterRoom(InPacket &iPacket);
	void OnLeaveRoom(InPacket &iPacket);
	void OnGameStart(InPacket &iPacket);
	void OnGameReady(InPacket &iPacket);

protected:
	Object::pointer m_pRoom;
};