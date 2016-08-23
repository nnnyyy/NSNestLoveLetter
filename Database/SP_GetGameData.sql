DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `GetGameData`(IN nSN INTEGER, 
OUT nWin INTEGER, OUT nLose INTEGER, OUT nForceDisconn INTEGER, OUT nUseGuard INTEGER, 
OUT nSuccessUseGuard INTEGER, OUT nAttackedByGuard INTEGER, OUT nUseGossip INTEGER, OUT nSuccessUseGossip INTEGER, 
OUT nAttackedByGossip INTEGER, OUT nQueenHeroWizard INTEGER, OUT nNoRoundWin INTEGER)
BEGIN
	SELECT win, lose, force_disconn, use_guard, success_use_guard, attacked_by_guard, use_gossip, success_use_gossip, attacked_by_gossip, queen_hero_wizard, no_round_win
	INTO nWin, nLose, nForceDisconn, nUseGuard, nSuccessUseGuard, nAttackedByGuard, nUseGossip, nSuccessUseGossip, nAttackedByGossip, nQueenHeroWizard, nNoRoundWin 
	FROM `game_love_letter`.`statistics` WHERE SN = nSN;
    IF ROW_COUNT() <= 0 THEN
		INSERT INTO statistics (SN,win, lose, force_disconn, use_guard, success_use_guard, attacked_by_guard, use_gossip, success_use_gossip, attacked_by_gossip, queen_hero_wizard, no_round_win) VALUES 
		(nSN,0,0,0,0,0,0,0,0,0,0,0);
    END IF;
END$$
DELIMITER ;
