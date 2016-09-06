using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNetwork;
using UnityEngine.UI;

public class GameLoveLetterMan : MonoBehaviour {

    public UIMsgBox msgBox;
    public UISelectTarget selectTarget;
    public UISelectCard selectCard;

    public Transform tfGrave;    
    public List<GameUser> m_aUser;
    public Dictionary<int, GameUser> m_mUser;   //  gameIndex to User
    public Dictionary<int, int> dSN_to_GameIdx;
    public bool bInteractable = false;

    public UIGameLog gameLog;
    public Button btnEmotion;

    private int nCurProcessCardNum;
    public bool isTouchProcessing { get; set; }

    private int nSelectedTargetIdx = -1;

    // Use this for initialization
    void Start () {
        m_aUser = new List<GameUser>();
        m_mUser = new Dictionary<int, GameUser>();
        selectTarget.btnBackEvent += OnBtnBackFromSelectTarget;
        selectCard.btnCardSelectEvent += OnBtnCardSelect;
        selectCard.btnBackEvent += OnBtnBackFromSelectCard;        
        Receiver.ClearEmotionEvent();
        Receiver.OnLLEmotionCallback += OnEmotionRet;
        btnEmotion.gameObject.SetActive(false);
    }
	
	// Update is called once per frame
	void Update () {	
	}

    bool PreprocessCard(int _cardNum)
    {
        switch (nCurProcessCardNum)
        {
            case 4:
                Sender.LLCompanion();
                isTouchProcessing = true;
                return true;
            case 7:
                Sender.LLLady();
                isTouchProcessing = true;
                return true;

            default:
                return false;                
        }
    }

    public void ProcessCard(int _cardNum)
    {
        if (isTouchProcessing) return;

        nCurProcessCardNum = _cardNum;
        if (PreprocessCard(nCurProcessCardNum))
        {
            return;
        }

        GameUser me = null;
        foreach (GameUser u in m_mUser.Values)
        {
            if (GlobalData.Instance.userSN == u.m_nUserSN)
            {
                me = u;
                break;
            }
        }

        if(_cardNum == 5 || _cardNum == 6)
        {            
            foreach (Card c in me.infoUI.liCardHand)
            {
                if(c.m_nNum == 7)
                {
                    msgBox.Show("왕비를 먼저 사용해야 합니다");
                    return;
                }
            }
        }        

        selectTarget.gameObject.SetActive(true);
        selectTarget.InitTargetList();
        int nTarget = 0;
        
        foreach(GameUser u in m_mUser.Values)
        {            
            if (u.infoUI.bDead || 
                u.infoUI.bShield || 
                (_cardNum != 5 && GlobalData.Instance.userSN == u.m_nUserSN) ) continue;

            selectTarget.AddTarget(u.m_nGameIndex, u.infoUI.m_lbName.text, OnSelectTarget);
            nTarget++;
        }

        if(nTarget == 0)
        {
            selectTarget.AddTarget(me.m_nGameIndex, me.infoUI.m_lbName.text, OnSelectTarget);
        }
    }

    void NextProcess()
    {
        selectTarget.gameObject.SetActive(false);
        selectCard.gameObject.SetActive(true);
    }

    public void OnSelectTarget(int _gameIdx)
    {
        selectTarget.gameObject.SetActive(false);
        isTouchProcessing = false;
        switch (nCurProcessCardNum)
        {
            case 1:
                nSelectedTargetIdx = _gameIdx;
                NextProcess();                
                break;

            case 2:
                Sender.LLRoyalSubject(_gameIdx);
                isTouchProcessing = true;
                break;

            case 3:
                Sender.LLGossip(_gameIdx);
                isTouchProcessing = true;
                break;

            case 4:
                Sender.LLCompanion();
                isTouchProcessing = true;
                break;

            case 5:
                Sender.LLHero(_gameIdx);
                isTouchProcessing = true;
                break;

            case 6:
                Sender.LLWizard(_gameIdx);
                isTouchProcessing = true;
                break;

            case 7:
                Sender.LLLady();
                isTouchProcessing = true;
                break;

            case 8:
                break;
        }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
    }

