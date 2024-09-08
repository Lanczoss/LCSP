OUT:=client
SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:%.c=%.o)
COM_OP:=-Wall -g -pthread
CC:=gcc

$(OUT):$(OBJS)
	$(CC) $^ -o $@
%.o:%.c
	$(CC) -c $< -o $@ $(COM_OP)

.PHONY:clean rebuild
clean:
	$(RM) $(OUT) $(OBJS)
rebuild:clean $(OUT)

