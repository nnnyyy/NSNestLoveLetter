using UnityEngine;
using System.Collections.Generic;
using NSNest.Common;

namespace NSNest.UI
{
    public static class ObjectAccessor
    {
        static Dictionary<string, Camera> mCameras = new Dictionary<string, Camera>();
        static Dictionary<string, Transform> mDocks = new Dictionary<string, Transform>();

        /// <summary>
        /// 카메라를 미리 등록합니다.
        /// </summary>
        public static void RegisterCamera(string key, Camera camera)
        {
            if (mCameras.ContainsKey(key))
            {
                if (mCameras[key] != null)
                {
                    Debug.Log("[ObjectAccessor] 같은 이름의 카메라가 이미 등록되어 있습니다. => " + key, camera);
                    return;
                }
            }

            mCameras[key] = camera;
        }

        public static void RemoveCamera(string key)
        {
            if (mCameras.ContainsKey(key))
                mCameras.Remove(key);
        }

        /// <summary>
        /// 등록된 카메라를 가져옵니다.
        /// </summary>
        public static Camera GetCamera(string key)
        {
            Camera camera;

            if (mCameras.TryGetValue(key, out camera))
            {
                if (camera == null)
                    mCameras.Remove(key);

                return camera;
            }
            return null;
        }

        /// <summary>
        /// 지정한 node 아래 UI 오브젝트를 붙이고 싶으면 dock으로 등록합니다.
        /// </summary>
        public static void RegisterDock(string key, Transform dock)
        {
            if (mDocks.ContainsKey(key))
            {
                if (mDocks[key] != null)
                {
                    Debug.LogWarning("[ObjectAccessor] 같은 이름의 dock이 이미 등록되어 있습니다. => " + key, dock);
                    return;
                }
            }

            mDocks[key] = dock;
        }

        public static void RemoveDock(string key)
        {
            if (mDocks.ContainsKey(key))
                mDocks.Remove(key);
        }

        /// <summary>
        /// dock을 반환합니다.
        /// </summary>
        public static Transform GetDock(string key)
        {
            if (!m_IsEventSet)
            {
                m_IsEventSet = true;
                SceneLoader.Instance.OnSceneLoaded += HandleOnLoaded;
            }

            Transform dock;
            if (mDocks.TryGetValue(key, out dock))
            {
                if (dock == null)
                    mDocks.Remove(key);

                return dock;
            }
            return null;
        }

        static bool m_IsEventSet = false;
        static void HandleOnLoaded(string sceneName)
        {
            mCameras.Clear();
            mDocks.Clear();
        }
    }
}