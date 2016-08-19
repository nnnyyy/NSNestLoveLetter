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

using namespace boost::chrono;

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

	if (status.bRoundOver) {
		//	���尡 ������ �Ǹ� ���� �׼� ��Ŷ�� ���� �ʴ´�.
		return;
	}
	
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
	if (m_vPlayers.size() <= nTargetIdx) return;

	Player::pointer pTargetPlayer = m_vPlayers[nTargetIdx];
	if (!pTargetPlayer) {
		return;
	}

	if (nCardTypeGuess == LOVELETTER_GAURD) {
		//	����� ���� ������ �� ����.
		return;
	}
	
	if (FindTarget(pTurnPlayer) && nTargetIdx == status.nCurTurnIndex) {
		//	�� �ڽ��� ������ �� ����.
		return;
	}

	if (pTargetPlayer->m_bDead) {
		//	���� ����� �������� ����.
		return;
	}

	if (pTargetPlayer->m_bGuard) {
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GAURD);
	if (!bDrop) {
		//	����
		return;
	}	

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	if (m_vPlayers[nTargetIdx]->m_vHandCards[0]->m_nType == nCardTypeGuess) {
		Dead(m_vPlayers[nTargetIdx]);
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
		oPacket.Encode4(nTargetIdx);
		oPacket.Encode4(nCardTypeGuess);
		pRoom->BroadcastPacket(oPacket);
	}

	Next();
	Process();
}

void CGameDealerLoveLetter::OnRoyalSubjectAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	LONG nTargetIdx = iPacket.Decode4();	//	�������� ��� ���ΰ�	
	if (m_vPlayers.size() <= nTargetIdx) return;
	Player::pointer pTargetPlayer = m_vPlayers[nTargetIdx];

	if (FindTarget(pTurnPlayer) && nTargetIdx == status.nCurTurnIndex) {
		//	�� �ڽ��� ������ �� ����.
		return;
	}

	if (pTargetPlayer->m_bDead) {
		//	���� ����� �������� ����.
		return;
	}

	if (pTargetPlayer->m_bGuard) {
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
			oPacket.Encode4(pTargetPlayer->m_vHandCards[0]->m_nType);
		}

		CUser::pointer pUser = pRoom->GetUser(m_vPlayers[i]->nUserSN);
		pUser->SendPacket(oPacket);
	}

	Next();
	Process();
}

void CGameDealerLoveLetter::OnGossipAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	LONG nWho = iPacket.Decode4();
	if (m_vPlayers.size() <= nWho) return;
	Player::pointer pTargetPlayer = m_vPlayers[nWho];

	if (pTargetPlayer->m_bDead) {
		//	���� �ڵ� Ÿ���� �� �� ����.
		return;
	}

	if (pTargetPlayer->m_bGuard) {
		return;
	}

	//	���
	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_GOSSIP);
	if (!bDrop) {
		//	����
		return;
	}	

	Card::pointer pTargetHandCard = pTargetPlayer->m_vHandCards[0];
	Card::pointer pMyHandCard = pTurnPlayer->m_vHandCards[0];

	BOOL bRet = FALSE;
	LONG nDeadIndex = -1;
	LONG nAliveIndex = -1;
	if (pTargetHandCard->m_nType > pMyHandCard->m_nType) {
		//	���� ����
		Dead(pTurnPlayer);
		bRet = TRUE;
		nDeadIndex = status.nCurTurnIndex;
		nAliveIndex = nWho;
	}
	else if (pTargetHandCard->m_nType < pMyHandCard->m_nType) {
		//	������ ����
		Dead(pTargetPlayer);
		bRet = TRUE;
		nDeadIndex = nWho;
		nAliveIndex = status.nCurTurnIndex;
	}
	else {
		//	��ܼ� �ƹ��ϵ� �Ͼ�� ����.
		nAliveIndex = nWho;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_GOSSIP);
	oPacket.Encode4(bRet);
	oPacket.Encode4(nAliveIndex);
	if (bRet) {
		oPacket.Encode4(nDeadIndex);		
	}	
	pRoom->BroadcastPacket(oPacket);

	Next();
	Process();
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

	pTurnPlayer->m_bGuard = TRUE;

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_COMPANION);
	oPacket.Encode4(status.nCurTurnIndex);
	pRoom->BroadcastPacket(oPacket);


	Next();
	Process();
}

