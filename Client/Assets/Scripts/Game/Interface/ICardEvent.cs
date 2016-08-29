using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    public delegate void PressCard(ICardInfo cardInfo);
    public delegate void MoveCard(ICardInfo cardInfo, int userNumber, CardPosition cardPosition);
    public delegate void ChangeCardStatus(ICardInfo cardInfo, CardStatus cardStatus);

    /// <summary>
    /// 카드 상태, 움직임 변화에 따른 이벤트
    /// </summary>
    public interface ICardEvent
    {
        /// <summary>
        /// 카드가 터치되었을때.
        /// </summary>
        event PressCard OnPressCard;

        /// <summary>
        /// 카드가 위치를 이동할때
        /// </summary>
        event MoveCard OnMoveCard;

        /// <summary>
        /// 카드 앞면, 뒷면 변경시
        /// </summary>
        event ChangeCardStatus OnChangeCardStatus;
    }
}
