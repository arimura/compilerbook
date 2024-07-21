install:
	docker build -t compilerbook .

attach:
	docker run --rm -it -v ./9cc:/9cc -w /9cc compilerbook

demo:
	gcc -o demo demo.c
	./demo

clean:
	rm -f demo
