CC = gcc
CFLAGS = -Wall -Wextra -pthread #-Werror #-pedantic
ODIR = build

SRC_SHARED = src
SRC_SERVER = src/server
SRC_USER = src/user

TARGET_SERVER = server
TARGET_USER = user

OBJS_SERVER = $(ODIR)/server.o $(ODIR)/request_queue.o $(ODIR)/log.o  $(ODIR)/office.o $(ODIR)/operations.o
OBJS_USER = $(ODIR)/args.o $(ODIR)/user.o $(ODIR)/log.o $(ODIR)/instruction.o


.PHONY: all clean

all: $(TARGET_SERVER) $(TARGET_USER)

$(TARGET_SERVER) : $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(OBJS_SERVER)

$(TARGET_USER) : $(OBJS_USER)
	$(CC) $(CFLAGS) -o $(TARGET_USER) $(OBJS_USER)


ifdef DEV_INFO
DEV_FLAGS = -save-temps
endif


$(ODIR)/%.o: $(SRC_SHARED)/%.c
	$(CC) $(CFLAGS) $(DEV_FLAGS) -MMD -c $< -o $@

$(ODIR)/%.o: $(SRC_SERVER)/%.c
	$(CC) $(CFLAGS) $(DEV_FLAGS) -MMD -c $< -o $@

$(ODIR)/%.o: $(SRC_USER)/%.c
	$(CC) $(CFLAGS) $(DEV_FLAGS) -MMD -c $< -o $@


clean:
	rm -f $(TARGET_SERVER) $(ODIR)/*.o $(ODIR)/*.d $(ODIR)/*.i $(ODIR)/*.s *.txt
	rm -f $(TARGET_USER) $(ODIR)/*.o $(ODIR)/*.d $(ODIR)/*.i $(ODIR)/*.s *.txt

-include $(TARGET:=.d)