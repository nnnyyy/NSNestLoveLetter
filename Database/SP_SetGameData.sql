DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `SetGameData`(IN nSN INTEGER, 
IN nWin INTEGER, IN nLose INTEGER, IN nForceDisconn INTEGER, IN nUseGuard INTEGER, 
IN nSuccessUseGuard INTEGER, IN nAttackedByGuard INTEGER, IN nUseGossip INTEGER, IN nSuccessUseGossip INTEGER, 
IN nAttackedByGossip INTEGER, IN nQueenHeroWizard INTEGER, IN nNoRoundWin INTEGER)
BEGIN
	DECLARE cnt INTEGER;
	SELECT COUNT(*) INTO cnt FROM `game_love_letter`.`statistics` WHERE SN = nSN;
	IF cnt <= 0 THEN
		INSERT INTO statistics (SN,win, lose, force_disconn, use_guard, success_use_guard, attacked_by_guard, use_gossip, success_use_gossip, attacked_by_gossip, queen_hero_wizard, no_round_win) VALUES 
		(nSN,nWin, nLose, nForceDisconn, nUseGuard, nSuccessUseGuard, nAttackedByGuard, nUseGossip, nSuccessUseGossip, nAttackedByGossip, nQueenHeroWizard, nNoRoundWin);
    ELSE
		UPDATE `game_love_letter`.`statistics` SET win = nWin, lose = nLose, force_disconn = nForceDisconn, use_guard = nUseGuard,
								success_use_guard = nSuccessUseGuard, attacked_by_guard = nAttackedByGuard, 
								use_gossip = nUseGossip, success_use_gossip = nSuccessUseGossip, attacked_by_gossip = nAttackedByGossip,
								queen_hero_wizard = nQueenHeroWizard, no_round_win = nNoRoundWin 
							WHERE SN = nSN;		    
	END IF;
END$$
DELIMITER ;