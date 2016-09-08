using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class Card : MonoBehaviour {
    public delegate void touchCardListener(Card c);
    public event touchCardListener touchEvent;

    Image imgBase;    
    public Sprite[] sprites;
    public int m_nNum;
    public bool bActive { get; set; }

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

    public void Set(int nIndex)
    {
        m_nNum = nIndex;
        if (!imgBase) { imgBase = GetComponent<Image>(); }
        imgBase.sprite = sprites[nIndex];
    }

    static public string GetName(int _cardIdx)
    {
        switch (_cardIdx)
        {
            case 1: return "경비병(1)";
            case 2: return "신하(2)";
            case 3: return "험담가(3)";
            case 4: return "동료(4)";
            case 5: return "영웅(5)";
            case 6: return "마법사(6)";
            case 7: return "왕비(7)";
            case 8: return "공주(8)";
        }

        return "---";
    }

    void Selected()
    {

    }
}


public class CardManager
{
    static public Card Prefeb;
    static public void Init()
    {
        Prefeb = Resources.Load<Card>("Card");
    }

    static public Card CreateCard(int nIndex) {
        Card newCard = GameObject.Instantiate<Card>(Prefeb);
        newCard.Set(nIndex);
        return newCard;
    }

}