FILE_LIST:=$(shell ls -C)
SRC_LIST:=$(filter %.c, ${FILE_LIST})
OBJ_LIST:=$(patsubst %.c,%.o,${SRC_LIST})
OBJ_FOLDER:=object
CC_FLAG:=-c
CC:=gcc

all:${OBJ_FOLDER}

${OBJ_FOLDER}:${OBJ_LIST}
	(test ! -d ${OBJ_FOLDER} && mkdir ${OBJ_FOLDER});rm -f ${OBJ_FOLDER}/$<;mv $< ${OBJ_FOLDER}/

${OBJ_LIST}:%.o:%.c
	${CC} ${CC_FLAG} $<

.PHONY:clean cleanall all
clean:
	-rm -f ${OBJ_LIST};rm -r -f ${OBJ_FOLDER}/
cleanall:
	-rm -f ${OBJ_LIST};rm -r -f ${OBJ_FOLDER}/

