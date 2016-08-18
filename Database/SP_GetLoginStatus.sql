DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `GetLoginStatus`(IN nSN INTEGER, OUT nStatus INTEGER)
BEGIN
	SELECT Status INTO nStatus FROM `game_love_letter`.`connect_status` WHERE SN = nSN;
    IF ROW_COUNT() <= 0 THEN
		INSERT INTO connect_status (SN,Status,LoginDate) VALUES (nSN,0,now());
        SET nStatus = 0;
    END IF;
END$$
DELIMITER ;
