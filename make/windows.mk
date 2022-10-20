CC = gcc


INCLUDE = -I$(INCLUDE_DIR)
DBFLAGS = -O2 -fstrict-aliasing

WFLAGS 		= 	-Wall -Wmissing-include-dirs -Wswitch-default -Wswitch-enum  	\
				-Wunused-parameter -Wextra -Werror -Wfatal-errors  				\
				-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations \
				-Wshadow -Wlarger-than-1048576 -Wconversion						\
				-Wfloat-equal -Wlong-long -Wredundant-decls -Wbad-function-cast	\
				-Wcast-qual -Wcast-align -Wwrite-strings -Wpointer-arith  		\
				-Winline -Wdisabled-optimization 								\
				-Wold-style-definition -Wmissing-noreturn -Wuninitialized		\
				-Winit-self -Winvalid-pch -Wpacked -Wpadded -Wunreachable-code 	

WNOFLAGS	=	-Wno-conversion -Wno-unused-result

CXXFLAGS 		= $(INCLUDE) $(DBFLAGS) $(WFLAGS) $(WNOFLAGS)
DEBUG_FLAGS 	= -DDEBUG

LINKFLAGS 	= -lws2_32 -lregex


all: init target


init:
	- mkdir $(BUILD_DIR) $(OUTPUT_DIR) $(INCLUDE_DIR) 

clean:
	- del $(OBJS) $(OUTPUT_DIR)\$(OUTPUT)


target: $(OBJS)
	$(CC) $^ $(LINKFLAGS) -o $(OUTPUT_DIR)\$(OUTPUT)


%.o: $(SRC_DIR)\%.c
	$(CC) "$<" $(CXXFLAGS) $(DEBUG_FLAGS) -c -o $@
