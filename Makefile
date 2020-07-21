SRC_DIR			= srcs
SRC_FILES		= main.cpp MimeTypes.cpp Utility.cpp PortListener.cpp Resource.cpp \
					Request.cpp Response.cpp Server.cpp Client.cpp ClientSet.cpp RFC.cpp \
					BaseConfiguration.cpp Configuration.cpp ServerConfiguration.cpp CGI.cpp \
					BodyList.cpp RawBody.cpp FileBody.cpp StreamBody.cpp CGIBody.cpp \
					CGIResponse.cpp StreamJob.cpp File.cpp Logger.cpp

SRCS			= $(addprefix $(SRC_DIR)/,$(SRC_FILES))
OBJ_DIR			= objs
OBJS			= $(addprefix $(OBJ_DIR)/,$(SRC_FILES:.cpp=.o))

CXX				= clang++
RM				= rm -f
CXXFLAGS		= -g3 -std=c++11 -Wall -Wextra -Werror -Iincludes -Iopenssl/include -Izlib
LDFLAGS		 	= openssl/libssl.a openssl/libcrypto.a zlib/libz.a -lpthread -ldl

NAME			= server

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp
				$(CXX) $(CXXFLAGS) -c $< -o $@


all:			$(NAME)

$(NAME):		openssl/libssl.a zlib/libz.a $(OBJS)
				$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
				$(RM) $(OBJS)

fclean:			clean
				$(RM) $(NAME)

re:				fclean $(NAME)

test:			$(ALL)
				./server

openssl/libssl.a:
				cd openssl; ./config; $(MAKE) -j4;

zlib/libz.a:
				cd zlib; ./configure; $(MAKE) -j4;

.PHONY:			all clean fclean re test openssl zlib
