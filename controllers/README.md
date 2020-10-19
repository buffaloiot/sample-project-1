# Controller

A simple IoT controller application that receives messages from a MQTT Topic and based on the topic format `<location>/<device>/<device ID>/<sensor>` will store the sensor reading in the configured PostgreSQL DB endpoint.

## Flow Diagram

Controller application flow diagram:

![Controller Flow Diagram](docs/controller-flow.png)

## Dependencies

* g++ 8 or greater
* `libmosquitto-dev`
* `libjsoncpp-dev`
* `libpqxx-dev`
* `libpq-dev`

## Build

To build the project, run:

```
make
```

## Run

```
MQTT_HOSTNAME=172.17.0.1 \
MQTT_PORT=1883 \
PG_CONNECTION_STRING=postgresql://sample:password@172.17.0.1:5432/sample \
MQTT_KEEPALIVE_INTERVAL=60 \
MQTT_SUB_TOPIC=# \
HANDLER_CONFIG_FILE=config/all.json \
./bin/controller
```

Note, assumes PostgreSQL and MQTT Broker is running in a Docker container with default ports exposed on docker network 172.17.0.1

## DB Schema

```
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
```

## Handlers

In addition, for a set of configured sensor handlers, a factory pattern was used to instantiate handlers from configuration that perform predefined business logic.

Three predefined handlers were included that send command values to a device subscribing on the handler's publish topic:

* `hysteresis`- If a sensor value exceeds a maximum value, a max output value is
                sent; otherwise, if the sensor value drops below a minimum value,
                a min value is sent.
* `state` - For a configured state map, a sensor value matching a state will send
            a value configured for that state.  A `state_count` map sets the number
            of times that state is seen consecutively before a value is sent for that
            state.  A value is only sent once for that state change.
* `scheduler` - For a configured schedule, a specific value is sent for the
                configured time.

## Sample handler config

```
{
  "engine_overheat": {
    "type": "hysteresis",
    "subTopic": "farm/tractor/device1/temp",
    "pubTopic": "farm/tractor/device1/cmd/speed",
    "hysteresis": {
      "max": {
        "limit": 40,
        "value": 0,
        "repeat": true
      },
      "min": {
        "limit": 30,
        "value": 60,
        "repeat": false
      }
    }
  },
  "wheel_position": {
    "type": "scheduler",
    "pubTopic": "farm/tractor/devices1/cmd/wheel_position",
    "interval": 1,
    "max": 60,
    "schedule": {
      "0": -45,
      "20": 0
    }
  },
  "door_open": {
    "type": "state",
    "subTopic": "farm/warehouse/device2/door_state",
    "pubTopic": "farm/warehouse/device2/cmd/temp",
    "state_count": {
      "0": 5,
      "1": 1
    },
    "state": {
      "0": 0,
      "1": -10
    }
  }
}
```

## Docker Image

To build the docker image, run:

```
make image
```

## License

Copyright 2020 Buffalo Iot

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
