#include "stdafx.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <algorithm>
#include <random>
#include "PacketProtocol.h"
#include "Packet.h"
#include "User.h"
#include "GameDealer.h"
#include "Room.h"

CGameDealer::CGameDealer() {
	
}

CGameDealer::~CGameDealer() {

}

LONG card_attribute[] = {
	LOVELETTER_GAURD,
	LOVELETTER_GAURD,
	LOVELETTER_GAURD,
	LOVELETTER_GAURD,
	LOVELETTER_GAURD,
	LOVELETTER_ROYAL,
	LOVELETTER_ROYAL,
	LOVELETTER_GOSSIP,
	LOVELETTER_GOSSIP,
	LOVELETTER_COMPANION,
	LOVELETTER_COMPANION,
	LOVELETTER_HERO,
	LOVELETTER_HERO,
	LOVELETTER_WIZARD,
	LOVELETTER_LADY,
	LOVELETTER_PRINCESS
};

CGameDealerLoveLetter::CGameDealerLoveLetter() {
	LONG nSize = sizeof(card_attribute) / sizeof(LONG);
	for (int i = 0; i < nSize; ++i) {
		Card::pointer p(new Card(card_attribute[i]));
		m_vCards.push_back(p);
	}	
}

CGameDealerLoveLetter::~CGameDealerLoveLetter() {

}

void CGameDealerLoveLetter::OnPacket(InPacket& iPacket, CUser::pointer pUser) {
	LONG nSubType = iPacket.Decode2();

	switch (nSubType) {
	case CGP_LL_GuardCheck: OnGuardAction(iPacket, pUser); break;
	case CGP_LL_RoyalSubject: OnRoyalSubjectAction(iPacket, pUser); break;
	case CGP_LL_Gossip: OnGossipAction(iPacket, pUser); break;
	case CGP_LL_Companion: OnCompanionAction(iPacket, pUser); break;
	case CGP_LL_Hero: OnHeroAction(iPacket, pUser); break;
	case CGP_LL_Wizard: OnWizardAction(iPacket, pUser); break;
	case CGP_LL_Lady: OnLadyAction(iPacket, pUser); break;
	case CGP_LL_Princess: OnPrincessAction(iPacket, pUser); break;
	}
}

void CGameDealerLoveLetter::OnGuardAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	LONG nTargetIdx		= iPacket.Decode4();	//	����?
	LONG nCardTypeGuess = iPacket.Decode4();	//	����?

	if (nCardTypeGuess == LOVELETTER_GAURD) {
		//	����� ���� ������ �� ����.
		return;
	}

	if (nTargetIdx == status.nCurTurnIndex) {
		//	�� �ڽ��� ������ �� ����.
		return;
	}

	if (m_vPlayers[nTargetIdx]->m_bDead) {
		//	���� ����� �������� ����.
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GAURD);
	if (!bDrop) {
		//	����
		return;
	}if (CheckDead()) {
		NextTurn();
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (m_vPlayers[nTargetIdx]->m_vHandCards[0]->m_nType == nCardTypeGuess) {
		m_vPlayers[nTargetIdx]->m_bDead = TRUE;
		Card::pointer pCard = m_vPlayers[nTargetIdx]->m_vHandCards.back();
		m_vPlayers[nTargetIdx]->m_vHandCards.pop_back();
		m_vPlayers[nTargetIdx]->m_vGroundCards.push_back(pCard);
		//	��Ҵ�.		
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_ActionRet);
		oPacket.Encode4(LOVELETTER_GAURD);		
		oPacket.Encode4(TRUE);
		oPacket.Encode4(nTargetIdx);
		pRoom->BroadcastPacket(oPacket);
	}
	else {
		//	�ƹ��ϵ� �Ͼ�� �ʴ´�.
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_ActionRet);
		oPacket.Encode4(LOVELETTER_GAURD);
		oPacket.Encode4(FALSE);		
		pRoom->BroadcastPacket(oPacket);
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;		
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnRoyalSubjectAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	LONG nTargetIdx = iPacket.Decode4();	//	�������� ��� ���ΰ�	

	if (nTargetIdx == status.nCurTurnIndex) {
		//	�� �ڽ��� ������ �� ����.
		return;
	}

	if (m_vPlayers[nTargetIdx]->m_bDead) {
		//	���� ����� �������� ����.
		return;
	}	

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_ROYAL);
	if (!bDrop) {
		//	����
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);	

	LONG nSize = m_vPlayers.size();	
	for (int i = 0; i < nSize; ++i) {		
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_ActionRet);
		oPacket.Encode4(LOVELETTER_ROYAL);
		BOOL bMyTurn = IsMyTurn(m_vPlayers[i]);
		oPacket.Encode4(bMyTurn);
		if (bMyTurn) {
			oPacket.Encode4(m_vPlayers[nTargetIdx]->m_vHandCards[0]->m_nType);
		}

		CUser::pointer pUser = pRoom->GetUser(m_vPlayers[i]->nUserSN);
		pUser->SendPacket(oPacket);
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnGossipAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GOSSIP);
	if (!bDrop) {
		//	����
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnCompanionAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_COMPANION);
	if (!bDrop) {
		//	����
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnHeroAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_HERO);
	if (!bDrop) {
		//	����
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnWizardAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_WIZARD);
	if (!bDrop) {
		//	����
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnLadyAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_LADY);
	if (!bDrop) {
		//	����
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

void CGameDealerLoveLetter::OnPrincessAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_PRINCESS);
	if (!bDrop) {
		//	����
		return;
	}

	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}

	NextTurn();
}

