CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
GTEST_DIR=googletest/googletest
SERVERCLASSES=config_parser.cc

default:  config_parser config_parser_test request_handler.o echo_handler.o file_handler.o not_found_handler.o status_handler.o request.o response.o mime_types.o session.o webserver 
#webserver_test session_test echo_handler_test file_handler_test not_found_handler_test

request_handler.o: request_handler.cc request_handler.h mime_types.h mime_types.cc response.cc response.h request.cc request.h
	g++ -c -std=c++11 request_handler.cc -lboost_system

file_handler.o: file_handler.cc file_handler.h request_handler.h mime_types.h mime_types.cc response.cc response.h request.cc request.h
	g++ -c -std=c++11 file_handler.cc -lboost_system

echo_handler.o: echo_handler.cc echo_handler.h request_handler.h mime_types.h mime_types.cc response.cc response.h request.cc request.h
	g++ -c -std=c++11 echo_handler.cc -lboost_system

not_found_handler.o: not_found_handler.cc not_found_handler.h request_handler.h mime_types.h mime_types.cc response.cc response.h request.cc request.h
	g++ -c -std=c++11 not_found_handler.cc -lboost_system
	
status_handler.o: status_handler.cc status_handler.h request_handler.h mime_types.h mime_types.cc response.cc response.h request.cc request.h
	g++ -c -std=c++11 status_handler.cc -lboost_system

request.o: request.cc request.h
	g++ -c -std=c++11 request.cc -lboost_system

response.o: response.cc response.h 
	g++ -c -std=c++11 response.cc -lboost_system

session.o: session.cc session.h request_handler.h echo_handler.h file_handler.h response.cc response.h request.cc request.h
	g++ -c -std=c++11 session.cc  -lboost_system

mime_types.o: mime_types.cc mime_types.h
	g++ -c -std=c++11 mime_types.cc

webserver: webserver.h webserver.cc webserver_main.cc config_parser.h config_parser.cc session.h request_handler.o session.o mime_types.o file_handler.o echo_handler.o not_found_handler.o status_handler.o request.o response.o
	g++ webserver.h webserver.cc webserver_main.cc config_parser.cc request_handler.o session.o mime_types.o file_handler.o echo_handler.o not_found_handler.o status_handler.o request.o response.o -I /usr/local/Cellar/boost/1.54.0/include -std=c++11 -lboost_system -o webserver


config_parser: config_parser.cc config_parser_main.cc
	$(CXX) -o $@ $^ $(CXXFLAGS)

file_handler_test:
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread file_handler_test.cc file_handler.cc mime_types.cc mime_types.h ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o file_handler_test -lboost_system

echo_handler_test:
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread echo_handler_test.cc echo_handler.cc mime_types.cc mime_types.h ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o echo_handler_test -lboost_system

request_handler_test: 
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread request_handler_test.cc request_handler.cc mime_types.cc mime_types.h ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o request_handler_test -lboost_system

request_test: 
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread request_test.cc request.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o request_test -lboost_system

response_test: 
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread response_test.cc response.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o response_test -lboost_system




session_test:
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread session_test.cc request_handler.cc request_handler.h session.cc session.h mime_types.cc file_handler.cc echo_handler.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o session_test -lboost_system

webserver_test:
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread webserver_test.cc webserver.cc config_parser.cc request_handler.cc session.cc mime_types.cc file_handler.cc echo_handler.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o webserver_test -lboost_system

config_parser_test:
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread config_parser_test.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o config_parser_test

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM *.gcda *.gcno *.gcov config_parser config_parser_test *.tar.gz webserver request_handler.o session.o request_handler_test webserver_test session_test echo_handler_test file_handler_test

test: clean default
	./webserver_test
	./session_test
	./echo_handler_test
	./file_handler_test
	./response_test
	./request_test

integration_test: clean default
	python integration_test.py
	
test_coverage:

	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread session_test.cc request_handler.cc request_handler.h session.cc mime_types.cc file_handler.cc echo_handler.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o session_test -lboost_system -fprofile-arcs -ftest-coverage
	./session_test; gcov -r session.cc
	
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread webserver_test.cc webserver.cc config_parser.cc request_handler.cc request_handler.h session.cc mime_types.cc file_handler.cc echo_handler.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o webserver_test -lboost_system -fprofile-arcs -ftest-coverage
	./webserver_test; gcov -r webserver.cc
  
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread echo_handler_test.cc echo_handler.cc mime_types.cc mime_types.h ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o echo_handler_test -lboost_system -fprofile-arcs -ftest-coverage
	./echo_handler_test; gcov -r echo_handler.cc

	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -lboost_system
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread file_handler_test.cc file_handler.cc mime_types.cc mime_types.h ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o file_handler_test -lboost_system -fprofile-arcs -ftest-coverage
	./file_handler_test; gcov -r file_handler.cc

	

	
