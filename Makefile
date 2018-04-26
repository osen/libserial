all:
	c++ -oexample -Isrc src/serial/*.cpp src/example/*.cpp
	c++ -oswitch -Isrc src/serial/*.cpp src/switch/*.cpp

clean:
	rm example
	rm switch
