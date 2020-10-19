CREATE USER sample;
ALTER USER sample WITH PASSWORD 'password';
CREATE DATABASE sample WITH OWNER sample;
GRANT ALL PRIVILEGES ON DATABASE sample TO sample;
GRANT CONNECT ON DATABASE sample to sample;

\connect sample sample

CREATE TABLE readings (
	location text,
	device_type text,
	device_id text,
	sensor text,
	ts timestamp with time zone,
	reading integer
);

CREATE OR REPLACE FUNCTION readings_get_json(
	_location text,
	_device_type text,
	_device_id text,
	_sensor text,
	_start_time timestamp with time zone,
	_end_time timestamp with time zone,
	_min_value integer,
	_max_value integer
)
RETURNS JSON
LANGUAGE plpgsql
AS
$$
DECLARE
	_result json;
BEGIN
	SELECT INTO _result to_json(array_agg(r))
	FROM (SELECT
		readings.location,
		readings.device_type,
		readings.device_id,
		readings.sensor,
		readings.ts,
		readings.reading
	FROM
		readings
	WHERE
		(_location IS NULL OR readings.location = _location) AND
		(_device_type IS NULL OR readings.device_type = _device_type) AND
		(_device_id IS NULL OR readings.device_id = _device_id) AND
		(_sensor IS NULL OR readings.sensor = _sensor) AND
		(_start_time IS NULL OR readings.ts >= _start_time) AND
		(_end_time IS NULL OR readings.ts <= _end_time) AND
		(_min_value IS NULL OR readings.reading >= _min_value) AND
		(_max_value IS NULL OR readings.reading <= _max_value)) AS r;
	RETURN _result;
END;
$$
