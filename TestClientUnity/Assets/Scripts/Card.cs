using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class Card : MonoBehaviour {
    public delegate void touchCardListener(Card c);
    public event touchCardListener touchEvent;

    Image imgBase;    
    public Sprite[] sprites;
    public int m_nNum;

    public enum SizeType
    {
        LOCAL_HAND,
        LOCAL_GROUND,
        REMOTE,
    }

	// Use this for initialization
	void Start () {
        
    }
	
	// Update is called once per frame
	void Update () {
        	
	}

    public void Set(SizeType t, int nIndex)
    {
        if (!imgBase) { imgBase = GetComponent<Image>(); }
        imgBase.sprite = sprites[nIndex];
    }
}


public class CardManager
{
    static public Card Prefeb;
    static public void Init()
    {
        Prefeb = Resources.Load<Card>("Card");
    }

    static public Card CreateCard(Card.SizeType t, int nIndex) {
        Card newCard = GameObject.Instantiate<Card>(Prefeb);
        newCard.Set(t, nIndex);
        return newCard;
    }

}