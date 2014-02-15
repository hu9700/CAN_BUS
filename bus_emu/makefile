FILE_LIST:=$(shell ls -C)
SRC_LIST:=$(filter %.c, ${FILE_LIST})
OBJ_LIST:=$(patsubst %.c,%.o,${SRC_LIST})
OUT_FILE:=can_emu.out
CC_FLAG:=-c
CC:=gcc

all:${OUT_FILE}

${OUT_FILE}:${OBJ_LIST}
	${CC} -o $@ $<

${OBJ_LIST}:%.o:%.c
	${CC} ${CC_FLAG} $<

.PHONY:clean cleanall all
clean:
	-rm ${OBJ_LIST}
cleanall:
	-rm ${OBJ_LIST} ${OUT_FILE}

