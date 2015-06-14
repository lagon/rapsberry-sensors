<?php
#$header = "Content-Type: text";
#header($header);

include("utility_functions.php");
$dbHandle = startSQLite('data/sensor_stats.db');
$stringValue = null;
$doubleValue = null;
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