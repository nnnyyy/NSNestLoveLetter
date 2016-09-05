using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using System.Collections.Generic;

public class UIGameLog : MonoBehaviour {
    public GameObject prefebLog;
    public GameObject ContentRoot;

	// Use this for initialization
	void Start () {
        Reset();
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    public void Reset() {
        List<Transform> liDelete = new List<Transform>();
        foreach (Transform child in ContentRoot.transform) { liDelete.Add(child); }
        foreach (Transform t in liDelete) { DestroyObject(t.gameObject); }
    }

    public void AddLog(string sLog)
    {
        GameObject newLog = GameObject.Instantiate(prefebLog);
        newLog.GetComponent<Text>().text = sLog;
        newLog.transform.SetParent(ContentRoot.transform);
        newLog.transform.localPosition = Vector3.zero;
        newLog.transform.localScale = new Vector3(1, 1, 1);

        Canvas.ForceUpdateCanvases();
        GetComponentInChildren<ScrollRect>().verticalNormalizedPosition = 0.0f;
        Canvas.ForceUpdateCanvases();
    }
}
