#pragma once

//	���״�� ���� ������.
//	������ ���극���� �����͸� �ǵ������.
class CGameData {
public:
	CGameData() {}
	~CGameData() {}

	//	���� ���
	LONG m_nWin;				//	�¸� Ƚ��
	LONG m_nLose;				//	�й� Ƚ��
	LONG m_nForcedDisconn;		//	���� ���� Ƚ��
	LONG m_nUseGuard;			//	��� ��� Ƚ��
	LONG m_nSuccessUseGuard;	//	��� ���� Ƚ��
	LONG m_nAttackedByGuard;	//	��񿡰� ���� Ƚ��
	LONG m_nUseGossip;			//	��㰡 ��� Ƚ��
	LONG m_nSuccessUseGossip;	//	��㰡 ���� Ƚ��
	LONG m_nAttackedByGossip;	//	��㰡���� ���� Ƚ��
	LONG m_nQueenHeroWizard;	//	�պ� ���� or ������� ���� �־ Ƚ��
	LONG m_nNoRoundWin;			//	�Ѷ��嵵 �� �� Ƚ��
};