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

public:
	typedef boost::shared_ptr<CUser> pointer;
	typedef boost::weak_ptr<CUser> wpointer;	
	void SendPacket(OutPacket& oPacket);

	void SetConnection(ULONG uSocketSN);
	void PostDisconnect();

	void OnRoomListRequest(InPacket &iPacket);
	void OnCreateRoom(InPacket &iPacket);	
	void OnEnterRoom(InPacket &iPacket);
	void OnLeaveRoom(InPacket &iPacket);
	void OnGameStart(InPacket &iPacket);
	void OnGameReady(InPacket &iPacket);

protected:
	Object::pointer m_pRoom;
	//GameData m_UserData;		// DB에 플러싱 해야 할 기록들(최종 승리 횟수, 험담가 시도 수, 험담가 적중 횟수, )
public:
	Object::pointer GetRoom() const { return m_pRoom; }
};