using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using UnityEngine.SceneManagement;
using NSNetwork;
using System;

public class Intro : MonoBehaviour {

    public UIMsgBox msgBox;
    public InputField m_lbID;
    public InputField m_lbPw;
    public Button btnLogin;

    public enum Type {
        OK = 0,
        OKCANCEL = 1,        
    }

	// Use this for initialization
	void Start () {
        SceneManager.UnloadScene("Lobby");
        SceneManager.UnloadScene("Game");
        NetworkUnityEvent.Instance.curMsgBox = msgBox;
        msgBox.gameObject.SetActive(false);
        NetworkUnityEvent.Instance.Connect(NSNest.Common.Const.SERVER_IP, NSNest.Common.Const.SERVER_PORT);
        SoundManager.Instance.PlayBGM("bgm1");
        string sID = PlayerPrefs.GetString(GlobalData.Instance.P_ID);
        string sPW = PlayerPrefs.GetString(GlobalData.Instance.P_PW);
        if(sID != "")
        {
            m_lbID.text = sID;
            m_lbPw.text = sPW;            
        }
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    public void OnBtnLogin() {
        if(!NetworkUnityEvent.Instance.IsConnected())
        {
            msgBox.Show("네트워크 연결을 확인 하십시오");
            return;
        }
        if (m_lbID.text == "" || m_lbPw.text == "")
        {
            msgBox.Show("아이디 또는 비밀번호를 입력 해 주세요");
            return;
        }

        Receiver.OnLoginRetCallback += OnLoginRet;
        try
        {
            Sender.Login(m_lbID.text, m_lbPw.text);
        }        
        catch(Exception e)
        {
            msgBox.Show(e.Message);
        }
        btnLogin.interactable = false;        
    }

    public void OnBtnRegister(){
        msgBox.Show("지원하지 않는 기능입니다.");
    }

    public void OnLoginRet(GCPLoginRet loginRet)
    {
        Receiver.OnLoginRetCallback -= OnLoginRet;
        if ( loginRet.result == 0)
        {
            PlayerPrefs.SetString(GlobalData.Instance.P_ID, m_lbID.text);
            PlayerPrefs.SetString(GlobalData.Instance.P_PW, m_lbPw.text);                        
            ScreenFade.Fade(0, 1, 1.0f, 0, true, () =>
            {
                SceneManager.LoadScene("Lobby");
            });
        }
        else
        {
            PlayerPrefs.SetString(GlobalData.Instance.P_ID, "");
            PlayerPrefs.SetString(GlobalData.Instance.P_PW, "");
            btnLogin.interactable = true;
            msgBox.Show("아이디 또는 비밀번호 오류");
        }
    }
}
