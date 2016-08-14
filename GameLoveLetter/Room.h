#pragma once

class CRoom : public Object, boost::enable_shared_from_this<CRoom> {
public:
	enum { USER_MIN = 2, USER_MAX = 4 };
	static LONG s_nSN;

	CRoom();
	~CRoom();

public:

	void Enter(CUser::pointer pUser);
	void SendEnterPacket(CUser::pointer pUser);
	void BroadcastPacket(OutPacket& oPacket);

	void Update(LONG tCur);
	void Destroy();

	LONG GetUserCount();
	void RemoveUser(CUser::pointer pUser);	

private:
	std::vector < CUser::pointer > m_vUsers;
	BOOL m_bGameStart;	
	LONG m_nSN;

public:
	typedef boost::shared_ptr<CRoom> pointer;
	LONG GetSN() const { return m_nSN; }
};

class CRoomManager : public boost::serialization::singleton<CRoomManager> {
public:
	CRoomManager() {}
	~CRoomManager() {}	

public:
	static CRoom::pointer MakeRoom();
	void Register(CRoom::pointer pRoom);
	CRoom::pointer GetRoom(LONG nSN);

	void Update();	

protected:
	std::vector<CRoom::pointer> m_vRooms;
	std::map<LONG, CRoom::pointer> m_mRooms;
};
