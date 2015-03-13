<?php
function startSQLite($filename) {
	$db = new SQLite3($filename, SQLITE3_OPEN_READONLY);
	return $db;
}

function readSensorDataFromSql($db, $sensorName) {
	$statement = $db->prepare("SELECT sensorDisplayName, measurementTime, sensorValue FROM sensorStats WHERE sensorDisplayName = ?;");
	$statement->bindValue(1, $sensorName, SQLITE3_TEXT);

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

    return($res);
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
?>