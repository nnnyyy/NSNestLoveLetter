#pragma once
enum {
	CGP_Login = 0,
	CGP_Logout,
	CGP_RegisterUser,
	CGP_AliveAck,
	CGP_User_Start,
	CGP_RoomListRequest = CGP_User_Start,
	CGP_CreateRoom,
	CGP_EnterRoom,
	CGP_LeaveRoom,
	CGP_GameStart,
	CGP_GameReady,
	CGP_GamePacket_Start,
		CGP_GameLoveLetter = CGP_GamePacket_Start,
	CGP_GamePacket_End,
	CGP_User_End
};

enum {
	GCP_LoginRet = 0,
	GCP_RegisterUserRet,
	GCP_AliveAck,
	GCP_RoomListRet,
	GCP_CreateRoomRet,
	GCP_EnterRoomRet,
	GCP_LeaveRoomRet,
	GCP_RoomState,	
	GCP_GameStartRet,
	GCP_GameLoveLetter,
};

enum {
	CGP_LL_GuardCheck,		//	��� �׼�
	CGP_LL_RoyalSubject,	//	���� �׼�
	CGP_LL_Gossip,			//	��㰡 �׼�
	CGP_LL_Companion,		//	���� �׼�
	CGP_LL_Hero,			//	���� �׼�
	CGP_LL_Wizard,			//	������ �׼�
	CGP_LL_Lady,			//	�ͺ��� �׼�
	CGP_LL_Princess,		//	���� �׼�
	CGP_LL_Action,		//	�����ϱ� ���� ����	
};

enum {
	//	���극�� ��Ŷ
	
	GCP_LL_InitStatus,	//	���� �ʱ� ���� ����
	GCP_LL_Status,		//	���� �� ���� ������
	GCP_LL_ActionRet,	//	ī�� ��� ���
	GCP_LL_RoundResult,	//	�� ���� ���
	GCP_LL_FinalResult,	//	���� ���
};