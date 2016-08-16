using UnityEngine;
using System.Collections.Generic;
using NSNest.Common;

namespace NSNest.UI
{
    public class IntroController : MonoBehaviour
    { 

        public void OnPressLogin()
        {
            Debug.Log("IntroController OnPressLogin");
            SceneLoader.Instance.Load("room");
        }

        public void OnPressJoin()
        {
            Debug.Log("IntroController OnPressJoin");
        }

        public void OnPressRanking()
        {
            Debug.Log("IntroController OnPressRanking");
        }
    }
}