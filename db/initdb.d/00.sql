CREATE USER sample;
ALTER USER sample WITH PASSWORD 'password';
CREATE DATABASE sample WITH OWNER sample;
GRANT ALL PRIVILEGES ON DATABASE sample TO sample;
GRANT CONNECT ON DATABASE sample to sample;

\connect sample sample

CREATE TABLE readings (
	device	text,
	sensor	text,
	ts	timestamp with time zone,
	reading	integer
);
