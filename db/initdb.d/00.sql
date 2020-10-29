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

CREATE OR REPLACE FUNCTION get_readings(
	_location text,
	_device_type text,
	_device_id text,
	_sensor text,
	_start_time timestamp with time zone,
	_end_time timestamp with time zone,
	_min_value integer,
	_max_value integer
)
RETURNS TABLE (
	location TEXT,
	device_type TEXT,
	device_id TEXT,
	sensor TEXT,
	ts TIMESTAMP WITH TIME ZONE,
	reading INTEGER
)
LANGUAGE plpgsql
AS $$
BEGIN
	RETURN QUERY SELECT
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
		(_max_value IS NULL OR readings.reading <= _max_value);
END;
$$;

\echo Create function get_sensors
CREATE OR REPLACE FUNCTION get_sensors (
	_device_id TEXT
)
RETURNS TEXT[]
AS $$
	DECLARE
		_sensors TEXT[];
	BEGIN
		_sensors = ARRAY(
			SELECT
				DISTINCT(sensor)
			FROM readings
			WHERE
				device_id = _device_id
		);
		RETURN _sensors;
	END;
$$ LANGUAGE 'plpgsql';

\echo Create function device_get_json
CREATE OR REPLACE FUNCTION get_devices(
	_location TEXT,
	_device_type TEXT,
	_device_id TEXT
)
RETURNS TABLE (
	location TEXT,
	device_type TEXT,
	device_id TEXT,
	sensors TEXT[]
)
AS $$
BEGIN
	RETURN QUERY SELECT
		results.location,
		results.device_type,
		results.device_id,
		get_sensors(results.device_id) AS sensors
	FROM (
		SELECT
			DISTINCT
				readings.location AS location,
				readings.device_type AS device_type,
				readings.device_id AS device_id
		FROM readings
		WHERE (
			(_location IS NULL OR readings.location = _location) AND
			(_device_type IS NULL OR readings.device_type = _device_type) AND
			(_device_id IS NULL OR readings.device_id = _device_id)
		)
	) AS results;
END;
$$ LANGUAGE 'plpgsql';
