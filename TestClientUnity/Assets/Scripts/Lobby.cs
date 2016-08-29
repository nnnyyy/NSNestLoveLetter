using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Lobby : MonoBehaviour {

    public GameObject ContentRoot;
    public Object PrefebRoomObject;
    public UIMsgBox msgBox;

	// Use this for initialization
	void Start () {
        InitRoomList();
        AddRoom(0, 1);
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    void InitRoomList()
    {
        List<Transform> liDelete = new List<Transform>();
        foreach (Transform child in ContentRoot.transform) { liDelete.Add(child); }
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

    }

    public void OnBtnBackToLobby()
    {
        msgBox.Show("로비로 돌아가시겠습니까?");
    }

    public void OnBtnRefresh()
    {

    }
}
