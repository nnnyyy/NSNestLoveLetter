using UnityEngine;
using System.Collections;
using DG.Tweening;
using System.Collections.Generic;
using NSNetwork;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

public class Game : MonoBehaviour {
    public UIMsgBox msgBox;
    static public Transform s_tfBaseForConv;
    static public Transform s_tfRoot;
    static public Vector3 ConvPosToRootLocal(Vector3 vSrcPos)
    {
        s_tfBaseForConv.position = vSrcPos;
        return s_tfBaseForConv.localPosition;
    }

    static public Vector3 ConvPosToRootGlobal(Vector3 vSrcPos)
    {
        s_tfBaseForConv.position = vSrcPos;
        return s_tfBaseForConv.position;
    }

    public UserLocalInfo m_LocalUser;
    public UserRemoteInfo[] m_aRemoteUsers;
    public Transform tfTestBase;
    public Transform tfBase;
    public Transform tfGrave;

    //  게임 관련
    bool isGameRunning = false;
    public List<GameUser> m_aUser;
    public Dictionary<int, GameUser> m_mUser;   //  gameIndex to User
    public Dictionary<int, int> dSN_to_GameIdx;

    public Text lbTitle;
    public Button btnReadyOrStart;

    // Use this for initialization
    void Start () {
        SetCallback();        
        s_tfBaseForConv = tfTestBase;
        s_tfRoot = tfBase;
        CardManager.Init();
        m_aUser = new List<GameUser>();
        m_mUser = new Dictionary<int, GameUser>();
        lbTitle.text = "Room : " + GlobalData.Instance.roomSN;
        Refresh();        
    }

    // Update is called once per frame
    void Update () {
            	
	}

    public void OnTouch(int nType, int nID, float x, float y, float dx, float dy)
    {
        Collider2D coll;
        Vector3 ray = Camera.main.ScreenToWorldPoint(new Vector3(x, y, 1));
        if (coll = Physics2D.OverlapPoint(new Vector2(ray.x, ray.y), 0x300))
        {
            if(coll.gameObject.CompareTag("Card"))
            {
                Debug.Log("Card Processing");
            }
            coll.transform.SendMessage("Selected");
        }
    }

    public void OnBtnReadyOrStart()
    {
        if( GlobalData.Instance.IsRoomMaster() )
        {
            if (GlobalData.Instance.roomUsers.Count < 4) return;
            foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
            {
                if(u.readyState == 0)
                {
                    return;
                }
            }
            Sender.GameStart();
        }
        else
        {
            Sender.GameReady();
        }
    }

    public void OnBtnLeave()
    {
        msgBox.ShowYesNo("방에서 나가시겠습니까?", ()=> {
            Sender.LeaveRoom();
        });        
    }

    public void OnLeaveRoomRet(GCPLeaveRoomRet leaveRoomRet)
    {
        RemoveCallback();
        SceneManager.LoadScene("Lobby");
    }

    void Refresh()
    {        
        ClearUI();

        if (GlobalData.Instance.roomUsers == null) return;
        int idx = 0;
        foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
        {
            UserInfoBase uib;
            if (u.sn == GlobalData.Instance.userSN)
            {
                uib = m_LocalUser;
            }
            else
            {
                uib = m_aRemoteUsers[idx];
                idx++;
            }
            
            uib.m_lbName.text = u.nickName;
            if(GlobalData.Instance.IsRoomMaster())
            {
                uib.m_lbReadyState.text = "Master";
            }
            else
            {
                uib.m_lbReadyState.text = u.readyState == 1 ? "Ready" : "Not Ready";
            }  
        }

        if(GlobalData.Instance.IsRoomMaster())
        {
            btnReadyOrStart.GetComponentInChildren<Text>().text = "Start!";
            if (GlobalData.Instance.roomUsers.Count < 4)
            {
                btnReadyOrStart.interactable = false;
            }
            else
            {
                btnReadyOrStart.interactable = true;
            }
        }
        else
        {
            GCPRoomState.UserInfo me = null;
            foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
            {
                if(u.sn == GlobalData.Instance.userSN) { me = u;  break; }
            }
            btnReadyOrStart.GetComponentInChildren<Text>().text = me.readyState == 1 ? "Cancel" : "Ready";
        }
    }

    void ClearUI()
    {
        m_LocalUser.ClearInfo();
        foreach (UserInfoBase uib in m_aRemoteUsers)
        {
            uib.ClearInfo();
        }
    }

    public void OnRoomState(GCPRoomState roomState)
    {        
        Refresh();
    }

    public void OnGameStartRet(GCPGameStartRet gameStartRet)
    {
        if(gameStartRet.result != 0)
        {
            return;
        }

        ResetUIForGame(gameStartRet);
        GameStart();        
    }

