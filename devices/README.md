# Devices

A simple IoT Simulated Device that sends sensor readings to a MQTT Topic.
Each device can be configured with any number of sensors using the predefined sensor simulator plugins.

## Dependencies

Install project dependencies with:

```
make depend
```

## Build

To build the project, run:

```
make package
```

## Run

```
node dist/device/index.js --log-level=debug --config-file=src/config/warehouse.json
```

## Sample config

```
{
	"mqtt": {
		"host": "172.17.0.1",
		"port": 1883,
		"connectTimeout": 30000
	},
	"device": {
		"interval": 10000,
		"topicPrefix": {
			"publish": "farm/warehouse/device2",
			"subscribe": "farm/warehouse/device2/cmd"
		},
		"sensors": {
			"temp": {
				"type": "thermostat",
				"enabled": true,
				"temp": 10,
				"target": 0,
				"simulator": {
					"type": "temp",
					"step_interval": 1,
					"decr_min": 2,
					"incr_max": 2
				}
			},
			"door_state": {
				"type": "door",
				"enabled": true,
				"state": 0,
				"simulator": {
					"type": "random",
					"min": 0,
					"max": 1,
					"incr_threshold": 0.85,
					"decr_threshold": 0.5
				}
			}
		}
	}
}
```

## Docker Image

To build the docker image, run:

```
make image
```

The local image will be tagged as: `localhost:5000/buffaloiot/device:latest`

## License

Copyright 2020 Buffalo Iot

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
