# sample-project-1 API

REST API to provide access to sensor readings from devices.

## API

The API is defined using OpenAPI 3.0. See src/definition/swagger.yaml.
For more info on OpenAPI  see (https://swagger.io/).

## Environment variables

| Name                | Default Value                                      | Description                                     |
| ------------------- | -------------------------------------------------- | ----------------------------------------------- |
| `CORS`              | `*`                                                | Cors accepted values                            |
| `LOG_LEVEL`         | `info`                                             | Level at or above which messages will be logged |
| `PG_CONNECT_STRING` | `postgres://sample:password@localhost:5432/sample` | Postgres URL                                    |
| `PORT`              |  8001                                              | Port API server listens on                      |

## TypeScript + Node 

The source for this application is written in typescript. It must be compiled to javascript to create the npm package.
It was based on the example at (https://github.com/rhappdev/nodejs-template/tree/openapi3).
It uses [express-openapi-validator](https://www.npmjs.com/package/express-openapi-validator) for input and output validation, and [swagger-ui-express](https://www.npmjs.com/package/swagger-ui-express) to serve the openAPI doc defining the API.

Special attributes in the swagger file:
- `x-swagger-router-controller` names the file in the routes directory
- `operationId` names the handler function to be invoked

## Directory Structure

| Name                     | Description                                                      |
| ------------------------ | ---------------------------------------------------------------- |
| **dist**                 | Distributable files for the npm package                          |
| **docker**               | Files to build the Docker image                                  |
| **node_modules**         | Npm dependencies                                                 |
| **src**                  | Source code that will be compiled to the dist dir                |
| **src/controllers**      | Controllers define functions to serve express routes             |
| **src/lib**              | Common libraries                                                 |
| **src/middlewares**      | Express middleware                                               |
| **src/routes**           | Express routes                                                   |
| Makefile                 | Contains targets to build the package and docker image.          |
| package.json             | Contains npm metadata; dependencies, build scripts, etc.         |
| tsconfig.json            | Config settings for compiling source code written in TypeScript  |
| tslint.json              | Config settings for TSLint code style checking                   |

## Building the package

Makefile has the targets necessary to build the npm package and Docker image.

- `make package` builds the npm package
- `make depend`  installs npm dependencies
- `make tarball` packs a tarball from the npm package
- `make image`   creates docker image

package.json contains the following scripts:

| Npm Script               | Description                                                      |
| ------------------------ | ------------------------------------------------------------ |
| `clean`                   | Removes dist                                                |
| `compile`                 | Compiles to dist                                            |
| `lint`                    | Runs TSLint on typescript files                             |
| `start`                   | Runs node on dist/index.js. Can be invoked with `npm start` |

