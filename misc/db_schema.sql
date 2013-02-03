-- Database: etlegacy
-- ------------------------------------------------------

--
-- Table structure for table `server_status`
--

DROP TABLE IF EXISTS `server_status`;
CREATE TABLE `server_status` (
  `ss_uid` mediumint(8) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Unique Row ID',
  `ss_key` varchar(20) NOT NULL COMMENT 'Status Key',
  `ss_val` varchar(40) NOT NULL COMMENT 'Status Value',
  `ss_persist` smallint(6) NOT NULL DEFAULT '0' COMMENT 'Set to 1 to retain in database',
  PRIMARY KEY (`ss_uid`),
  UNIQUE KEY `ss_key` (`ss_key`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COMMENT='Server Status';

--
-- Dumping data for table `server_status`
--

INSERT INTO `server_status` VALUES (1,'DB_Test','OK',1);
INSERT INTO `server_status` VALUES (2,'Current_Map','None',1);

-- End of Schema Dump
