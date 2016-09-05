using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class TargetPlayer : MonoBehaviour {

    public delegate void OnTouchEventDelegate(int _gameIdx);
    public event OnTouchEventDelegate eventTouch;
    public Text lbNick;    
    int playerIdx = -1;    

	// Use this for initialization
	void Start () {
        	
	}
	
	// Update is called once per frame
	void Update () {	
	}    

    public void OnBtnSelected()
    {
        eventTouch(playerIdx);
    }

    public void SetInfo(int _gameIdx, string _sNick) {
        playerIdx = _gameIdx;
        lbNick.text = _sNick;
    }
}
