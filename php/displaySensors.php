<!DOCTYPE html>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<title>Sensors</title>
	<link rel="stylesheet" href="style.css">
	<script src="d3.v3.js"> </script>
	<script src="functions.js"> </script>
	<script type="text/javascript">
		function show_tab(tabName) {
			httpreq = new XMLHttpRequest();
			httpreq.open("GET", "tabs/" + tabName + ".php", true);
			httpreq.onreadystatechange=function() {
				if (httpreq.readyState==4 && httpreq.status==200) {
    				document.getElementById("main_content").innerHTML=httpreq.responseText;
    			} else {
    				document.getElementById("main_content").innerHTML=httpreq.statusText;
//    				alert(httpreq.statusText);
    			}
			}
			document.getElementById("main_content").innerHtml = "Loading...";
			httpreq.send();
			document.getElementById("top_menu_current").className = "inactive_tab";
			document.getElementById("top_menu_graphs").className = "inactive_tab";
			document.getElementById("top_menu_lights").className = "inactive_tab";
			document.getElementById(tabName).className = "active_tab";

		}
	</script>
</head>
<body>

<ul id="top_menu">
	<li id="top_menu_current"><a href="javascript:show_tab('top_menu_current')">Current Values</a></li>
	<li id="top_menu_graphs"><a href="javascript:show_tab('top_menu_graphs')">History Graphs</a></li>
	<li id="top_menu_lights"><a href="javascript:show_tab('top_menu_lights')">Light Control</a></li>
</ul>

<div id="main_content">
</div>

<script type="text/javascript">show_tab('top_menu_current')</script>

</body>
</html>