void CGameDealerLoveLetter::OnHeroAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	LONG nWho = iPacket.Decode4();
	if (m_vPlayers.size() <= nWho) return;
	Player::pointer pTargetPlayer = m_vPlayers[nWho];
	if (pTargetPlayer->m_bDead) {
		//	���� �ڵ� Ÿ���� �� �� ����.
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_HERO);
	if (!bDrop) {
		//	����
		return;
	}

	LONG nDroppedCardType = DropAndGetNewCardByHero(pTargetPlayer);
	BOOL bKill = FALSE;
	if (nDroppedCardType == LOVELETTER_PRINCESS) {
		Dead(pTargetPlayer);
		bKill = TRUE;
	}

	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_ActionRet);
	oPacket.Encode4(LOVELETTER_HERO);
	oPacket.Encode4(status.nCurTurnIndex);	// From
	oPacket.Encode4(nWho);					// To
	oPacket.Encode4(bKill);					// Princess Drop Kill
	pRoom->BroadcastPacket(oPacket);

	Next();
	Process();
}

void CGameDealerLoveLetter::OnWizardAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	LONG nWho = iPacket.Decode4();
	if (m_vPlayers.size() <= nWho) return;
	Player::pointer pTargetPlayer = m_vPlayers[nWho];
	if (pTargetPlayer->m_bDead) {
		//	���� �ڵ� Ÿ���� �� �� ����.
		return;
	}	

	if (FindTarget(pTurnPlayer) && nWho == status.nCurTurnIndex) {
		//	Ÿ���� �����ִµ� ������ ���°� �ȵ�
		return;
	}

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_WIZARD);
	if (!bDrop) {
		//	����
		return;
	}	

	// ��ȯ ó��
	ExchangeCard(pTurnPlayer, pTargetPlayer);

	Next();
	Process();
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

	Next();
	Process();
}

void CGameDealerLoveLetter::OnPrincessAction(InPacket& iPacket, CUser::pointer pUser) {
	Player::pointer pTurnPlayer = m_vPlayers[status.nCurTurnIndex];
	if (pTurnPlayer->nUserSN != pUser->m_nUserSN) {
		//	���� �ƴ� ������� �׼� ��Ŷ�� ���� ��ȿó�� 
		return;
	}

	//	���� ������ ���� ���� ���� ������ �����Ƿ� ����
	return;

	BOOL bDrop = DropCard(pTurnPlayer, LOVELETTER_PRINCESS);
	if (!bDrop) {
		//	����
		return;
	}

	Next();
	Process();
}

LONG CGameDealerLoveLetter::DropAndGetNewCardByHero(Player::pointer pTargetPlayer) {
	//	�׻� 1���� �־�� �Ѵ�.
	BOOST_ASSERT(pTargetPlayer->m_vHandCards.size() == 1);
	Card::pointer pCardDropped = pTargetPlayer->m_vHandCards[0];
	pTargetPlayer->m_vHandCards.clear();
	pTargetPlayer->m_vGroundCards.push_back(pCardDropped);
	if (pCardDropped->m_nType != LOVELETTER_PRINCESS) {
		GetCardFromDeck(pTargetPlayer);
	}	
	return pCardDropped->m_nType;
}

