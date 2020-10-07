# Main makefile

all: build deploy

build: device

device:
	@echo "==> building device artifacts"
	@cd devices && make tarball

deploy:
	@docker-compose up --build -d
