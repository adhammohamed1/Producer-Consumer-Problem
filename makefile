target:
	g++ -g -c consumer.cpp
	g++ -g -c producer.cpp
	g++ -g -o producer producer.o
	g++ -g -o consumer consumer.o	
