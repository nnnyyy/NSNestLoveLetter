using UnityEngine;
using System.Collections;

public class UISelectCard : MonoBehaviour {

    public delegate void btnBackCallback();
    public event btnBackCallback btnBackEvent;
    public delegate void btnCardSelectedCallback(int _cardNum);
    public event btnCardSelectedCallback btnCardSelectEvent;
    public Transform ContentRoot;
    public TargetCard[] cards;    

    // Use this for initialization
    void Start()
    {        
        foreach(TargetCard c in cards)
        {
            c.eventTouch += OnCardSelected;
        }
    }

    // Update is called once per frame
    void Update()
    {
    }

    public void OnCardSelected(int _cardNum)
    {
        if(btnCardSelectEvent != null)
        {
            btnCardSelectEvent(_cardNum);
        }
    }

    public void OnBtnBack()
    {
        if (btnBackEvent != null)
        {
            btnBackEvent();
        }
    }
}
