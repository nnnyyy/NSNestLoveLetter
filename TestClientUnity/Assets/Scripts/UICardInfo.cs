using UnityEngine;
using System.Collections;

public class UICardInfo : MonoBehaviour {

    [SerializeField]
    private GameObject[] aInfo;
    private int _cur;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    public void Show(int _cardNum)
    {
        _cur = _cardNum - 1;
        aInfo[_cur].SetActive(true);
    }

    public void Hide()
    {
        aInfo[_cur].SetActive(false);
    }
}
