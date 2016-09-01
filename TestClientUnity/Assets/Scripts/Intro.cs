using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using UnityEngine.SceneManagement;
using NSNetwork;

public class Intro : MonoBehaviour {

    public UIMsgBox msgBox;
    public InputField m_lbID;
    public InputField m_lbPw;

    public enum Type {
        OK = 0,
        OKCANCEL = 1,        
    }

	// Use this for initialization
	void Start () {
        msgBox.gameObject.SetActive(false);
        NetworkUnityEvent.Instance.Connect(NSNest.Common.Const.SERVER_IP, NSNest.Common.Const.SERVER_PORT);        
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
        Sender.Login(m_lbID.text, m_lbPw.text);
    }

    public void OnBtnRegister(){
        msgBox.Show("지원하지 않는 기능입니다.");
    }

    public void OnLoginRet(GCPLoginRet loginRet)
    {
        Receiver.OnLoginRetCallback -= OnLoginRet;
        if ( loginRet.result == 0)
        {
            SceneManager.LoadScene("Lobby");
        }
        else
        {
            msgBox.Show("아이디 또는 비밀번호 오류");
        }
    }
}
