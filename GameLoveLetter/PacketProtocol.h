#pragma once
enum {
	CGP_Login = 0,
	CGP_HeartBeat,
	CGP_User_Start,
	CGP_CreateRoom = CGP_User_Start,
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
	GCP_CreateRoomRet,
	GCP_EnterRoomRet,
	GCP_LeaveRoomRet,
	GCP_RoomState,	
	GCP_GameStartRet,
	GCP_GameLoveLetter,
};

enum {
	//	���극�� ��Ŷ
	GCP_LL_Action,		//	�����ϱ� ���� ����	
	GCP_LL_Status,		//	�� ���� ����	
	GCP_LL_GuardCheck,	//	��� �׼�
};