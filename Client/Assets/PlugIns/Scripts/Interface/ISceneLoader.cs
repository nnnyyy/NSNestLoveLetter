
namespace NSNest.Common
{
    public interface ISceneLoader
    {
        /// <summary>
        /// 원하는 씬을 불러오고 싶을때 호출
        /// </summary>
        void Load(string sceneName, bool isAdditive = false);

        /// <summary>
        /// 씬 로딩 시작 이벤트
        /// </summary>
        event SceneLoadBegin OnSceneLoadBegin;

        /// <summary>
        /// 씬 로딩 완료 이벤트
        /// </summary>
        event SceneLoaded OnSceneLoaded;
    }
}