BOOL CGameDealerLoveLetter::DropCard(Player::pointer pTurnPlayer, LONG nCardType) {
	BOOL bFind = FALSE;
	std::vector<Card::pointer>::iterator iterFind;
	LONG nFindCardIndex = -1;
	Card::pointer pCardDropped;
	for (std::vector<Card::pointer>::iterator iter = pTurnPlayer->m_vHandCards.begin(); iter != pTurnPlayer->m_vHandCards.end(); ++iter) {
		if ((*iter)->m_nType == nCardType) {
			bFind = TRUE;
			pCardDropped = *iter;
			iterFind = iter;
			//pTurnPlayer->m_vHandCards.erase(iter);
			break;
		}
	}

	if (!bFind)
	{
		return FALSE;
	}
	
	if (nCardType == LOVELETTER_HERO || nCardType == LOVELETTER_WIZARD) {
		//	�������� ī�尡 �����̳� ���ڵ��� ��,
		//	������ ī�忡 �ͺ����� ������ �ͺ����� ���� ������ ��.
		for each (Card::pointer pCard in pTurnPlayer->m_vHandCards)
		{
			if (pCard->m_nType == LOVELETTER_LADY) {
				return FALSE;
			}
		}
	}	

	pTurnPlayer->m_vHandCards.erase(iterFind);
	pTurnPlayer->m_vGroundCards.push_back(pCardDropped);
	return TRUE;
}

void CGameDealerLoveLetter::Update() {
	
}

void CGameDealerLoveLetter::InitGame() {	
	//	���� �ʱ�ȭ
	//	������ ���ϰ� ī�带 ���´�.

	//SetUserSeq();
	if (status.nPrevRoundWinIndex != -1) {

	}
	//	ù��° ���� ����
	status.nCurTurnIndex = 0;

	m_vPlayers.clear();
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	std::vector < CUser::pointer > &vUsers = pRoom->GetUsers();
	for (std::vector < CUser::pointer >::iterator iter = vUsers.begin(); iter != vUsers.end(); ++iter) {
		Player::pointer p(new Player());
		p->nUserSN = (*iter)->m_nUserSN;
		p->m_bDead = FALSE;
		m_vPlayers.push_back(p);
	}

	//ShuffleCard();
	m_vDeck.clear();
	m_pSecretCard = NULL;
	m_vDeck.resize(m_vCards.size());
	std::copy(m_vCards.begin(), m_vCards.end(), m_vDeck.begin());
	auto engine = std::default_random_engine(std::random_device{}());
	std::shuffle(std::begin(m_vDeck), std::end(m_vDeck), engine);
	
	//DistributeCard();
	//	���� ī�� ���� �����
	m_pSecretCard = m_vDeck.back();
	m_vDeck.pop_back();

	LONG nPlayerNum = m_vPlayers.size();
	for (int i = 0; i < nPlayerNum; ++i) {
		m_vPlayers[i]->m_vHandCards.push_back(m_vDeck.back());
		m_vDeck.pop_back();
	}
}
BOOL CGameDealerLoveLetter::CheckDead() {
	Player::pointer player = m_vPlayers[status.nCurTurnIndex];
	if (player->m_bDead) {
		status.nCurTurnIndex++;
		if (status.nCurTurnIndex >= m_vPlayers.size()) {
			status.nCurTurnIndex = 0;
		}		
		return TRUE;
	}

	return FALSE;
}

