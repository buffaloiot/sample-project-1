# Main makefile

all: build deploy

build: device controller

device:
	@echo "==> building device artifacts"
	@cd devices && make tarball

controller:
	@echo "==> building controller artifacts"
	@cd controller && make tarball

deploy:
	@docker-compose up --build -d
