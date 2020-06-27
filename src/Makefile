CC = gcc
CFLAGS = -Wall -Wshadow -pedantic -O2 -g
OBJDIR    =   objdir
OBJ       =   $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(wildcard *.c)))
TARGET    =   chip8

.PHONY: all clean

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ -lSDL2

clean:
	@rm -f $(TARGET) $(wildcard *.o)
	@rm -rf $(OBJDIR)
