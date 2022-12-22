.PHONY: kmod user

kmod:
	@sudo make -C kmod

user:
	@sudo make build -C user

