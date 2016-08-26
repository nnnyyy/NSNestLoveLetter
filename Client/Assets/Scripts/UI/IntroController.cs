using UnityEngine;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;


namespace NSNest.UI
{
    public class IntroController : MonoBehaviour
    {
        public GameObject objCreatePopup;

        public UIInput inputID;
        public UIInput inputPW;

        public UIInput inputCAId;
        public UIInput inputCANickname;
        public UIInput inputCAPw;

        void Start()
        {
            objCreatePopup.SetActive(false);
            NetworkUnityEvent.Instance.Connect(Const.SERVER_IP, Const.SERVER_PORT);

            inputID.value = PlayerPrefs.GetString("UserID", "");
            inputPW.value = PlayerPrefs.GetString("UserPW", "");
        }

        void OnDestroy()
        {
            Receiver.OnLoginRetCallback -= OnLoginCallBack;
            Receiver.OnRegisterUserRetCallback -= OnRegisterUserRetCallback;
        }

        public void OnClickOpenCreateAccount()
        {
            objCreatePopup.SetActive(true);
        }

        public void OnClickLogin()
        {
            if (string.IsNullOrEmpty(inputID.value) == true)
            {
                DefaultPopup.OpenPopup("Login", "id is null", DefaultPopup.eType.OK);
                return;
            }

            if (string.IsNullOrEmpty(inputPW.value) == true)
            {
                DefaultPopup.OpenPopup("Login", "pw is null", DefaultPopup.eType.OK);
                return;
            }

            Receiver.OnLoginRetCallback += OnLoginCallBack;
            Receiver.OnRegisterUserRetCallback += OnRegisterUserRetCallback;
            Debug.Log("id >> " + inputID.value + ", pw >> " + inputPW.value);
            Sender.Login(inputID.value, inputPW.value);
        }

        public void OnClickClose()
        {
            Application.Quit();
        }

        public void OnClickCreateAccount()
        {
            string patternID = @"^[a-zA-Z0-9]*$";
            string patternNick = @"^[a-zA-Z0-9가-힣]*$";

            if (string.IsNullOrEmpty(inputCAId.value) == true)
            {
                DefaultPopup.OpenPopup("계정생성", "아이디를 입력하시오!", DefaultPopup.eType.OK);
                return;
            }

            if (string.IsNullOrEmpty(inputCANickname.value) == true)
            {
                DefaultPopup.OpenPopup("계정생성", "닉네임을 입력하시오!", DefaultPopup.eType.OK);
                return;
            }

            if (string.IsNullOrEmpty(inputCAPw.value) == true)
            {
                DefaultPopup.OpenPopup("계정생성", "비밀번호를 입력하시오!", DefaultPopup.eType.OK);
                return;
            }

            if ( System.Text.RegularExpressions.Regex.IsMatch(inputCAId.value, patternID) == false )
            {
                DefaultPopup.OpenPopup("계정생성", "사용불가능한 아이디", DefaultPopup.eType.OK);
                return;
            }

            if( System.Text.RegularExpressions.Regex.IsMatch(inputCANickname.value, patternNick) == false )
            {
                DefaultPopup.OpenPopup("계정생성", "사용불가능한 닉네임", DefaultPopup.eType.OK);
                return;
            }

            Sender.RegisterUser(inputCAId.value, inputCANickname.value, inputCAPw.value);
        }

        public void OnClickCloseCreateAccountPopup()
        {
            inputCAId.value = "";
            inputCANickname.value = "";
            inputCAPw.value = "";
            objCreatePopup.SetActive(false);
        }

        void OnLoginCallBack(GCPLoginRet loginRet)
        {
            switch(loginRet.result)
            {
                case GCPLoginRet.eResult.Success:
                    PlayerPrefs.SetString("UserID", inputID.value);
                    PlayerPrefs.SetString("UserPW", inputPW.value);
                    SceneLoader.Instance.Load("Room");
                    break;

                case GCPLoginRet.eResult.ErrorIdOrPassword:
                    DefaultPopup.OpenPopup("Login", "ErrorIdOrPassword", DefaultPopup.eType.OK);
                    break;

                case GCPLoginRet.eResult.NotJoined:
                    DefaultPopup.OpenPopup("Login", "NotJoined", DefaultPopup.eType.OK);
                    break;

                case GCPLoginRet.eResult.NowConneting:
                    DefaultPopup.OpenPopup("Login", "NowConneting", DefaultPopup.eType.OK);
                    break;

                default:
                    DefaultPopup.OpenPopup("Login", "default", DefaultPopup.eType.OK);
                    break;
            }
        }

        void OnRegisterUserRetCallback(GCPRegisterUserRet registerUserRet)
        {
            switch(registerUserRet.result)
            {
                case GCPRegisterUserRet.eReuslt.Success:
                    DefaultPopup.OpenPopup("계정생성", "성공!!", DefaultPopup.eType.OK, () => {
                        inputID.value = inputCAId.value;
                        inputPW.value = inputCAPw.value;
                        OnClickCloseCreateAccountPopup();
                    } );
                    break;

                case GCPRegisterUserRet.eReuslt.AlreadyRegisted:
                    DefaultPopup.OpenPopup("계정생성", "이미 존재하는 계정임", DefaultPopup.eType.OK);
                    break;

                case GCPRegisterUserRet.eReuslt.Error:
                    DefaultPopup.OpenPopup("계정생성", "알 수 없는 에러!", DefaultPopup.eType.OK);
                    break;
            }
        }
    }
}