CC      := gcc
SRCDIR  := src
OUTDIR  := out
OBJDIR  := $(OUTDIR)/obj
TARGET  := $(OUTDIR)/orbit

BREW    := $(shell brew --prefix 2>/dev/null || echo /opt/homebrew)

CFLAGS  := -std=c11 -Wall -Wextra -O2 -Iinclude -I$(BREW)/include -DGL_SILENCE_DEPRECATION
LDFLAGS := -L$(BREW)/lib
LDLIBS  := -lglfw -lm -framework OpenGL

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(OUTDIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OUTDIR) $(OBJDIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OUTDIR)

-include $(DEPS)
