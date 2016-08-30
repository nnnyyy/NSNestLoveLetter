using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class Card : MonoBehaviour {
    Image imgBase;    
    public Sprite[] sprites;

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
        float f = 0.5f;
        switch (t)
        {
            case SizeType.LOCAL_HAND:
                f = 0.5f;
                break;

            case SizeType.LOCAL_GROUND:
                f = 0.25f;
                break;

            case SizeType.REMOTE:
                f = 0.25f;
                break;
        }
        transform.localScale = new Vector3(f, f, f);
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