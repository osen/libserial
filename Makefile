all:
	c++ -oexample -Isrc src/serial/*.cpp src/example/*.cpp

clean:
	rm example
