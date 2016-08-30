using UnityEngine;
using System.Collections;

public class UserLocalInfo : UserInfoBase {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    public override void PutHand(Card c)
    {
        base.PutHand(c);
        c.transform.parent = m_panelHands.transform;
        c.transform.localPosition = new Vector3(0, 0, 1);
        c.transform.localScale = new Vector3(0.5f, 0.5f, 1);        
    }
}
