--sqlite3 ./sensor_stats.db < sensor_stats.sql

create table sensorStats (
	sensorID varchar not null,
	sensorDisplayName varchar,
	measurementTime bigint not null,
	sensorValue double,
	primary key (sensorID, measurementTime)
);
