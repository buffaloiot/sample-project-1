# Main makefile

all: build deploy

build: api device controller

device:
	@echo "==> building device artifacts"
	@cd devices && make tarball

controller:
	@echo "==> building controller artifacts"
	@cd controllers && make tarball

api:
	@echo "==> building api artifacts"
	@cd api && make tarball

deploy:
	@docker-compose up --build -d

.PHONY: api
