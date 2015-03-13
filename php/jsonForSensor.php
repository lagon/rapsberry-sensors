<?php
$header = "Content-Type: application/json";
header($header);

include("utility_functions.php");

$sensorName = "";
switch ($_GET["sensorName"]) {
	case "H21DF-Temperature":
		$sensorName = "H21DF-Temperature";
		break;
	case "H21DF-Humidity":
		$sensorName = "H21DF-Humidity";
		break;
	case "BMP183-Pressure":
		$sensorName = "BMP183-Pressure";
		break;
	default:
		die("Invalid Sensor Name Passed");
}

$dbHandle = startSQLite('data/sensor_stats.db');
$temp = readSensorDataFromSql($dbHandle, $sensorName);
closeSQLite($dbHandle);

echo(json_encode($temp));

?>