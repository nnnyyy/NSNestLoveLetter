#pragma once


class CRoom : public boost::enable_shared_from_this<CRoom> {
public:
	enum { USER_MIN = 2, USER_MAX = 4 };
	static LONG s_nSN;

	CRoom();
	~CRoom();

public:

	void Enter(CUser::pointer pUser);
	void SendEnterPacket(CUser::pointer pUser);
	void BroadcastPacket(OutPacket& oPacket);

private:
	std::vector < CUser::pointer > m_vUsers;
	BOOL m_bGameStart;
	LONG m_nSN;
};
