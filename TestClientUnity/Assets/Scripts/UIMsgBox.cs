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

    public Button btnOK;
    public Button btnYes;
    public Button btnNo;

    Action okCallback;

    // Use this for initialization
    void Start() {        
    }

    // Update is called once per frame
    void Update() {

    }

    public void Show(string sMsg, string sOkMsg = DEFAULT_OK_MSG, Action callback = null)
    {
        SoundManager.instance.PlaySingle(2);
        btnOK.gameObject.SetActive(true);
        btnYes.gameObject.SetActive(false);
        btnNo.gameObject.SetActive(false);
        m_lbMsg.text = sMsg;
        m_goWithoutPanel.transform.DOScale(0.7f, 0);
        this.gameObject.SetActive(true);

        m_lbOKMsg.text = sOkMsg;
        okCallback = callback;        
        m_goWithoutPanel.transform.DOScale(1, 0.2f);
    }

    public void ShowYesNo(string sMsg, Action _okCallback = null)
    {
        btnOK.gameObject.SetActive(false);
        btnYes.gameObject.SetActive(true);
        btnNo.gameObject.SetActive(true);
        m_lbMsg.text = sMsg;
        m_goWithoutPanel.transform.DOScale(0.7f, 0);
        this.gameObject.SetActive(true);
        
        okCallback = _okCallback;
        m_goWithoutPanel.transform.DOScale(1, 0.2f);
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

    public void OnBtnCancel()
    {
        Hide();        
    }
}
