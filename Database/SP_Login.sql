DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `Login`(IN sID VARCHAR(45), IN sPW VARCHAR(45),
							OUT nRet INTEGER, OUT nSN INTEGER, OUT sNick VARCHAR(45))
BEGIN
	DECLARE nCount INTEGER DEFAULT 0;    	
    SELECT SN INTO nSN FROM `game_love_letter`.`account` WHERE ID=sID;    
    IF ROW_COUNT() <= 0 THEN
		-- ������ ACCOUNT�� ���� ����.
        SET nRet = -2;
    ELSE
		CALL GetLoginStatus(nSN, @nStatus);
        IF @nStatus = 1 THEN 
			SET nRet = -3;
        ELSE        
			SELECT COUNT(*) INTO nCount FROM `game_love_letter`.`account` WHERE ID=sID AND PW=PASSWORD(sPW);
			IF nCount > 0 THEN
			-- Success Login
				SELECT SN, Nick INTO nSN, sNick FROM `game_love_letter`.`account` WHERE ID=sID;
				SET nRet = 0;
				CALL SetLoginStatus(nSN,1);
			ELSE 
				SET nRet = -1;
				SET nSN = -1;
			END IF;
		END IF;
	END IF;
END$$
DELIMITER ;
