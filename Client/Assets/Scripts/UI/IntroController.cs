using UnityEngine;
using System.Collections.Generic;

namespace NSNest.UI
{
    public interface IButtonEventReceiver
    {
        void OnPressButton(string buttonName);
    }

    public class IntroController : MonoBehaviour, IButtonEventReceiver
    { 
        void Start()
        {
            this.InitButton();
        }

        void OnDestroy()
        {
            this.ReleaseButton();
        }

        public void OnPressButton(string buttonName)
        {
            Debug.Log("IntroController OnPressButton :" + buttonName);
        }
    }
}