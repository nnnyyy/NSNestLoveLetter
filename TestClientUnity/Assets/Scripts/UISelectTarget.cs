using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class UISelectTarget : MonoBehaviour {
    public delegate void btnBackCallback();
    public event btnBackCallback btnBackEvent;
    public Transform ContentRoot;
    public Object PrefebTargetObject;

    // Use this for initialization
    void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    public void InitTargetList()
    {        
        List<Transform> liDelete = new List<Transform>();
        foreach (Transform child in ContentRoot.transform) {liDelete.Add(child);}        
        foreach (Transform t in liDelete){DestroyObject(t.gameObject);}
    }

    public void AddTarget(int nGameIdx, string sNick, TargetPlayer.OnTouchEventDelegate _delegate)
    {
        GameObject newObject = (GameObject)GameObject.Instantiate(PrefebTargetObject, Vector3.zero, Quaternion.identity);
        newObject.transform.SetParent(ContentRoot.transform);
        newObject.transform.localScale = new Vector3(1, 1, 1);
        TargetPlayer newTarget = newObject.GetComponent<TargetPlayer>();
        newTarget.eventTouch += _delegate;
        newTarget.SetInfo(nGameIdx, sNick);
    }

    public void OnBtnBack()
    {
        if (btnBackEvent != null) {
            btnBackEvent();
        }
    }
}
