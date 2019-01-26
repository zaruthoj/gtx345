CC=g++
GTEST_DIR=gtest/googletest
GMOCK_DIR=gtest/googlemock
CFLAGS=-std=c++11 -g -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) -isystem $(GMOCK_DIR)/include -isystem libraries/CircularBuffer -I$(GMOCK_DIR) -pthread

gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc 
	$(CC) $(CFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

libgtest.a: gtest-all.o
	ar -rv libgtest.a gtest-all.o

libgmock.a: $(GMOCK_DIR)/src/gmock-all.cc gtest-all.o
	$(CC) $(CFLAGS) -c $(GMOCK_DIR)/src/gmock-all.cc
	ar -rv libgmock.a gtest-all.o gmock-all.o

listener_test.o: tests/listener_test.cpp
	$(CC) $(CFLAGS) -c tests/listener_test.cpp

listener.o: listener.cpp listener.h
	$(CC) $(CFLAGS) -c listener.cpp

listener_test: listener_test.o listener.o libgtest.a libgmock.a
	$(CC) $(CFLAGS) listener_test.o listener.o libgtest.a libgmock.a -o listener_test

edit_test.o: tests/edit_test.cpp tests/u8g2_mock.h
	$(CC) $(CFLAGS) -c tests/edit_test.cpp

catalog.o: catalog.cpp catalog.h tests/u8g2_mock.h
	$(CC) $(CFLAGS) -c catalog.cpp

edit_test: edit_test.o catalog.o listener.o libgtest.a libgmock.a
	$(CC) $(CFLAGS) edit_test.o listener.o catalog.o libgtest.a libgmock.a -o edit_test

clean:
	rm *.o *.a edit_test

.PHONY: clean
