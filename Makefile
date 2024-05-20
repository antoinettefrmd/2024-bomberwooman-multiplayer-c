# Variables

NAME		= bomberwoman
NAME_C		= client
INCLUDE		= includes
SRC_DIR		= srcs/
OBJ_DIR		= objects/
CC			= cc
CFLAGS		= -Wall -Wextra -Werror 
RM			= rm -f
SMAKE		= make --no-print-directory

# Colors

END			=	\033[0m
BOLD		=	\033[1m
UNDER		=	\033[4m
REV			=	\033[7m
DEF_COLOR	=	\033[0;39m
GRAY		=	\033[0;90m
RED			=	\033[0;91m
LIGHT_RED	=	\033[0;31m
GREEN		=	\033[0;92m
YELLOW		=	\033[0;93m
BLUE		=	\033[0;94m
MAGENTA		=	\033[0;95m
CYAN		=	\033[0;96m
WHITE		=	\033[0;97m
 
SRC_FILES	=	main serveur_partie

CLIENT_FILES =  client ncurses ncurses_utils



SRC			=	$(addprefix $(SRC_DIR), $(addsuffix .c, $(SRC_FILES)))
OBJ			=	$(addprefix $(OBJ_DIR), $(addsuffix .o, $(SRC_FILES)))

CLIENT		=	$(addprefix $(SRC_DIR), $(addsuffix .c, $(CLIENT_FILES)))
OBJ_C		=	$(addprefix $(OBJ_DIR), $(addsuffix .o, $(CLIENT_FILES)))

###

OBJF		=	.cache_exists

all:		$(NAME) $(NAME_C)


$(NAME):	$(OBJ)
			@$(CC) $(OBJ) -pthread -lncurses -o $(NAME)
			@echo "$(GREEN)$(BOLD)$(NAME) compiled!$(DEF_COLOR)"

$(NAME_C):	$(OBJ_C)
			@$(CC) $(OBJ_C) -pthread -lncurses -o $(NAME_C)
			@echo "$(GREEN)$(MAGENTA)$(NAME_C) compiled!$(DEF_COLOR)"

$(OBJ_DIR)%.o: $(SRC_DIR)%.c | $(OBJF)
			@echo "\033[1A                                                     "
			@echo -n "\033[1A"
			@echo "$(YELLOW)Compiling: $< $(DEF_COLOR)"
			@$(CC) $(CFLAGS) -I $(INCLUDE) -I $(LIBFT)/includes -c $< -o $@

$(OBJF):
			@mkdir -p $(OBJ_DIR)

clean:
			@$(RM) -r $(OBJ_DIR)
			@echo "$(BLUE)$(NAME) object files cleaned!$(DEF_COLOR)"

fclean:		clean
			@$(RM) $(NAME)
			@$(RM) $(NAME_C)
			@echo "$(CYAN)$(NAME) executable files cleaned!$(DEF_COLOR)"
			@echo "$(CYAN)$(NAME_C) executable files cleaned!$(DEF_COLOR)"

re:			fclean all

.PHONY:		all clean fclean re bonus

