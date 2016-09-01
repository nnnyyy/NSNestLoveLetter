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
            newUser.m_nGameIndex = gameIndex;
            newUser.infoUI = infoRemote;
            newUser.infoUI.SetNickName(userInfo.nickName);
            newUser.infoUI.SetReadyStateMsg("");
            newUser.infoUI.SetWinLoseMsg("");
            m_mUser.Add(gameIndex, newUser);
            m_aUser.Add(newUser);
        }        
    }

    void SetCallback()
    {        
        Receiver.OnRoomStateCallback += OnRoomState;
        Receiver.OnLeaveRoomRetCallback += OnLeaveRoomRet;
        Receiver.OnGameStartRetCallback += OnGameStartRet;
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

    }
}