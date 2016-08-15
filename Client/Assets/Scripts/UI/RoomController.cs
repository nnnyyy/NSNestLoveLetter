using UnityEngine;

namespace NSNest.UI
{
    public class RoomController : MonoBehaviour, IButtonEventReceiver
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
            Debug.Log("RoomController OnPressButton :" + buttonName);
        }
    }
}