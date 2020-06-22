CC = g++
CFLAGS = -Wall -Wextra -Wshadow -pedantic -O2
OBJDIR    =   objdir
OBJ       =   $(addprefix $(OBJDIR)/, $(patsubst %.cc, %.o, $(wildcard *.cc)))
TARGET    =   chip8

.PHONY: all clean

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ -lSDL2

clean:
	@rm -f $(TARGET) $(wildcard *.o)
	@rm -rf $(OBJDIR)
