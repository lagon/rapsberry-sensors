<?php
include("../utility_functions.php");

$all_light_modes = generateAllLightModes();

$x = array();
foreach ($all_light_modes as $light) {
	array_push($x, "<li class='singleSensor'><a href='javascript:sendLightInputUpdat(" . $light["name"] . ", " . $light["mode"] ."'>" . $light["name"] . "-" . $light["mode"] . "</a></li>");
}
$all_light_modes = $x;
?>

<ul>
<?php foreach ($all_light_modes as $light) {echo $light;} ?>
</ul>

