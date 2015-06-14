var shown_sensor_graph = "";
var shown_time_frame = "";

function readDataAndDisplaySensorGraph(sensor_id, time_frame_id) {
	if ((sensor_id === "") || (time_frame_id === "")) {
		document.getElementById("sensor_graph_area").innerHTML = "<div class='Info'>Please select sensor and time period</div>";
		return;
	}

	httpreq = new XMLHttpRequest();
	httpreq.open("GET", "jsonForSensor.php?sensor=" + sensor_id + "&time_frame=" + time_frame_id, true);
	httpreq.onreadystatechange=function() {
		if (httpreq.readyState==4 && httpreq.status==200) {
			document.getElementById("log").innerHTML=httpreq.responseText;
			displayGraphFromJSON(httpreq.responseText);
		} else {
			document.getElementById("log").innerHTML=httpreq.statusText;
//			document.getElementById("sensor_graph_area").innerHTML=httpreq.statusText;
		}
	}
	httpreq.send();
}

function displayGraphFromJSON(json_string) {
	console.log(json_string);
  var graph_data = JSON.parse(json_string);

  var parseDate = d3.time.format("%Y-%m-%d %H:%M:%S.000000").parse;

  var margin = {top: 20, right: 20, bottom: 50, left: 100};
  var width = 900;
  var height = 650;

  var x = d3.time.scale()
      .range([0, width]);

  var y = d3.scale.linear()
      .range([height, 0]);

  var xAxis = d3.svg.axis()
      .scale(x)
      .orient("bottom");

  var yAxis = d3.svg.axis()
      .scale(y)
      .orient("left");

  var line = d3.svg.line()
      .x(function(d) { return x(d.measurementTime); })
      .y(function(d) { return y(d.sensorValue); });

  d3.select("#sensor_graph_area svg").remove();
  var svg = d3.select("#sensor_graph_area").append("svg")
      .attr("width", width + margin.left + margin.right)
      .attr("height", height + margin.top + margin.bottom)
      .style("margin", "100px")
    .append("g")
      .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
  
  graph_data.forEach(function(d) {
  	console.log(d.measurementTime.date);
    d.measurementTime = parseDate(d.measurementTime.date);
  });

  x.domain(d3.extent(graph_data, function(d) { return d.measurementTime; }));
  y.domain(d3.extent(graph_data, function(d) { return d.sensorValue; }));
  console.log(x);
  console.log(y);

  svg.append("g")
      .attr("class", "x axis")
      .attr("transform", "translate(0," + height + ")")
      .call(xAxis)
      .append("Text")
      .text("Value");

  svg.append("g")
      .attr("class", "y axis")
      .call(yAxis)
    .append("text")
      .attr("transform", "rotate(-90)")
      .attr("y", 6)
      .attr("dy", ".71em")
      .style("text-anchor", "end")
      .text("Value");

  svg.append("path")
      .datum(graph_data)
      .attr("class", "line")
      .attr("d", line);
}

function updateGraphTimeFrame(time_frame_id) {
	shown_time_frame = time_frame_id;
	readDataAndDisplaySensorGraph(shown_sensor_graph, shown_time_frame);
}

function updateGraphSensor(sensor_id) {
	shown_sensor_graph = sensor_id;	
	readDataAndDisplaySensorGraph(shown_sensor_graph, shown_time_frame);
}

function sendInputValue(input_name_id, input_value, input_type) {
  httpreq = new XMLHttpRequest();
  httpreq.open("GET", "setInputValue.php?input_name=" + input_name_id + "&value=" + input_value + "&type=" + input_type, true);
  httpreq.onreadystatechange=function() {
    // document.getElementById("log").innerHTML=httpreq.responseText;
    // alert(httpreq.responseText);
  }
  httpreq.send();
}

function sendLightInputUpdate(light_name, mode_name) {
  sendInputValue(light_name + "input", mode_name, "string");
}