    void ResetUIForGame(GCPGameStartRet startRet)
    {
        int nLocalIdx = startRet.dSN_to_GameIdx[GlobalData.Instance.userSN];
        dSN_to_GameIdx = startRet.dSN_to_GameIdx;

        List<int> liGameIndex = new List<int>();
        int nLocalNext = (nLocalIdx + 1) % 4;
        for(int i = nLocalNext; i < 4; ++i)
        {
            liGameIndex.Add(i);
        }

        for(int i = 0; i < nLocalIdx; ++i)
        {
            liGameIndex.Add(i);
        }

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

        UserLocalInfo info = m_LocalUser;
        info.m_nGameIndex = gameIndex;
        info.m_nUserSN = SN;
        newLocalUser.m_nGameIndex = gameIndex;
        newLocalUser.infoUI = info;
        newLocalUser.infoUI.SetNickName(userInfo.nickName);
        newLocalUser.infoUI.SetReadyStateMsg("");
        newLocalUser.infoUI.SetWinLoseMsg("");
        m_mUser.Add(gameIndex, newLocalUser);
        m_aUser.Add(newLocalUser);

        for (int i = 0; i < 3; ++i)
        {
            GameUser newUser = new GameUser();
            newUser.m_bLocal = false;
            gameIndex = liGameIndex[i];
            SN = startRet.dGameIdx_to_SN[gameIndex];
            userInfo = null;
            foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
            {
                if(u.sn == SN)
                {
                    userInfo = u;
                    break;
                }
            }

            UserRemoteInfo infoRemote = m_aRemoteUsers[i];
            infoRemote.m_nGameIndex = gameIndex;
            infoRemote.m_nUserSN = SN;
            newUser.m_nGameIndex = gameIndex;
            newUser.infoUI = infoRemote;
            newUser.infoUI.SetNickName(userInfo.nickName);
            newUser.infoUI.SetReadyStateMsg("");
            newUser.infoUI.SetWinLoseMsg("");
            m_mUser.Add(gameIndex, newUser);
            m_aUser.Add(newUser);
        }        
    }

    void OnLLInitStatus(GCPLLInitStatus status)
    {
        Debug.Log("현재 턴 : " + status.currentTurnUserIndex);

        foreach(GCPLLInitStatus.PlayerInfo pinfo in status.listPlayer)
        {
            int gidx = dSN_to_GameIdx[pinfo.userSN];
            GameUser guser = m_mUser[gidx];
            Debug.Log(guser.m_nGameIndex);
            guser.infoUI.Refresh(pinfo);
        }
    }

    void OnLLStatus(GCPLLStatus status) {
        GameUser turnUser = m_mUser[status.currentTurnUserIndex];
        turnUser.infoUI.SetShield(false);
        Card c = CardManager.CreateCard(turnUser.m_bLocal ? status.currentTurnUserGetCardIndex : 0);
        turnUser.infoUI.PutHand(c);
    }

    void OnLLActionRet(GCPLLActionRet action)
    {
        switch (action.nCardType)
        {
            case 1:
                m_mUser[action.nSrcIdx].infoUI.DropCard(1);
                if (action.bSucceed)
                {
                    Debug.Log("Dead - " + action.nTargetIdx);
                    m_mUser[action.nTargetIdx].infoUI.DropCard(action.nCardIdx);
                    //m_mUser[action.nTargetIdx].infoUI.Dead();
                }
                break;

            case 2:
                m_mUser[action.nSrcIdx].infoUI.DropCard(2);
                if (action.bMyTurn)
                {
                    // AddGameLog();
                    Debug.Log("[2] - " + action.nCardIdx);
                }
                break;

            case 3:
                m_mUser[action.nSrcIdx].infoUI.DropCard(3);
                if(action.nRet == 1)
                {
                    m_mUser[action.nTargetIdx].infoUI.DropCard(action.nDeadCardIdx);
                    //m_mUser[action.nTargetIdx].infoUI.Dead();
                }
                else if(action.nRet == -1)
                {
                    m_mUser[action.nSrcIdx].infoUI.DropCard(action.nDeadCardIdx);
                    //m_mUser[action.nSrcIdx].infoUI.Dead();
                }
                else
                {

                }
                break;

            case 4:
                m_mUser[action.nSrcIdx].infoUI.DropCard(4);
                m_mUser[action.nSrcIdx].infoUI.SetShield(true);
                break;

            case 5:
                m_mUser[action.nSrcIdx].infoUI.DropCard(5);
                m_mUser[action.nTargetIdx].infoUI.DropCard(action.nDropCardIdx);
                if (action.nDropCardIdx == 8)
                {
                    //m_mUser[action.nTargetIdx].infoUI.Dead();                    
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
                m_mUser[action.nSrcIdx].infoUI.DropCard(6);
                if(action.nSrcIdx == action.nTargetIdx)
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
                m_mUser[action.nSrcIdx].infoUI.DropCard(7);
                break;
        }
    }

    void SetCallback()
    {        
        Receiver.OnRoomStateCallback += OnRoomState;
        Receiver.OnLeaveRoomRetCallback += OnLeaveRoomRet;
        Receiver.OnGameStartRetCallback += OnGameStartRet;
        Receiver.OnLLInitStatusCallback += OnLLInitStatus;
        Receiver.OnLLActionRetCallback += OnLLActionRet;
        Receiver.OnLLStatusCallback += OnLLStatus;
        TouchMan.Instance.ResetEvent();
        TouchMan.Instance.begin0 += OnTouch;
        TouchMan.Instance.move0 += OnTouch;
        TouchMan.Instance.end0 += OnTouch;
        /*begin0 += OnTouch;
        move0 += OnTouch;
        end0 += OnTouch;*/
    }

    void RemoveCallback()
    {
        Receiver.OnRoomStateCallback -= OnRoomState;
        Receiver.OnLeaveRoomRetCallback -= OnLeaveRoomRet;
        Receiver.OnGameStartRetCallback -= OnGameStartRet;
        Receiver.OnLLInitStatusCallback -= OnLLInitStatus;
        Receiver.OnLLActionRetCallback -= OnLLActionRet;
        Receiver.OnLLStatusCallback -= OnLLStatus;
        TouchMan.Instance.ResetEvent();
        TouchMan.Instance.begin0 -= OnTouch;
        TouchMan.Instance.move0 -= OnTouch;
        TouchMan.Instance.end0 -= OnTouch;
        /*begin0 -= OnTouch;
        move0 -= OnTouch;
        end0 -= OnTouch;*/
    }

    void GameStart()
    {
        btnReadyOrStart.gameObject.SetActive(false);
    }
}