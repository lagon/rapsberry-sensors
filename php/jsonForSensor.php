<?php
$header = "Content-Type: application/json";
header($header);

include("utility_functions.php");


$dbHandle = startSQLite('data/sensor_stats.db');
$allSensors = readUniqueSensorNamesFromSql($dbHandle);

if (!in_array($_GET["sensor"], $allSensors)) {
	echo("ERROR");
}

if (!in_array($_GET["time_frame"], getAllTimeFramesAllowed())) {
	echo("ERROR");
}

$temp = readSensorDataFromSql($dbHandle, $_GET["sensor"], $_GET["time_frame"]);
closeSQLite($dbHandle);

echo(json_encode($temp));

?>