CXX			= c++

NAME		= webserv

RM			= rm -rf

SRCS_DIR	= src

TEST_DIR	= config_tests

OBJS_DIR		= obj
TEST_OBJS_DIR	= test_objs

CXXFLAGS	= -Wall -Wextra -Werror -MP -MD -std=c++98 -g -Iinc -I$(GTEST_DIR)/include


TESTFLAGS	= -Wall -Wextra -Werror -MP -MD -g -Iinc -I$(GTEST_DIR)/include

SRCS   	= \
		$(SRCS_DIR)/Node.cpp \
		$(SRCS_DIR)/ClientInfo.cpp \
		$(SRCS_DIR)/LocationInfo.cpp \
		$(SRCS_DIR)/Server.cpp \
		$(SRCS_DIR)/Log.cpp \
		$(SRCS_DIR)/Request.cpp \
		$(SRCS_DIR)/Director.cpp \
        $(SRCS_DIR)/Config.cpp \
        $(SRCS_DIR)/ConfigParser.cpp \
        $(SRCS_DIR)/ConfigSetters.cpp \
        $(SRCS_DIR)/ConfigDispatcher.cpp \
        $(SRCS_DIR)/Utils.cpp \
		$(SRCS_DIR)/main_config.cpp
TESTS	= \
		

OBJS		= $(SRCS:${SRCS_DIR}/%.cpp=${OBJS_DIR}/%.o)
TEST_OBJS	= $(TESTS:${TEST_DIR}/%.cpp=${TEST_OBJS_DIR}/%.o)

DEPS	= $(OBJS:%.o=%.d)

$(NAME): $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) 

$(TEST_OBJS_DIR)/%.o: $(TEST_DIR)/%.cpp
	mkdir -p $(TEST_OBJS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@


all		: $(NAME)

run		: 
	./webserv config_files/simple.conf

debug	:
	gdb --arg ./webserv config_files/simple.conf

fclean	: clean
		$(RM) $(NAME)
		$(RM) $(TEST_OBJS_DIR)
		$(RM) $(OBJS_DIR)
		$(RM) webserv.d

clean:
	$(RM) $(OBJS)

re		: fclean all

.PHONY	: all clean fclean re run