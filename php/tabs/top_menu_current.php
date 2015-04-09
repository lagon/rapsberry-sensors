<?php

include("../utility_functions.php");

$latest = readActualValues("../data/latestValues.csv");

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

?>
