chat_server: server/*.cpp common/*.cpp
	g++ -Icommon -o chat_server $^

chat_client: client/*.cpp common/*.cpp
	g++ -Icommon -o chat_client $^

clean:
	rm -f chat_server chat_client
