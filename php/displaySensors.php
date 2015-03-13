<!DOCTYPE html>
<html>
<head>
	<title>Sensors</title>
	<script src="http://d3js.org/d3.v3.js"></script>
</head>
<body>

<?php


include("utility_functions.php");

$latest = readActualValues("data/latestValues.csv");

echo("<table>");
echo("<tr>");
foreach (array_keys($latest) as $i) {
	echo("<td>". $i . "</td>");
}

echo("</tr>");
echo("<tr>");
foreach (array_keys($latest) as $i) {
	echo("<td>". number_format($latest[$i], 2, ".", " ") . "</td>");
}
echo("</tr>");
echo("</table>");

$dbHandle = startSQLite('data/sensor_stats.db');

$temp = readSensorDataFromSql($dbHandle, "H21DF-Temperature");
echo("<h1>Inside Temperature</h1>");
doTableWithSensorValues($temp);

$humidity = readSensorDataFromSql($dbHandle, "H21DF-Humidity");
echo("<h1>Inside Humidity</h1>");
doTableWithSensorValues($humidity);

$pressure = readSensorDataFromSql($dbHandle, "BMP183-Pressure");
echo("<h1>Athmospheric Pressure</h1>");
doTableWithSensorValues($pressure);



?>

</body>
</html>
