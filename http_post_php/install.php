<?php

//Connect to database and create table
	$servername = "localhost";
	$username = "id9969311_esp8266";
	$password = "esp8266test";
	$dbname = "id9969311_testdb";
 
	// Create connection
	$conn = new mysqli($servername, $username, $password, $dbname);
	// Check connection
	if ($conn->connect_error) {
	    die("Connection failed: " . $conn->connect_error);
	}
 
	//Sr No, Station, Status(OK, NM, WM, ACK) Date, Time
	//1         A          NM                 12-5-18    12:15:00 am
	// sql to create table
	$sql = "CREATE TABLE logs (
	id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	station VARCHAR(30),
	status VARCHAR(30),
	remark VARCHAR(50),
	`Date` DATE NULL,
	`Time` TIME NULL, 
	`TimeStamp` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
	)";
 
	if ($conn->query($sql) === TRUE) {
	    echo "Table logs created successfully";
	} else {
	    echo "Error creating table: " . $conn->error;
	}
 
	$conn->close();
?>