using UnityEngine;
using NSNest.Common;

namespace NSNest.UI
{
    public class RoomController : MonoBehaviour
    {
        public void OnPressBack(string buttonName)
        {
            SceneLoader.Instance.Load("intro");
        }
    }
}