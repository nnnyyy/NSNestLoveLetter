using UnityEngine;

namespace NSNest.UI
{
    public class DockRegister : MonoBehaviour
    {
        [SerializeField]
        string m_Key;

        void Awake()
        {
            if (!string.IsNullOrEmpty(m_Key))
                ObjectAccessor.RegisterDock(m_Key, transform);
        }

        void OnDestroy()
        {
            if (!string.IsNullOrEmpty(m_Key))
                ObjectAccessor.RemoveDock(m_Key);
        }
    }
}