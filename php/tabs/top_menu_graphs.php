<?php
include("../utility_functions.php");

$all_sensors = getAllSensorsFromDB();

$x = array();
foreach ($all_sensors as $sensor) {
	array_push($x, "<li class='singleSensor'><a href='javascript:updateGraphSensor(\"".$sensor."\");'>" . $sensor . "</a></li>");
}
$all_sensors = $x;


$x = getAllTimeFramesAllowed();
$all_time_frames = array();
foreach ($x as $time_frame) {
	array_push($all_time_frames, "<li class='singleSensor'><a href='javascript:updateGraphTimeFrame(\"".$time_frame."\");'>" . $time_frame . "</a></li>");
}
?>

<div class="selectors">
<h3>Sensors</h3>
<ul>
<?php foreach ($all_sensors as $sensor) {echo $sensor;} ?>
</ul>

<h3>Time Frame</h3>
<ul>
<?php foreach ($all_time_frames as $time_frame) {echo $time_frame;} ?>
</ul>
</div>

<div id="sensor_graph_area">
<svg></svg>
</div>

<div id="log">
</div>