CXX = g++
CXXFLAGS = -g -Wall -std=c++20 -Iinclude

# Source files used in both clients and server
COMMON_SRCS = model.cpp message.cpp wire.cpp except.cpp csapp.cpp io.cpp

# Source files used only in the clients
CLIENT_SRCS = client_util.cpp

# Source files used only in the server
SERVER_SRCS = server.cpp client.cpp passwd_db.cpp message_queue.cpp

# Source files used in unit test programs
TEST_SRCS = message_tests.cpp io_tests.cpp tctest.cpp
TEST_MAIN_SRCS = $(subst tctest.cpp,,$(TEST_SRCS))

# Source files with program main() functions
MAIN_SRCS = server_main.cpp updater_main.cpp display_main.cpp

ALL_SRCS = $(COMMON_SRCS) $(CLIENT_SRCS) $(SERVER_SRCS) $(TEST_SRCS) $(MAIN_SRCS)

TEST_EXES = $(TEST_MAIN_SRCS:%.cpp=build/%)
PROGRAM_EXES = $(MAIN_SRCS:%_main.cpp=build/%)

build/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) -c src/$*.cpp -o build/$*.o

all : $(PROGRAM_EXES) $(TEST_EXES)

build/server : build/server_main.o $(COMMON_SRCS:%.cpp=build/%.o) $(SERVER_SRCS:%.cpp=build/%.o)
	$(CXX) -o $@ $+ -lpthread

build/updater : build/updater_main.o $(COMMON_SRCS:%.cpp=build/%.o) $(CLIENT_SRCS:%.cpp=build/%.o)
	$(CXX) -o $@ $+

build/display : build/display_main.o $(COMMON_SRCS:%.cpp=build/%.o) $(CLIENT_SRCS:%.cpp=build/%.o)
	$(CXX) -o $@ $+

.PHONY: tests
tests : $(TEST_EXES)

build/message_tests : build/message_tests.o build/tctest.o $(COMMON_SRCS:%.cpp=build/%.o)
	$(CXX) -o $@ $+

build/io_tests : build/io_tests.o build/tctest.o $(COMMON_SRCS:%.cpp=build/%.o)
	$(CXX) -o $@ $+

# Create a zipfile to submit to Gradescope
.PHONY: solution.zip
solution.zip :
	rm -f $@
	zip -9r $@ Makefile README.txt src/*.cpp include/*.h

depend :
	$(CXX) $(CXXFLAGS) -M $(ALL_SRCS:%=src/%) \
		| perl -ne 's,^(\S),build/$$1,;print' \
		> build/depend.mak

build/depend.mak :
	touch $@

clean :
	rm -f build/*.o build/depend.mak $(TEST_EXES) $(PROGRAM_EXES) solution.zip

include build/depend.mak
