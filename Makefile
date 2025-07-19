TARGET := clicker

SRC_DIR         := src
LIB_DIR         := lib
LIB_SDL_DIR     := ${LIB_DIR}/sdl
LIB_SDL_TTF_DIR := ${LIB_DIR}/sdl_ttf

BUILD_DIR             := out
BUILD_SRC_DIR         := ${BUILD_DIR}/${SRC_DIR}
BUILD_LIB_DIR         := ${BUILD_DIR}/${LIB_DIR}
BUILD_LIB_SDL_DIR     := ${BUILD_DIR}/${LIB_SDL_DIR}
BUILD_LIB_SDL_TTF_DIR := ${BUILD_DIR}/${LIB_SDL_TTF_DIR}

BUILD_TARGET := ${BUILD_DIR}/${TARGET}

SRCS := $(addprefix ${SRC_DIR}/, \
		main.c                   \
	)

OBJS := $(patsubst ${SRC_DIR}/%.c, ${BUILD_SRC_DIR}/%.o, ${SRCS})

DIRS := $(sort                   \
		${BUILD_DIR}             \
		${BUILD_SRC_DIR}         \
		${BUILD_LIB_DIR}         \
		${BUILD_LIB_SDL_DIR}     \
		${BUILD_LIB_SDL_TTF_DIR} \
	)

INCLUDES := ${LIB_SDL_DIR}/include
INCLUDES += ${LIB_SDL_TTF_DIR}

CC      := gcc
CFLAGS  := -Wall -Wextra
CFLAGS  += -pedantic
CFLAGS  += $(addprefix -I, ${INCLUDES})
LDFLAGS += -L${BUILD_LIB_SDL_DIR} -lSDL2 -lSDL2main
LDFLAGS += -L${BUILD_LIB_SDL_TTF_DIR} -lSDL2_ttf
LDFLAGS += -lm

.PHONY: all src lib
all: ${BUILD_TARGET}

${BUILD_TARGET}: lib src

${DIRS}:
	@mkdir -p ${DIRS}

src: ${OBJS} | ${DIRS}
	$(info [${TARGET}] BUILD ${SRC_DIR})
	@${CC} ${CFLAGS} ${OBJS} -o ${BUILD_TARGET} ${LDFLAGS}

${BUILD_SRC_DIR}/%.o: ${SRC_DIR}/%.c
	@${CC} ${CFLAGS} -c $< -o $@

lib: lib-sdl lib-sdl-ttf | ${DIRS}
	$(info [${TARGET}] BUILD ${LIB_DIR})

.PHONY: lib-sdl
lib-sdl:
	@cmake -S ${LIB_SDL_DIR} -B ${BUILD_LIB_SDL_DIR}
	@make -C ${BUILD_LIB_SDL_DIR} -j${nproc}

.PHONY: lib-sdl-ttf
lib-sdl-ttf:
	@cmake -S ${LIB_SDL_TTF_DIR} -B ${BUILD_LIB_SDL_TTF_DIR}
	@make -C ${BUILD_LIB_SDL_TTF_DIR} -j${nproc}

PHONY: run
run:
	$(info [${TARGET}] RUN)
	@./${BUILD_TARGET}

.PHONY: clean clean-all src-clean lib-clean
clean: src-clean

clean-all: src-clean lib-clean
	$(info [${TARGET}] CLEAN ALL)
	@rm -rf ${BUILD_DIR}

src-clean:
	$(info [${TARGET}] CLEAN ${SRC_DIR})
	@rm -rf ${BUILD_SRC_DIR}

lib-clean:
	$(info [${TARGET}] CLEAN ${LIB_DIR})
	@rm -rf ${BUILD_LIB_DIR}

.PHONY: tags
tags:
	@find -name "*.[chsS]" | ctags -L- --exclude=${BUILD_DIR}
