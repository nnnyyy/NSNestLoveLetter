-- 유저 관리 테이블
CREATE TABLE `game_love_letter`.`account` (
  `SN` INT NOT NULL AUTO_INCREMENT,
  `ID` VARCHAR(45) NOT NULL,
  `PW` VARCHAR(200) NOT NULL,
  `Nick` VARCHAR(45) NOT NULL,
  `RegDate` DATETIME NULL,
  `LastLogoutDate` DATETIME NULL,
  PRIMARY KEY (`SN`));
  
  
-- 중복 로그인을 막기 위한 테이블
CREATE TABLE `game_love_letter`.`connect_status` (
`SN` INT NOT NULL,  
`Status` INT NOT NULL DEFAULT '0',
`LoginDate` DATETIME NULL,  
PRIMARY KEY (`SN`));


-- 게임 통계
CREATE TABLE `game_love_letter`.`statistics` (
`SN` INT NOT NULL,  
`win` INT NOT NULL,
`lose` INT NOT NULL,
`force_disconn` INT NOT NULL,
`use_guard` INT NOT NULL,
`success_use_guard` INT NOT NULL,
`attacked_by_guard` INT NOT NULL,
`use_gossip` INT NOT NULL,
`success_use_gossip` INT NOT NULL,
`attacked_by_gossip` INT NOT NULL,
`queen_hero_wizard` INT NOT NULL,
`no_round_win` INT NOT NULL,
PRIMARY KEY (`SN`));