BOOL CGameDealerLoveLetter::DropCard(Player::pointer pTurnPlayer, LONG nCardType) {
	//	�׻� 2���� �־�� �Ѵ�.
	BOOST_ASSERT(pTurnPlayer->m_vHandCards.size() == 2);

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

void CGameDealerLoveLetter::ExchangeCard(Player::pointer pPlayer1, Player::pointer pPlayer2) {
	BOOST_ASSERT(pPlayer1->m_vHandCards.size() == 1 && pPlayer2->m_vHandCards.size() == 1);
	Card::pointer pCard1 = pPlayer1->m_vHandCards[0];
	Card::pointer pCard2 = pPlayer2->m_vHandCards[0];
	pPlayer1->m_vHandCards.clear();
	pPlayer1->m_vHandCards.push_back(pCard2);
	pPlayer2->m_vHandCards.clear();
	pPlayer2->m_vHandCards.push_back(pCard1);
}

void CGameDealerLoveLetter::Dead(Player::pointer pPlayer) {
	pPlayer->m_bDead = TRUE;
	if (!pPlayer->m_vHandCards.empty()) {
		Card::pointer pCard = pPlayer->m_vHandCards.back();
		pPlayer->m_vHandCards.pop_back();
		BOOST_ASSERT(pPlayer->m_vHandCards.empty());		
		pPlayer->m_vGroundCards.push_back(pCard);
	}	
}

BOOL CGameDealerLoveLetter::FindTarget(Player::pointer pTurnPlayer) {
	for each (Player::pointer p in m_vPlayers)
	{
		if (p != pTurnPlayer && !p->m_bDead && !p->m_bGuard) {
			return TRUE;
		}
	}

	return FALSE;
}

void CGameDealerLoveLetter::Update() {		
	if (status.bRoundOver && 
		duration_cast<milliseconds>(system_clock::now() - status.tRoundOverStart).count() >= GameStatus::WAIT_NEXT_ROUND_TIME ) {
		if (status.bFinalOver) {
			AllReset();
		}
		else {
			InitGame();
			Process();
		}		
		return;
	}
}

void CGameDealerLoveLetter::InitGame() {	
	//	���� �ʱ�ȭ
	//	������ ���ϰ� ī�带 ���´�.
	//SetUserSeq();
	status.bRoundOver = FALSE;
	if (status.nPrevRoundWinIndex != -1) {
		status.nCurTurnIndex = status.nPrevRoundWinIndex;
		status.bFinalOver = FALSE;
	}
	else {
		status.nCurTurnIndex = 0;
	}
	

	if (m_vPlayers.empty()) {
		CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
		std::vector < CUser::pointer > &vUsers = pRoom->GetUsers();
		LONG idx = 0;
		for (std::vector < CUser::pointer >::iterator iter = vUsers.begin(); iter != vUsers.end(); ++iter) {
			Player::pointer p(new Player());
			p->Init();
			p->nUserSN = (*iter)->m_nUserSN;
			p->m_bDead = FALSE;
			m_vPlayers.push_back(p);
			p->m_nIndex = idx;
			idx++;
		}
	}
	else {
		for each (Player::pointer p in m_vPlayers)
		{
			p->Init();
		}
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

BOOL CGameDealerLoveLetter::CheckGameOver() {

	if (m_vDeck.size() <= 0) {
		GameOver(GAMEOVER_DECK_IS_EMPTY);
		return TRUE;
	}

	LONG nDeadCnt = 0;	
	for each (Player::pointer p in m_vPlayers)
	{
		if (p->m_bDead) nDeadCnt++;
	}

	if (m_vPlayers.size() - 1 <= nDeadCnt) {
		GameOver(GAMEOVER_ALL_DEAD);
		return TRUE;
	}

	return FALSE;
}

void CGameDealerLoveLetter::GameOver(LONG nReason) {
	//	�� �Լ� ������ �� �̻� ���� ��Ŷ�� ���� �ʰ� �÷��׸� �����ϰ�,
	//	���� �̰���� ���� ����� �������� �Ѵ�.
	//	���尡 ���� �ִٸ� ���� ���带 �����ϰ�
	//	�װ͵� �� �̰����� ���� ��� ó���� �Ѵ�.	
	Player::pointer pWinner;
	if (nReason == GAMEOVER_DECK_IS_EMPTY) {		
		for each (Player::pointer p in m_vPlayers)
		{
			if (p->m_bDead) continue;
			if (pWinner == NULL) {
				pWinner = p;
				continue;
			}
			else {
				if (pWinner->m_vHandCards[0]->m_nType < p->m_vHandCards[0]->m_nType) {
					pWinner = p;
					continue;
				}
			}
		}
	}

	if (nReason == GAMEOVER_ALL_DEAD) {		
		for each (Player::pointer p in m_vPlayers)
		{
			if (p->m_bDead) continue;
			if (pWinner == NULL) {
				pWinner = p;
				break;
			}			
		}
	}

	if (pWinner) {
		//	�� ���� �� ������ ������
		pWinner->m_nRoundWin++;
		status.bRoundOver = TRUE;
		status.tRoundOverStart = system_clock::now();

		if (pWinner->m_nRoundWin >= 3 /* 4���̸� ���� 3�� ������ */) {
			status.bFinalOver = TRUE;
			SendFinalRoundOver();			
			return;
		}
		else {			
			CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
			OutPacket oPacket(GCP_GameLoveLetter);
			oPacket.Encode2(GCP_LL_RoundResult);
			oPacket.Encode4(pWinner->m_nIndex);
			pRoom->BroadcastPacket(oPacket);
		}		
	}
	else {
		//	���� �ȵ�.
		//	�����̳� ������ ����.
	}
}

void CGameDealerLoveLetter::Next() {
	status.nCurTurnIndex++;
	if (status.nCurTurnIndex >= m_vPlayers.size()) {
		status.nCurTurnIndex = 0;
	}
}

void CGameDealerLoveLetter::Process() {

	//	���� ����� �� ������ ������ �����Ѵ�.
	if( CheckGameOver() ) { return; }

	if (CheckDead()) {
		Process();
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
	BOOST_ASSERT(pPlayer->m_vHandCards.size() == 2 || pPlayer->m_vHandCards.size() == 1);
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

void CGameDealerLoveLetter::AllReset() {
	//	
	status.Reset();
	m_vPlayers.clear();

}

void CGameDealerLoveLetter::SendFinalRoundOver() {
	CRoom::pointer pRoom = boost::dynamic_pointer_cast<CRoom>(m_pRoom);
	OutPacket oPacket(GCP_GameLoveLetter);
	oPacket.Encode2(GCP_LL_FinalResult);
	pRoom->BroadcastPacket(oPacket);
	pRoom->ResetReady();
	pRoom->BroadcastRoomState();
}