    public void ResetUIForGame(GCPGameStartRet startRet, UserLocalInfo local, UserRemoteInfo[] aRemote)
    {
        btnEmotion.gameObject.SetActive(true);
        int nLocalIdx = startRet.dSN_to_GameIdx[GlobalData.Instance.userSN];
        dSN_to_GameIdx = startRet.dSN_to_GameIdx;

        int nUserCnt = startRet.dSN_to_GameIdx.Count;

        List<int> liGameIndex = new List<int>();
        int nLocalNext = (nLocalIdx + 1) % nUserCnt;
        for (int i = nLocalNext; i < nUserCnt; ++i)
        {
            liGameIndex.Add(i);
        }

        for (int i = 0; i < nLocalIdx; ++i)
        {
            liGameIndex.Add(i);
        }

        tfGrave.gameObject.SetActive(true);
        ClearUI();
        m_mUser.Clear();
        m_aUser.Clear();

        GameUser newLocalUser = new GameUser();
        newLocalUser.m_bLocal = true;
        int gameIndex = nLocalIdx;
        int SN = startRet.dGameIdx_to_SN[gameIndex];
        GCPRoomState.UserInfo userInfo = null;
        foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
        {
            if (u.sn == SN)
            {
                userInfo = u;
                break;
            }
        }

        UserLocalInfo info = local;
        info.m_nGameIndex = gameIndex;
        info.m_nUserSN = SN;
        newLocalUser.m_nGameIndex = gameIndex;
        newLocalUser.m_nUserSN = SN;
        newLocalUser.infoUI = info;
        newLocalUser.infoUI.SetNickName(userInfo.nickName);
        newLocalUser.infoUI.SetReadyStateMsg("");
        newLocalUser.infoUI.SetWinLoseMsg("");
        m_mUser.Add(gameIndex, newLocalUser);
        m_aUser.Add(newLocalUser);

        for (int i = 0; i < (nUserCnt - 1); ++i)
        {
            GameUser newUser = new GameUser();
            newUser.m_bLocal = false;
            gameIndex = liGameIndex[i];
            SN = startRet.dGameIdx_to_SN[gameIndex];
            userInfo = null;
            foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
            {
                if (u.sn == SN)
                {
                    userInfo = u;
                    break;
                }
            }

            UserRemoteInfo infoRemote = aRemote[i];
            infoRemote.m_nGameIndex = gameIndex;
            infoRemote.m_nUserSN = SN;
            newUser.m_nGameIndex = gameIndex;
            newUser.m_nUserSN = SN;
            newUser.infoUI = infoRemote;
            newUser.infoUI.SetNickName(userInfo.nickName);
            newUser.infoUI.SetReadyStateMsg("");
            newUser.infoUI.SetWinLoseMsg("");
            m_mUser.Add(gameIndex, newUser);
            m_aUser.Add(newUser);
        }
    }

    public void OnLLInitStatus(GCPLLInitStatus status)
    {
        gameLog.AddLog("새로운 라운드가 시작 되었습니다");

        foreach (GCPLLInitStatus.PlayerInfo pinfo in status.listPlayer)
        {
            int gidx = dSN_to_GameIdx[pinfo.userSN];
            GameUser guser = m_mUser[gidx];            
            Debug.Log(guser.m_nGameIndex);
            guser.infoUI.Refresh(pinfo);
        }
    }

    public void OnLLStatus(GCPLLStatus status)
    {
        GameUser turnUser = m_mUser[status.currentTurnUserIndex];
        if (turnUser.m_bLocal)
        {
            bInteractable = true;
        }
        else
        {
            bInteractable = false;
        }

        foreach(GameUser u in m_mUser.Values)
        {
            u.infoUI.SetMyTurn(u.m_nGameIndex == status.currentTurnUserIndex);
        }        
        turnUser.infoUI.SetShield(false);
        Card c = CardManager.CreateCard(turnUser.m_bLocal ? status.currentTurnUserGetCardIndex : 0);
        c.transform.position = tfGrave.position;
        turnUser.infoUI.PutHand(c);
        isTouchProcessing = false;
    }

    string str_green(string s)
    {
        return "<color=green>" + s + "</color>";
    }

