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
	//	러브레터 패킷
	GCP_LL_Action,		//	선택하기 전의 엑션	
	GCP_LL_Status,		//	방 상태 정보	
	GCP_LL_GuardCheck,	//	경비병 액션
};