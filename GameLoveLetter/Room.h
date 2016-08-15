#pragma once

class CRoom : public Object {
public:
	enum { USER_MIN = 2, USER_MAX = 4 };
	static LONG s_nSN;

	CRoom();
	~CRoom();

public:

	enum {
		FLAG_READY = 0x01,
		FLAG_ALL = 0xffff
	};

	void Enter(CUser::pointer pUser);
	void SendEnterPacket(CUser::pointer pUser);
	void BroadcastPacket(OutPacket& oPacket);
	void BroadcastRoomState(DWORD dwFlag = FLAG_ALL);

	void Update(LONG tCur);
	void Destroy();

	void OnGamePacket(InPacket& iPacket, CUser::pointer pUser);

	void Start(CUser::pointer pUser);	//	게임 시작

	LONG GetUserCount();
	void RemoveUser(CUser::pointer pUser);	

	std::vector < CUser::pointer >& GetUsers() { return m_vUsers; }
	CUser::pointer GetUser(LONG nUserSN) {
		if (m_mUsers.find(nUserSN) == m_mUsers.end()) return NULL;
		return m_mUsers.at(nUserSN);
	}

private:
	std::vector < CUser::pointer > m_vUsers;
	std::map < ULONG, CUser::pointer > m_mUsers;
	CUser::pointer m_pMaster;	//	방장
	BOOL m_bGameStart;	
	LONG m_nSN;

	CGameDealerLoveLetter::pointer m_pDealer;	//	게임 딜러

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

	void MakeRoomListPacket(OutPacket& oPacket);

protected:
	std::vector<CRoom::pointer> m_vRooms;
	std::map<LONG, CRoom::pointer> m_mRooms;
};
