CC     = gcc
CFLAGS = -O3 -Wall -Wextra -fPIC -std=c11
INC    = -I/opt/cadence/SPECTRE201/354/tools/mmsim/include
NAME   = uwicsv
SRCS   = $(NAME).c
OBJS   = $(SRCS:.c=.o)
TARGET = lib$(NAME).so

.PHONY: clean
    
all: $(TARGET)
	@echo Successfully compiled $(NAME) Shared Library

$(TARGET): $(OBJS) 
	$(CC) -shared -o $(TARGET) $(OBJS) 

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@

clean:
	$(RM) *.o *.so *.log $(NAME).raw/*
