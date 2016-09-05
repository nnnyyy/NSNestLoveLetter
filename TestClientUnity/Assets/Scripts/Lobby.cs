using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNetwork;
using UnityEngine.SceneManagement;

public class Lobby : MonoBehaviour {

    public GameObject ContentRoot;
    public Object PrefebRoomObject;
    public UIMsgBox msgBox;

	// Use this for initialization
	void Start () {
        SoundManager.Instance.PlayLoop(0);
        Receiver.OnRoomListRetCallback += OnRoomListRet;
        Receiver.OnEnterRoomRetCallback += OnEnterRoomRet;
        Receiver.OnCreateRoomRetCallback += OnCreateRoomRet;
        Receiver.OnRoomStateCallback += OnRoomState;        
        Sender.RoomListRequest();
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
        SoundManager.instance.PlaySingle(1);
        Sender.CreateRoom();
    }

    public void OnBtnBackToLobby()
    {
        
    }

    public void OnBtnRefresh()
    {
        SoundManager.instance.PlaySingle(1);
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
        }
    }

    public void OnRoomState(GCPRoomState roomState)
    {        
        Receiver.OnRoomListRetCallback -= OnRoomListRet;
        Receiver.OnEnterRoomRetCallback -= OnEnterRoomRet;
        Receiver.OnCreateRoomRetCallback -= OnCreateRoomRet;
        Receiver.OnRoomStateCallback -= OnRoomState;
        SceneManager.LoadScene("Game");
    }

    public void OnRoomListRet(GCPRoomListRet roomListRet)
    {
        InitRoomList();
        foreach(GCPRoomListRet.RoomInfo info in roomListRet.listRooms)
        {
            AddRoom(info.sn, info.userCount);
        }        
    }
}
