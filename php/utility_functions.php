<?php
function startSQLite($filename) {
	$db = new SQLite3($filename, SQLITE3_OPEN_READWRITE);
	return $db;
}

function getAllTimeFramesAllowed() {
	return(array("12 hours", "24 hours", "week", "month"));
}

function convertTimeFrameID2usecs($timeFrame) {
	$usecs = 1; //1 sec
	switch ($timeFrame) {
		case '12 hours':
			$usecs *= 12 * 60 * 60;
			break;
		case '24 hours':
			$usecs *= 24 * 60 * 60;
			break;
		case 'week':
			$usecs *= 7 * 24 * 60 * 60;
			break;
		case 'month':
			$usecs *= 30 * 24 * 60 * 60;
			break;
	}
	return (microtime(true) - $usecs) * 1000 * 1000;
}

function readSensorDataFromSql($db, $sensorName, $timeFrame) {
	$statement = $db->prepare("SELECT sensorDisplayName, measurementTime, sensorValue FROM sensorStats WHERE sensorDisplayName = ? and measurementTime >= ?;");
	$statement->bindValue(1, $sensorName, SQLITE3_TEXT);
	$statement->bindValue(2, convertTimeFrameID2usecs($timeFrame), SQLITE3_FLOAT);
	$result = $statement->execute();

	if (!$result) {
	   die("Cannot execute query. $error");
	}

	$res = array();

	$row = $result->fetchArray(SQLITE3_ASSOC);
    while($row) {
    	$date = new DateTime();
    	$row["measurementTime"] = $date->setTimestamp($row["measurementTime"] / 1000 / 1000);
    	array_push($res, $row);
		$row = $result->fetchArray(SQLITE3_ASSOC);
    }
    $statement->close();
    return($res);
}

function readUniqueSensorNamesFromSql($db) {
	$statement = $db->prepare("SELECT DISTINCT sensorDisplayName FROM sensorStats;");

	$result = $statement->execute();

	if (!$result) {
	   die("Cannot execute query. $error");
	}

	$res = array();

	$row = $result->fetchArray(SQLITE3_ASSOC);
    while($row) {
    	array_push($res, $row["sensorDisplayName"]);
		$row = $result->fetchArray(SQLITE3_ASSOC);
    }
    $statement->close();
    return($res);	
}

function setInputValue($db, $inputName, $stringValue, $doubleValue) {
	$statement = $db->prepare("INSERT INTO inputs (inputName, stringValue, doubleValue) VALUES (?, ?, ?);");
	$statement->bindValue(1, $inputName,   SQLITE3_TEXT);
	$statement->bindValue(2, $stringValue, SQLITE3_TEXT);
	$statement->bindValue(3, $doubleValue, SQLITE3_FLOAT);
	$statement->execute();
	$statement->close();
}

function closeSQLite($db) {
	$db->close();
}

function readActualValues($filename) {
	$f = fopen($filename, "r");
	$res = array();
	
	$line = fgetcsv($f, 2048, "\t");

	while($line != FALSE && $line != NULL) {
		$res[$line[0]] = $line[1];
		$line = fgetcsv($f, 2048, "\t");
	}
	fclose($f);
	return $res;
}

function generateAllLightModes() {
	return array(
		array("name" => "chodba", "mode" => "full on"),
		array("name" => "chodba", "mode" => "full off"),
		array("name" => "chodba", "mode" => "half way"),
		array("name" => "chodba", "mode" => "5 minute delay"),
	);
}

function doTableWithSensorValues($table) {
	$table = array_reverse($table);
	echo("<table>");
	foreach ($table as $value) {
		$time = $value["measurementTime"]->format("H:i:s d. F y");
		echo("<tr>");
		echo("<td>". $time ."</td><td>&nbsp;</td>");
		echo("<td>". number_format($value["sensorValue"], 2, ".", " ") ."</td>");
		echo("</tr>");		
	}
	echo("</table>");

}

function getAllSensorsFromDB() {
	$db = startSQLite("../data/sensor_stats.db");
	$sensorNames = readUniqueSensorNamesFromSql($db);
	closeSQLite($db);
	return $sensorNames;
}

?>