# Device Makefile

package:
	@echo "==> Building package"
	@npm run compile
	@echo "Done."

depend:
	@echo "==> Installing dependencies"
	@npm install --no-package-lock
	@echo "Done."

tarball:
	@echo "==> Building npm package tarball"
	@mv $$(npm pack 2>&1 | tail -1) docker/device/device.tgz

image: package tarball
	@echo "==> Build image"
	@cd ./docker/device && docker build . -t localhost:5000/buffaloiot/device:latest

.PHONY: package depend image