    public void OnLLActionRet(GCPLLActionRet action)
    {
        string sSrcNick="";
        string sTargetNick="";
        string sUsedCardName = "<color=yellow>" + Card.GetName(action.nCardType) + "</color>";
        switch (action.nCardType)
        {
            case 1:
                sSrcNick = str_green(m_mUser[action.nSrcIdx].GetNickName());
                sTargetNick = str_green(m_mUser[action.nTargetIdx].GetNickName());
                string sCardName = "<color=yellow>" + Card.GetName(action.nCardIdx) + "</color>";

                m_mUser[action.nSrcIdx].infoUI.DropCard(action.nCardType);                
                gameLog.AddLog("["+ sUsedCardName + "] " + sSrcNick + "이 " + sTargetNick + "을 " + sCardName + "라고 지목 했습니다.");
                if (action.bSucceed)
                {
                    m_mUser[action.nTargetIdx].infoUI.DropCard(action.nCardIdx);
                    m_mUser[action.nTargetIdx].infoUI.Dead();
                    gameLog.AddLog("[" + sUsedCardName + "] " + sTargetNick + "이 사망하였습니다!");
                }
                else {
                    gameLog.AddLog("[" + sUsedCardName + "] 아무 일도 일어나지 않았습니다");
                }              

                break;

            case 2:
                sSrcNick = str_green(m_mUser[action.nSrcIdx].GetNickName());
                sTargetNick = str_green(m_mUser[action.nTargetIdx].GetNickName());
                m_mUser[action.nSrcIdx].infoUI.DropCard(action.nCardType);

                gameLog.AddLog("[" + sUsedCardName + "] " + sSrcNick + "이(가) " + sTargetNick + "에게 물어봤습니다.");
                if (action.bMyTurn)
                {
                    gameLog.AddLog("["+ sUsedCardName + "] " + sTargetNick + "의 카드는 " + "<color=yellow>" + Card.GetName(action.nCardIdx) + "</color>" + "입니다!!");
                }
                break;

            case 3:
                sSrcNick = str_green(m_mUser[action.nSrcIdx].GetNickName());
                sTargetNick = str_green(m_mUser[action.nTargetIdx].GetNickName());
                m_mUser[action.nSrcIdx].infoUI.DropCard(action.nCardType);
                gameLog.AddLog("[" + sUsedCardName + "] " + sSrcNick + "이(가) " + sTargetNick + "에게 공격합니다.");
                if (action.nRet == 1)
                {
                    gameLog.AddLog("[" + sUsedCardName + "] " + sSrcNick + "이(가) 승리 했습니다.");
                    m_mUser[action.nTargetIdx].infoUI.DropCard(action.nDeadCardIdx);
                    m_mUser[action.nTargetIdx].infoUI.Dead();
                }
                else if (action.nRet == -1)
                {
                    gameLog.AddLog("[" + sUsedCardName + "] " + sTargetNick + "가 승리 했습니다.");
                    m_mUser[action.nSrcIdx].infoUI.DropCard(action.nDeadCardIdx);
                    m_mUser[action.nSrcIdx].infoUI.Dead();
                }
                else
                {
                    gameLog.AddLog("[" + sUsedCardName + "] 아무 일도 없었습니다.");
                }
                break;

            case 4:
                sSrcNick = str_green(m_mUser[action.nSrcIdx].GetNickName());                
                gameLog.AddLog("[" + sUsedCardName + "] " + sSrcNick + "이(가) 보호막을 사용합니다.");
                m_mUser[action.nSrcIdx].infoUI.DropCard(action.nCardType);
                m_mUser[action.nSrcIdx].infoUI.SetShield(true);
                break;

            case 5:
                sSrcNick = str_green(m_mUser[action.nSrcIdx].GetNickName());
                sTargetNick = str_green(m_mUser[action.nTargetIdx].GetNickName());
                gameLog.AddLog("[" + sUsedCardName + "] " + sSrcNick + "이(가) " + sTargetNick + "에게 사용합니다.");
                m_mUser[action.nSrcIdx].infoUI.DropCard(action.nCardType);
                if(action.nDropCardIdx == -1)
                {
                    gameLog.AddLog("[" + sUsedCardName + "] 더 이상 드롭할 카드가 없습니다.");
                    return;
                }
                m_mUser[action.nTargetIdx].infoUI.DropCard(action.nDropCardIdx);
                if (action.nDropCardIdx == 8)
                {
                    m_mUser[action.nTargetIdx].infoUI.Dead();
                    gameLog.AddLog("[" + sUsedCardName + "] " + sTargetNick + "이(가) 사망했습니다.");
                }
                else
                {
                    if (action.bTargetPlayer)
                    {
                        Card c = CardManager.CreateCard(action.nNewCard);
                        m_mUser[action.nTargetIdx].infoUI.PutHand(c);
                    }
                    else
                    {
                        Card c = CardManager.CreateCard(0);
                        m_mUser[action.nTargetIdx].infoUI.PutHand(c);
                    }
                }
                break;

            case 6:
                sSrcNick = str_green(m_mUser[action.nSrcIdx].GetNickName());
                sTargetNick = str_green(m_mUser[action.nTargetIdx].GetNickName());
                gameLog.AddLog("[" + sUsedCardName + "] " + sSrcNick + "이 " + sTargetNick + "와 교환합니다.");
                m_mUser[action.nSrcIdx].infoUI.DropCard(action.nCardType);
                if (action.nSrcIdx == action.nTargetIdx)
                {
                    return;
                }

                if (action.bSrcOrTarget)
                {
                    UserInfoBase srcUI = m_mUser[action.nSrcIdx].infoUI;
                    UserInfoBase targetUI = m_mUser[action.nTargetIdx].infoUI;
                    srcUI.SendCard(targetUI, action.nSrcToTargetCardIdx);
                    targetUI.SendCard(srcUI, action.nSrcToTargetCardIdx);
                }
                break;

            case 7:
                sSrcNick = str_green(m_mUser[action.nSrcIdx].GetNickName());
                sTargetNick = str_green(m_mUser[action.nTargetIdx].GetNickName());
                gameLog.AddLog("[" + sUsedCardName + "] " + sSrcNick + "이 왕비를 버립니다. 무슨 일 일까요?");
                m_mUser[action.nSrcIdx].infoUI.DropCard(action.nCardType);
                break;
        }
    }

