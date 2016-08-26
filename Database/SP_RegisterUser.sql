DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `RegisterUser`(IN sID VARCHAR(45), IN sPW VARCHAR(45), IN sNick VARCHAR(45), OUT nRet INTEGER)
BEGIN
	DECLARE cnt INTEGER;
	SELECT COUNT(*) INTO cnt FROM `game_love_letter`.`account` WHERE id = sID;
	IF cnt <= 0 THEN
		INSERT INTO ACCOUNT (id,pw,nick,regdate,lastlogoutdate) values (sID,password(sPW),sNick,now(),now());
		IF ROW_COUNT() <= 0 THEN 
			SET nRet = -2;
		ELSE
			SET nRet = 0;
		END IF;
    ELSE
		SET nRet = -1;
	END IF;
END$$
DELIMITER ;