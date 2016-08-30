using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

public class UserLocalInfo : UserInfoBase {

    private List<Card> liCard = new List<Card>();

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    public override void PutHand(Card c)
    {
        base.PutHand(c);
        c.transform.SetParent(m_panelHands.transform);        
        c.transform.localScale = new Vector3(0.5f, 0.5f, 1);
        c.transform.localPosition = new Vector3(0, 0, 1);
        liCard.Add(c);
        SortCards();
    }

    void SortCards()
    {
        int idx = 0;
        int nStart = 0 - ((int)m_panelHands.gameObject.GetComponent<RectTransform>().rect.width / 2 );
        foreach(Card c in liCard)
        {
            c.transform.DOLocalMove(new Vector3(nStart + idx * 40, 0, 1), 0.5f);            
            idx++;
        }
    }
}