    public void OnRoundResult(GCPLLRoundResult ret)
    {
        gameLog.Reset();
        if(ret.nReason == 0)
        {
            tfGrave.gameObject.SetActive(false);
        }        
        string sWinner = m_mUser[ret.winUserIndex].GetNickName();
        gameLog.AddLog(sWinner + "이(가) 승리했습니다! 축하합니다!");
    }

    public void OnFinalRoundResult(GCPLLFinalResult ret)
    {
        gameLog.Reset();
        GameUser winner = m_mUser[ret.winUserIndex];
        string sWinner = winner.GetNickName();
        winner.infoUI.AddTokken();
        gameLog.AddLog(sWinner + "이(가) 최종 승리했습니다!!!!! 레알 축하합니다!");
        btnEmotion.gameObject.SetActive(false);
    }

    public void OnAborted(GCPLLAborted ret)
    {
        gameLog.AddLog("게임이 도중에 중단 되었습니다.");
        btnEmotion.gameObject.SetActive(false);
    }

    void OnBtnCardSelect(int _cardNum)
    {
        Debug.Log("CardSelected : " + _cardNum);
        Sender.LLGuardCheck(nSelectedTargetIdx, _cardNum);
        selectCard.gameObject.SetActive(false);
        isTouchProcessing = true;
    }

    public void OnBtnBackFromSelectTarget()
    {
        selectTarget.gameObject.SetActive(false);
        isTouchProcessing = false;
    }

    public void OnBtnBackFromSelectCard()
    {
        selectTarget.gameObject.SetActive(true);
        selectCard.gameObject.SetActive(false);        
    }

    public void OnBtnEmotionLaugh()
    {
        int gameIdx = dSN_to_GameIdx[GlobalData.Instance.userSN];
        Sender.LLEmotionLaugh(gameIdx);
    }

    public void OnEmotionRet(GCPLLEmotion packet)
    {
        m_mUser[packet.gameIdx].infoUI.emotion.Laugh();
    }

    void ClearUI()
    {
        foreach(GameUser user in m_mUser.Values)
        {
            user.infoUI.ClearInfo();
        }
    }
}
