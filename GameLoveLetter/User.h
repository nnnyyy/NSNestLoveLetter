#pragma once

class CUser : public boost::enable_shared_from_this<CUser> {

public:
	CUser();
	~CUser();

protected:

	ULONG m_uSocketSN;	

public:
	ULONG m_nUserSN;
	BOOL m_bReady;
	std::string m_sNick;

public:
	typedef boost::shared_ptr<CUser> pointer;
	typedef boost::weak_ptr<CUser> wpointer;	
	void SendPacket(OutPacket& oPacket);

	void SetConnection(ULONG uSocketSN);
	void PostDisconnect();

	void OnRoomListRequest(InPacket &iPacket);
	void OnCreateRoom(InPacket &iPacket);	
	void OnEnterRoom(InPacket &iPacket);
	void OnCPUFlag(InPacket &iPacket);
	void OnLeaveRoom(InPacket &iPacket);
	void OnGameStart(InPacket &iPacket);
	void OnGameReady(InPacket &iPacket);

protected:
	Object::pointer m_pRoom;	
public:
	Object::pointer GetRoom() const { return m_pRoom; }
public:
	CGameData gamedata;
};