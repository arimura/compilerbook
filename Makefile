install:
	docker build -t compilerbook .

attach:
	docker run --rm -it -v ./9cc:/9cc -w /9cc compilerbook