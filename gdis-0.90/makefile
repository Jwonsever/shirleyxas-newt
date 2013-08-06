# --- auto generated GDIS makefile

USE_GUI = NO
USE_GRISU = NO
include makefile.linux
include makefile.src

CFLAGS := $(CFLAGS) 
INCS := $(INCS) `pkg-config --cflags gtk+-2.0 gthread-2.0 gmodule-2.0` 
LIBS := $(LIBS) `pkg-config --libs gtk+-2.0 gthread-2.0 gmodule-2.0` 

OBJ = $(SRC:.c=.o)

gdis: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o gdis $(LIBS)
.c.o:
	$(CC) $(CFLAGS) -c $< $(INCS)
