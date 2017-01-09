CXX:=g++

gateway_server:gateway_server.o ../connect_manage/client_connect.o ../config/configure.o ../message_solve/account_message_solve.o ../datalayer/sql_operation.o
	$(CXX) $^ -I/usr/include/mysql/ -L/usr/lib64/mysql/ -lmysqlclient -o gateway_server

gateway_server.o:gateway_server.cpp gateway_server.h
	$(CXX) -c $<

logic_server:logic_server.o ../connect_manage/client_connect.o ../config/configure.o ../message_solve/account_message_solve.o ../datalayer/sql_operation.o
	$(CXX) $^ -I/usr/include/mysql/ -L/usr/lib64/mysql/ -lmysqlclient -o logic_server

logic_server.o:logic_server.cpp logic_server.h
	$(CXX) -c $<

clean:
	rm *.o

