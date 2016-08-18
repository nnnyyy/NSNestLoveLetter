DELIMITER $$
CREATE PROCEDURE `SetLoginStatus`(IN nSN INTEGER, IN nStatus INTEGER)
BEGIN
	UPDATE connect_status SET Status = nStatus, LoginDate = now() WHERE SN = nSN;
    IF ROW_COUNT() <= 0 THEN
		INSERT INTO connect_status (SN,Status,LoginDate) VALUES (nSN,nStatus,now());
    END IF;
END$$