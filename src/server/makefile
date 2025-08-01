service:Test.cpp base64.cpp
	g++ -o service Test.cpp base64.cpp -std=c++17 -lpthread -ljsoncpp -levent -lstdc++fs -lbundle

.PHONY:clean
clean:
	rm -rf service ./low_storage ./deep_storage storage.data