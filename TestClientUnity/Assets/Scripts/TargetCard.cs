using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class TargetCard : MonoBehaviour {

    public delegate void OnTouchEventDelegate(int _cardIdx);
    public event OnTouchEventDelegate eventTouch;    
    public int cardIdx;

    // Use this for initialization
    void Start()
    {

    }

    // Update is called once per frame
    void Update()
    {
    }

    public void OnBtnSelected()
    {
        eventTouch(cardIdx);
    }

    public void SetInfo(int _gameIdx, string _sNick)
    {
        cardIdx = _gameIdx;        
    }
}
