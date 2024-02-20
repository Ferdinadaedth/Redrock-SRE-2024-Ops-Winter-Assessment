CREATE DATABASE USER;
USE USER;

CREATE TABLE `users` (
  `id` INT AUTO_INCREMENT,
  `username` varchar(50) NOT NULL,
  `password` varchar(50) NOT NULL,
  PRIMARY KEY(id)
)
