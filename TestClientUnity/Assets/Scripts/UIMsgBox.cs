using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System;
using DG.Tweening;

public class UIMsgBox : MonoBehaviour {

    private const string DEFAULT_OK_MSG = "확인";
    public Text m_lbMsg;
    public Text m_lbOKMsg;
    public GameObject m_goWithoutPanel;

    Action okCallback;

    // Use this for initialization
    void Start() {        
    }

    // Update is called once per frame
    void Update() {

    }

    public void Show(string sMsg, string sOkMsg = DEFAULT_OK_MSG, Action callback = null)
    {
        m_lbMsg.text = sMsg;
        m_goWithoutPanel.transform.DOScale(0, 0);
        this.gameObject.SetActive(true);

        m_lbOKMsg.text = sOkMsg;
        okCallback = callback;        
        m_goWithoutPanel.transform.DOScale(1, 0.3f);
    }

    public void Hide()
    {
        this.gameObject.SetActive(false);
    }

    public void OnBtnOK()
    {
        Hide();
        if(okCallback != null)
        {
            okCallback();
        }
    }
}
