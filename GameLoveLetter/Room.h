#pragma once

class CRoom : public Object {
public:
	enum { USER_MIN = 3, USER_MAX = 4 };
	static LONG s_nSN;

	CRoom();
	~CRoom();

public:
	struct CPUInfo {
		std::string sName;
	};

	enum {
		FLAG_READY = 0x01,
		FLAG_WITHOUT_ROOM_MASTER = 0x7fffffff,
		FLAG_ROOM_MASTER = 0x80000000,
	};

	void Enter(CUser::pointer pUser);
	void SendEnterPacket(CUser::pointer pUser);
	void BroadcastPacket(OutPacket& oPacket);
	void BroadcastRoomState(DWORD dwFlag = FLAG_WITHOUT_ROOM_MASTER);

	void Update(LONG tCur);
	void Destroy();

	void OnGamePacket(InPacket& iPacket, CUser::pointer pUser);

	void Start(CUser::pointer pUser);	//	���� ����

	LONG GetUserCount();
	void RemoveUser(CUser::pointer pUser);	

	std::vector < CUser::pointer >& GetUsers() { return m_vUsers; }
	std::vector < CPUInfo >& GetCPUs() { return m_vCPUs; }
	CUser::pointer GetUser(LONG nUserSN) {
		if (m_mUsers.find(nUserSN) == m_mUsers.end()) return NULL;
		return m_mUsers.at(nUserSN);
	}

	CUser::pointer GetMaster() const { return m_pMaster; }

private:	

	std::vector < CUser::pointer > m_vUsers;
	std::vector < CPUInfo > m_vCPUs;
	std::map < ULONG, CUser::pointer > m_mUsers;
	CUser::pointer m_pMaster;	//	����
	BOOL m_bGameStart;	
	LONG m_nSN;

	CGameDealerLoveLetter::pointer m_pDealer;	//	���� ����

public:
	typedef boost::shared_ptr<CRoom> pointer;
	LONG GetSN() const { return m_nSN; }
	void ResetReady();
	BOOL IsGameRunning() const { return m_bGameStart; }
protected:
	void RegisterCPU();
	LONG GetPlayerCount() const;
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
	boost::mutex m_LockMutex;
};
