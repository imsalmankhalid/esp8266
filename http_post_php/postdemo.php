<?php
//Creates new record as per request
    //Connect to database
	$servername = "localhost";
	$username = "id9969311_esp8266";
	$password = "esp8266test";
	$dbname = "id9969311_testdb";
 
    // Create connection
    $conn = new mysqli($servername, $username, $password, $dbname);
    // Check connection
    if ($conn->connect_error) {
        die("Database Connection failed: " . $conn->connect_error);
    }
 
    //Get current date and time
    date_default_timezone_set('Asia/Kolkata');
    $d = date("Y-m-d");
    //echo " Date:".$d."<BR>";
    $t = date("H:i:s");
 
    if(!empty($_POST['status']) && !empty($_POST['station']))
    {
    	$status = $_POST['status'];
    	$station = $_POST['station'];
 
	    $sql = "INSERT INTO logs (station, status, Date, Time)
		
		VALUES ('".$station."', '".$status."', '".$d."', '".$t."')";
 
		if ($conn->query($sql) === TRUE) {
		    echo "OK";
		} else {
		    echo "Error: " . $sql . "<br>" . $conn->error;
		}
	}
 
 
	$conn->close();
?>