OUT:=main
SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:%.c=%.o)
COM_OP:=-Wall -g -lpthread -lmysqlclient -lcrypt -ll8w8jwt
CC:=gcc

$(OUT):$(OBJS)
	$(CC) $^ -o $@	$(COM_OP)
%.o:%.c
	$(CC) -c $< -o $@ $(COM_OP)

.PHONY:clean rebuild
clean:
	$(RM) $(OUT) $(OBJS)
rebuild:clean $(OUT)

