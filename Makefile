
NAME	:= kilo

INC_DIR	:= inc
SRC_DIR	:= src
OBJ_DIR	:= obj

COMPILE	:= gcc
C_FLAGS	:= -Wall -Werror -Wextra
# C_FLAGS	+= -pedantic -std=c99 #might remove c99 (allows declaration anywhere within a function, whereas ANSI C wants them at top of function or block)

HEADERS	:= -I$(INC_DIR)

REMOVE	:= rm -rf
NPD		:= --no-print-directory
VOID	:= /dev/null
OS		:= $(shell uname)
USER	:= $(shell whoami)
TIME	:= $(shell date "+%H:%M:%S")

# **************************************************************************** #
# -------------------------------- ALL * FILES ------------------------------- #
# **************************************************************************** #
INCS	:=	$(wildcard $(INC_DIR)/*.h)
SRCS	:=	$(wildcard $(SRC_DIR)/*.c)
OBJS	:=	$(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
# **************************************************************************** #
# ---------------------------------- RULES ----------------------------------- #
# **************************************************************************** #
all: $(NAME)

$(NAME): $(OBJS) $(INCS)
	@$(COMPILE) $(C_FLAGS) $(HEADERS) $(OBJS) -o $@

# @echo "$$TITLE"
# @echo "Compiled for $(ITALIC)$(BOLD)$(PURPLE)$(USER)$(RESET) \
		$(CYAN)$(TIME)$(RESET)\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCS) | $(OBJ_DIR)
	@echo "$(CYAN)Compiling...$(ORANGE)\t$(notdir $<)$(RESET)"
	@$(COMPILE) $(C_FLAGS) $(HEADERS) -c $< -o $@
	@printf "$(UP)$(ERASE_LINE)"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@if [ -d "$(OBJ_DIR)" ]; then \
		$(REMOVE) $(OBJ_DIR); \
		echo "[$(BOLD)$(PURPLE)$(NAME)$(RESET)] \
		$(GREEN)Object files removed$(RESET)"; \
	else \
		echo "[$(BOLD)$(PURPLE)$(NAME)$(RESET)] \
		$(YELLOW)No object files to remove$(RESET)"; \
	fi

fclean: clean
	@if [ -n "$(wildcard $(NAME))" ]; then \
		$(REMOVE) $(NAME); \
		echo "[$(BOLD)$(PURPLE)$(NAME)$(RESET)] \
		$(GREEN)Executable removed$(RESET)"; \
	else \
		echo "[$(BOLD)$(PURPLE)$(NAME)$(RESET)] \
		$(YELLOW)No executable to remove$(RESET)"; \
	fi

re: fclean all

.PHONY: all clean fclean re
# **************************************************************************** #
# ---------------------------------- UTILS ----------------------------------- #
# **************************************************************************** #
run: all
	@./$(NAME)

DEBUG_FLAGS	:= -g
debug: $(C_FLAGS) += $(DEBUG_FLAGS)
debug: re

FORCE_FLAGS	:= \
-Wno-unused-variable \
-Wno-unused-function
force: C_FLAGS += $(FORCE_FLAGS)
force: re
	@echo "adding flags $(YELLOW)$(FORCE_FLAGS)$(RESET)"
	@echo "$(RED)Forced compilation$(RESET)"
	./$(NAME)

.PHONY: run debug force
# **************************************************************************** #
# ---------------------------------- BACKUP (zip) ---------------------------- #
# **************************************************************************** #
USER		:=$(shell whoami)
ROOT_DIR	:=$(notdir $(shell pwd))
TIMESTAMP	:=$(shell date "+%Y%m%d_%H%M%S")
BACKUP_NAME	:=$(ROOT_DIR)_$(USER)_$(TIMESTAMP).zip
MOVE_TO		:= ~/Desktop/$(BACKUP_NAME)

zip: fclean
	@if which zip > $(VOID); then \
		zip -r --quiet $(BACKUP_NAME) ./*; \
		mv $(BACKUP_NAME) $(MOVE_TO); \
		echo "[$(BOLD)$(PURPLE)$(NAME)$(RESET)] \
		compressed as: $(CYAN)$(UNDERLINE)$(MOVE_TO)$(RESET)"; \
	else \
		echo "Please install zip to use the backup feature"; \
	fi

.PHONY: zip
# **************************************************************************** #
# ----------------------------------- DECO ----------------------------------- #
# **************************************************************************** #
define TITLE
[$(BOLD)$(PURPLE)$@$(RESET)]\t$(GREEN)ready$(RESET)
$(PURPLE)
██╗  ██╗██╗██╗      ██████╗ 
██║ ██╔╝██║██║     ██╔═══██╗
█████╔╝ ██║██║     ██║   ██║
██╔═██╗ ██║██║     ██║   ██║
██║  ██╗██║███████╗╚██████╔╝
╚═╝  ╚═╝╚═╝╚══════╝ ╚═════╝ 
$(RESET)

type 'make run' to execute

endef
export TITLE

title:
	@echo "$$TITLE"
	@echo "Compiled for $(ITALIC)$(BOLD)$(PURPLE)$(USER)$(RESET) \
		$(CYAN)$(TIME)$(RESET)\n"

.PHONY: title
# **************************************************************************** #
# ----------------------------------- ANSI ----------------------------------- #
# **************************************************************************** #
ESC			:= \033

RESET		:= $(ESC)[0m
BOLD		:= $(ESC)[1m
DIM			:= $(ESC)[2m
ITALIC		:= $(ESC)[3m
UNDERLINE	:= $(ESC)[4m
BLINK		:= $(ESC)[5m
INVERT		:= $(ESC)[7m
HIDDEN		:= $(ESC)[8m
STRIKE		:= $(ESC)[9m

# Cursor movement
UP			:= $(ESC)[A
DOWN		:= $(ESC)[B
FORWARD		:= $(ESC)[C
BACK		:= $(ESC)[D
NEXT_LINE	:= $(ESC)[E
PREV_LINE	:= $(ESC)[F
COLUMN		:= $(ESC)[G
TOP_LEFT	:= $(ESC)[1;1H

# Erasing
ERASE_REST	:= $(ESC)[K
ERASE_LINE	:= $(ESC)[2K
ERASE_ALL	:= $(ESC)[2J
# **************************************************************************** #
# ---------------------------------- COLORS ---------------------------------- #
# **************************************************************************** #
# Text
BLACK		:= $(ESC)[30m
RED			:= $(ESC)[91m
GREEN		:= $(ESC)[32m
YELLOW		:= $(ESC)[93m
ORANGE		:= $(ESC)[38;5;208m
BLUE		:= $(ESC)[94m
PURPLE		:= $(ESC)[95m
CYAN		:= $(ESC)[96m
WHITE		:= $(ESC)[37m
GRAY		:= $(ESC)[90m

# Background
BG_BLACK	:= $(ESC)[40m
BG_RED		:= $(ESC)[101m
BG_GREEN	:= $(ESC)[102m
BG_YELLOW	:= $(ESC)[103m
BG_ORANGE	:= $(ESC)[48;5;208m
BG_BLUE		:= $(ESC)[104m
BG_PURPLE	:= $(ESC)[105m
BG_CYAN		:= $(ESC)[106m
BG_WHITE	:= $(ESC)[47m
BG_GRAY		:= $(ESC)[100m