void CGameDealerLoveLetter::NextTurn() {

	//	���� ����� �� ������ ������ �����Ѵ�.
	//if( !CheckDeadCount() ) { GameOver(); return; }

	if (CheckDead()) {
		NextTurn();
		return;
	}
	
	Player::pointer player = m_vPlayers[status.nCurTurnIndex];
	//	ī�� ���� �̱�
	player->m_bGuard = FALSE;	//	�� ���� ���ƿ��� ���尡 Ǯ����.
	BOOL bSucceed = GetCardFromDeck(player);
	if (!bSucceed) {
		//	ī�尡 ����.
		//	GameOver(); 
		return;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	LONG nSize = m_vPlayers.size();	
	for (int i = 0; i < nSize; ++i) {		
		OutPacket oPacket(GCP_GameLoveLetter);
		oPacket.Encode2(GCP_LL_Status);
		status.EncodeStatus(oPacket);
		EncodePlayerInfo(oPacket);
		BOOL bMyTurn = IsMyTurn(m_vPlayers[i]);
		oPacket.Encode4(bMyTurn);		
		if (!m_vPlayers[i]->m_bDead) {
			oPacket.Encode4(m_vPlayers[i]->m_vHandCards[0]->m_nType);
			if (bMyTurn) {
				oPacket.Encode4(m_vPlayers[i]->m_vHandCards[1]->m_nType);
			}
		}
		else {
			//	�׾��� ���
		}

		CUser::pointer pUser = pRoom->GetUser(m_vPlayers[i]->nUserSN);
		pUser->SendPacket(oPacket);
	}

	//	���� ���� �÷��̾��� 2��° ī�带 �� �÷��̾�Ը� �����Ѵ�.
	//	������ �÷��̾�� �޸��� ���� ( Ŭ�� �׼ǿ� )
}

void CGameDealerLoveLetter::GameOver() {
	//	�� �Լ� ������ �� �̻� ���� ��Ŷ�� ���� �ʰ� �÷��׸� �����ϰ�,
	//	���� �̰���� ���� ����� �������� �Ѵ�.
	//	���尡 ���� �ִٸ� ���� ���带 �����ϰ�
	//	�װ͵� �� �̰����� ���� ��� ó���� �Ѵ�.
}

void CGameDealerLoveLetter::EncodePlayerInfo(OutPacket& oPacket) {
	LONG nSize = m_vPlayers.size();
	oPacket.Encode4(nSize);
	for (int i = 0; i < nSize; ++i) {
		oPacket.Encode4(m_vPlayers[i]->nUserSN);
		oPacket.Encode4(m_vPlayers[i]->m_bDead);
		oPacket.Encode4(m_vPlayers[i]->m_bGuard);
		LONG nGroundCardSize = m_vPlayers[i]->m_vGroundCards.size();
		oPacket.Encode4(nGroundCardSize);		
		if (nGroundCardSize) {
			for (int i2 = 0; i2 < nGroundCardSize; ++i2) {
				Card::pointer pCard = m_vPlayers[i]->m_vGroundCards[i2];
				oPacket.Encode4(pCard->m_nType);
			}						
		}
	}
}

void CGameDealerLoveLetter::EncodePlayerIndexList(OutPacket& oPacket) {
	LONG nSize = m_vPlayers.size();
	oPacket.Encode4(nSize);
	for (int i = 0; i < nSize; ++i) {
		oPacket.Encode4(m_vPlayers[i]->nUserSN);
		oPacket.Encode4(i);
	}	
}

BOOL CGameDealerLoveLetter::GetCardFromDeck(Player::pointer pPlayer) {
	pPlayer->m_vHandCards.push_back(m_vDeck.back());
	BOOST_ASSERT(pPlayer->m_vHandCards.size() == 2);
	m_vDeck.pop_back();	
	return TRUE;
}

BOOL CGameDealerLoveLetter::IsMyTurn(Player::pointer pPlayer) {
	return m_vPlayers[status.nCurTurnIndex] == pPlayer;
}

void CGameDealerLoveLetter::GameStatus::EncodeStatus(OutPacket& oPacket) {
	oPacket.Encode4(nCurTurnIndex);
}

void CGameDealerLoveLetter::Player::Init() {
	m_vHandCards.clear();
	m_vGroundCards.clear();
	m_bDead = FALSE;
	m_bGuard = FALSE;
}

void CGameDealerLoveLetter::Player::Encode(OutPacket& oPacket) {
	oPacket.Encode1(m_vGroundCards.size());
	oPacket.Encode1(m_bDead);
	oPacket.Encode1(m_bGuard);
}