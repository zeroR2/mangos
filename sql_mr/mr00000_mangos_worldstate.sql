-- WorldStateMgr commands/info
DELETE FROM `command` WHERE `name` IN ('ws','ws add', 'ws list', 'ws global', 'ws reload', 'ws update');

INSERT INTO `command`
    (`name`, `security`, `help`)
VALUES
    ('ws',2,'Syntax: .ws \r\n\r\nList settings of WorldState manager.'),
    ('ws list',2,'Syntax: .ws list\r\n\r\nList all current WorldStates for player.'),
    ('ws global',3,'Syntax: .ws global\r\n\r\nList all current WorldStates in world.'),
    ('ws update',3,'Syntax: .ws update Id Value\r\n\r\nUpdate WorldState [Id] value to [Value].'),
    ('ws add',3,'Syntax: .ws add Id \r\n\r\nAdd WorldState [Id] with default values to world.'),
    ('ws reload',3,'Syntax: .ws reload \r\n\r\nReload all WorldState/templates (same as .reload worldstate).');

-- State manager command line info
DELETE FROM `mangos_string` WHERE `entry` IN (1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1558, 1559);
INSERT INTO `mangos_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES
(1550,' State: %u, Type: %u, Condition: %u, Instance: %u, Value: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1551,' State: %u, %u, %u, %u, %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1552,' State: %u added, (instance %u) value: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1553,' State: %u (instance %u) updated from value %u to %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1554,' State: %u (instance %u) added!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1555,' World States reloaded, count of states %u, templates %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1556,' Error in WorldState operations!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1557,' WorldState settings: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

