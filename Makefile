install:
	docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile

attach:
	docker run --rm -it -v ./9cc:/9cc compilerbook