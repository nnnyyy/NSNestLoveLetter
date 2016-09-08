using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNetwork;
using UnityEngine.SceneManagement;

public class Lobby : MonoBehaviour {

    public GameObject ContentRoot;
    public Object PrefebRoomObject;
    public UIMsgBox msgBox;
    [SerializeField]
    private GameObject lbMsgNoRoom;

	// Use this for initialization
	void Start () {
        SoundManager.Instance.PlayBGM("bgm1");
        Receiver.ClearLobbyEvent();
        ScreenFade.Fade(1, 0, 0.5f, 0, true, () =>
        {
            NetworkUnityEvent.Instance.curMsgBox = msgBox;            
            Receiver.OnRoomListRetCallback += OnRoomListRet;
            Receiver.OnEnterRoomRetCallback += OnEnterRoomRet;
            Receiver.OnCreateRoomRetCallback += OnCreateRoomRet;
            Receiver.OnRoomStateCallback += OnRoomState;
            Sender.RoomListRequest();
        });        
    }
	
	// Update is called once per frame
	void Update () {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            msgBox.ShowYesNo("게임을 종료하시겠습니까?", () =>
            {
                Application.Quit();
                return;
            });
        }
    }

    void InitRoomList()
    {
        Debug.Log("InitRoomList");
        List<Transform> liDelete = new List<Transform>();
        foreach (Transform child in ContentRoot.transform) {
            Debug.Log("InitRoomList1 - " + child);
            liDelete.Add(child);
        }
        Debug.Log("InitRoomList2");
        foreach (Transform t in liDelete)
        {            
            DestroyObject(t.gameObject);
        }
    }

    void AddRoom(int nRoomIdx, int nUserCnt)
    {
        GameObject newObject = (GameObject)GameObject.Instantiate(PrefebRoomObject, Vector3.zero, Quaternion.identity);
        newObject.transform.SetParent(ContentRoot.transform);
        newObject.transform.localScale = new Vector3(1, 1, 1);
        RoomBtn newRoomBtn = newObject.GetComponent<RoomBtn>();
        newRoomBtn.SetRoomInfo(nRoomIdx, nUserCnt);
    }

    public void OnBtnCreateRoom()
    {
        SoundManager.Instance.PlaySfx("btnNormal");
        Sender.CreateRoom();
    }

    public void OnBtnBackToLobby()
    {
        msgBox.ShowYesNo("로그아웃 하시겠습니까?", () =>
        {
            NetworkUnityEvent.Instance.Disconnect();
            GlobalData.Instance.ClearData();
            Receiver.ClearLobbyEvent();
            SceneManager.LoadScene("MainMenu");
        });        
    }

    public void OnBtnRefresh()
    {
        SoundManager.Instance.PlaySfx("btnNormal");
        Sender.RoomListRequest();
    }

    public void OnCreateRoomRet(GCPCreateRoomRet createRoomRet)
    {
        if(createRoomRet.result == GCPCreateRoomRet.eReuslt.Success)
        {
            GlobalData.Instance.roomSN = createRoomRet.sn;            
        }
        else
        {
            msgBox.Show("방 생성에 실패 했습니다.");
        }
    }

    public void OnEnterRoomRet(GCPEnterRoomRet enterRoomRet)
    {
        if( enterRoomRet.result == GCPEnterRoomRet.eResult.Fail)
        {
            msgBox.Show("방 입장에 실패 했습니다.");
            Sender.RoomListRequest();
        }
    }

    public void OnRoomState(GCPRoomState roomState)
    {
        ScreenFade.Fade(0, 1, 0.5f, 0, true, () =>
        {
            Receiver.ClearLobbyEvent();
            SceneManager.LoadScene("Game");
        });        
    }

    public void OnRoomListRet(GCPRoomListRet roomListRet)
    {
        InitRoomList();
        if (roomListRet.listRooms == null || roomListRet.listRooms.Count == 0) {
            lbMsgNoRoom.SetActive(true);
            return;
        }

        foreach(GCPRoomListRet.RoomInfo info in roomListRet.listRooms)
        {
            AddRoom(info.sn, info.userCount);
        }        
    }
}
