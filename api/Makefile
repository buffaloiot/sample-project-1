# Device Makefile

package:
	@echo "==> Building package"
	@npm run compile
	@echo "Done."

depend:
	@echo "==> Installing dependencies"
	@npm install --no-package-lock
	@echo "Done."

tarball: package
	@echo "==> Building npm package tarball"
	@mv $$(npm pack 2>&1 | tail -1) docker/api/api.tgz

image: package tarball
	@echo "==> Build image"
	@cd ./docker/api && docker build . -t localhost:5000/buffaloiot/api:latest

.PHONY: package depend image
