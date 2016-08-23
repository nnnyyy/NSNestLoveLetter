#pragma once

//	말그대로 게임 데이터.
//	지금은 러브레터의 데이터만 건드려보자.
class CGameData {
public:
	CGameData() {}
	~CGameData() {}

	//	게임 통계
	LONG m_nWin;				//	승리 횟수
	LONG m_nLose;				//	패배 횟수
	LONG m_nForcedDisconn;		//	강제 절단 횟수
	LONG m_nUseGuard;			//	경비 사용 횟수
	LONG m_nSuccessUseGuard;	//	경비 성공 횟수
	LONG m_nAttackedByGuard;	//	경비에게 당한 횟수
	LONG m_nUseGossip;			//	험담가 사용 횟수
	LONG m_nSuccessUseGossip;	//	험담가 성공 횟수
	LONG m_nAttackedByGossip;	//	험담가에게 당한 횟수
	LONG m_nQueenHeroWizard;	//	왕비가 영웅 or 마법사랑 같이 있어본 횟수
	LONG m_nNoRoundWin;			//	한라운드도 못 딴 횟수
};