<?php

function notifySensorRunner() {
	$notificationFile = "/tmp/ra_sen_notification_pipe";
	$f = fopen($notificationFile, "w");
	echo(fwrite($f, "1", 1));
	echo("<br> Notification written at : " . date("Y/m/d H:i:s.u", time()) . "<br>");
	echo(fclose($f));
}


#$header = "Content-Type: text";
#header($header);

echo "AAAAA";

include("utility_functions.php");
$dbHandle = startSQLite('data/sensor_stats.db');
$stringValue = NULL;
$doubleValue = NULL;
if ($_GET["type"] == "string") {
	$stringValue = $_GET["value"];
} else if ($_GET["type"] == "double") {
	$doubleValue = $_GET["value"];
} else {
	die();
}
setInputValue($dbHandle, $_GET["input_name"], $stringValue, $doubleValue);
closeSQLite($dbHandle);
notifySensorRunner();
?>