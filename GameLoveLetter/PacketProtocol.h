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
	CGP_LL_GuardCheck,		//	경비병 액션
	CGP_LL_RoyalSubject,	//	신하 액션
	CGP_LL_Gossip,			//	험담가 액션
	CGP_LL_Companion,		//	동료 액션
	CGP_LL_Hero,			//	영웅 액션
	CGP_LL_Wizard,			//	마법사 액션
	CGP_LL_Lady,			//	귀부인 액션
	CGP_LL_Princess,		//	공주 액션
	CGP_LL_Emotion,		//	이모션
	CGP_LL_Action,		//	선택하기 전의 엑션	
};

enum {
	//	러브레터 패킷
	
	GCP_LL_InitStatus,	//	게임 초기 상태 정보
	GCP_LL_Status,		//	현재 턴 등을 보내줌
	GCP_LL_ActionRet,	//	카드 사용 결과
	GCP_LL_RoundResult,	//	한 라운드 결과
	GCP_LL_FinalResult,	//	최종 결과
	GCP_LL_Aborted,	//	게임 중간 종료
	GCP_LL_Emotion, //  이모션 결과
};