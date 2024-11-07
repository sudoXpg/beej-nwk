NAME := $(shell uname -n)
MAKEFLAGS += --no-print-directory
all:
	@echo running the server in background
	@gcc streamsocketclient.c -o client
	@gcc streamsocketserver.c -o server
	@./server &
	@sleep 1
	@echo
	@echo
	./client $(NAME)
	@killall ./server
	@$(MAKE) clean

clean:
	@rm server
	@rm client