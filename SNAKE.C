#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>


/* Snake segment structure with direction tracking */
struct SnakeSegment {
    int x, y;
    char segment_direction;  /* Direction this segment is facing */
    struct SnakeSegment *next;
} *snake_head;

/* Barrier position structure */
struct BarrierPosition {
    int x, y;
} barriers[9];  /* Store up to 9 barrier positions */

/* Global game variables */
int barrier_count = 0;
int food_x, food_y;
int score = 0;
char hundreds_digit, tens_digit, units_digit;
char current_direction;

/* CGA Color constants */
#define BLACK        0
#define BLUE         1
#define GREEN        2
#define CYAN         3
#define RED          4
#define MAGENTA      5
#define BROWN        6
#define LIGHTGRAY    7
#define DARKGRAY     8
#define LIGHTBLUE    9
#define LIGHTGREEN   10
#define LIGHTCYAN    11
#define LIGHTRED     12
#define LIGHTMAGENTA 13
#define YELLOW       14
#define WHITE        15

/* Macro to create color attribute with foreground and background */
#define MAKE_COLOR(fg, bg) ((bg << 4) | fg)

/* Macro to create blinking color attribute */
#define MAKE_BLINK_COLOR(fg, bg) (0x80 | (bg << 4) | fg)

/* Game colors */
#define SNAKE_COLOR MAKE_COLOR(YELLOW, RED)
#define FOOD_BLINK_COLOR MAKE_BLINK_COLOR(WHITE, GREEN)
#define BARRIER_BLINK_COLOR MAKE_BLINK_COLOR(LIGHTCYAN, BLUE)
#define GAMEOVER_BLINK_COLOR MAKE_BLINK_COLOR(LIGHTRED, RED)

/* Video memory pointer */
/* CGA mode in segment B800H */
unsigned char far *video_memory = (unsigned char far *)0xB8000000L;
int current_color = LIGHTGRAY;

/* Function prototypes - required for C89/C90 */
char get_snake_body_character(char direction);
void set_text_color(int color);
void put_character_at_position(int x, int y, char c, int color);
void draw_game_barrier();
void draw_barrier_at_specific_position(int start_x, int start_y);
void clear_existing_barrier();
void move_barrier_to_new_position();
int is_position_on_barrier(int x, int y);
void draw_food_at_position(int x, int y);
void clear_food_at_position(int x, int y);
int is_position_on_food(int x, int y);
void initialize_snake();
void display_game_over();
void start_new_game();
void handle_keyboard_input();
void move_snake();
void check_snake_collision();
int check_position_availability(int x, int y);
void create_game_layout();
void redraw_entire_snake();

/* Utility function to split integer into individual digit characters */
void split_integer_to_digit_characters(int number, char *hundreds, char *tens, char *units) {
    if (number < 0) {
        number = -number; /* Handle negative numbers */
    }

    /* Get each digit */
    *hundreds = (number / 100) % 10 + '0';
    *tens = (number / 10) % 10 + '0';
    *units = number % 10 + '0';
}

/* Function to get the appropriate body character based on direction */
char get_snake_body_character(char direction) {
    switch (direction) {
        case 'n':  /* Up */
        case 's':  /* Down */
            return '=';
        case 'e':  /* Right */
        case 'w':  /* Left */
        default:
            return '=';
    }
}

void clear_existing_barrier() {
    int i;
    
    /* Clear all current barrier positions */
    for (i = 0; i < barrier_count; i++) {
        put_character_at_position(barriers[i].x, barriers[i].y, ' ', MAKE_COLOR(BLACK, BLACK));
    }
}

void move_barrier_to_new_position() {
    int start_x, start_y;
    int attempts = 0;
    int valid_position = 0;
    int zigzag_pattern_x[] = {4, 3, 2, 1, 0, 1, 2, 3, 4}; /* Relative X positions for zigzag */
    int i;
    int barrier_x, barrier_y;
    struct SnakeSegment *temp_segment;
    
    /* Clear old barrier positions */
    clear_existing_barrier();
    
    /* Try to find a valid new position for the barrier */
    while (!valid_position && attempts < 50) {
        /* Generate random starting position with 5-char offset from borders */
        /* Play area is from (2,2) to (79,24), so with offset: (7,7) to (74,19) */
        /* But we need room for the 9-row pattern, so Y range: (7,15) */
        start_x = rand() % (74 - 7 - 4) + 7;  /* -4 to account for zigzag width */
        start_y = rand() % (15 - 7) + 7;
        
        /* Check if this position would be valid (no conflict with snake or food) */
        valid_position = 1;
        
        for (i = 0; i < 9 && valid_position; i++) {
            barrier_x = start_x + zigzag_pattern_x[i];
            barrier_y = start_y + i;
            
            /* Check if position conflicts with snake */
            temp_segment = snake_head;
            if (temp_segment->x == barrier_x && temp_segment->y == barrier_y) {
                valid_position = 0;
                break;
            }
            temp_segment = temp_segment->next;
            while (temp_segment != snake_head && valid_position) {
                if (temp_segment->x == barrier_x && temp_segment->y == barrier_y) {
                    valid_position = 0;
                    break;
                }
                temp_segment = temp_segment->next;
            }
            
            /* Check if position conflicts with food */
            if (valid_position && is_position_on_food(barrier_x, barrier_y)) {
                valid_position = 0;
            }
        }
        
        attempts++;
    }
    
    /* If we found a valid position, draw the new barrier */
    if (valid_position) {
        draw_barrier_at_specific_position(start_x, start_y);
    } else {
        /* If no valid position found, redraw at original position */
        draw_game_barrier();
    }
}

void draw_barrier_at_specific_position(int start_x, int start_y) {
    int i;
    int zigzag_pattern_x[] = {4, 3, 2, 1, 0, 1, 2, 3, 4}; /* Relative X positions for zigzag */
    int barrier_x, barrier_y;
    
    /* Reset barrier count */
    barrier_count = 0;
    
    /* Draw the zigzag pattern and store positions */
    for (i = 0; i < 9; i++) {
        barrier_x = start_x + zigzag_pattern_x[i];
        barrier_y = start_y + i;
        
        /* Store barrier position */
        barriers[barrier_count].x = barrier_x;
        barriers[barrier_count].y = barrier_y;
        barrier_count++;
        
        put_character_at_position(barrier_x, barrier_y, '&', BARRIER_BLINK_COLOR);
    }
}

void draw_game_barrier() {
    int start_x, start_y;
    
    /* Generate random starting position with 5-char offset from borders */
    /* Play area is from (2,2) to (79,24), so with offset: (7,7) to (74,19) */
    /* But we need room for the 9-row pattern, so Y range: (7,15) */
    start_x = rand() % (74 - 7 - 4) + 7;  /* -4 to account for zigzag width */
    start_y = rand() % (15 - 7) + 7;
    
    draw_barrier_at_specific_position(start_x, start_y);
}

int is_position_on_barrier(int x, int y) {
    int i;
    
    for (i = 0; i < barrier_count; i++) {
        if (barriers[i].x == x && barriers[i].y == y) {
            return 1;
        }
    }
    return 0;
}

void draw_food_at_position(int x, int y) {
    int i, j;
    
    /* Draw 4x2 pattern (4 rows, 2 columns) with blinking effect */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 2; j++) {
            put_character_at_position(x + j, y + i, '*', FOOD_BLINK_COLOR);
        }
    }
}

void clear_food_at_position(int x, int y) {
    int i, j;
    
    /* Clear 4x2 pattern by drawing spaces */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 2; j++) {
            put_character_at_position(x + j, y + i, ' ', MAKE_COLOR(BLACK, BLACK));
        }
    }
}

int is_position_on_food(int x, int y) {
    int i, j;
    
    /* Check if position is within the 4x2 food pattern */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 2; j++) {
            if (x == food_x + j && y == food_y + i) {
                return 1;
            }
        }
    }
    return 0;
}

void redraw_entire_snake() {
    struct SnakeSegment *temp_segment = snake_head;
    
    /* Draw head first */
    put_character_at_position(temp_segment->x, temp_segment->y, '0', SNAKE_COLOR);
    temp_segment = temp_segment->next;
    
    /* Draw body segments with appropriate characters */
    while (temp_segment != snake_head) {
        put_character_at_position(temp_segment->x, temp_segment->y, 
                                  get_snake_body_character(temp_segment->segment_direction), SNAKE_COLOR);
        temp_segment = temp_segment->next;
    }
}

void main() {
    srand((unsigned)time(NULL));  /* Initialize random seed - cast for C89 */
    start_new_game();
}

void set_text_color(int color) {
    current_color = color;
}

void put_character_at_position(int x, int y, char character, int color) {
    int memory_offset;
    
    /* Calculate video memory offset (80 columns * 2 bytes per char) */
    memory_offset = ((y - 1) * 80 + (x - 1)) * 2;
    
    /* Write character and attribute to video memory */
    video_memory[memory_offset] = character;         /* Character */
    video_memory[memory_offset + 1] = color;        /* Color attribute */
}

void start_new_game() {
    current_direction = 'e';
    clrscr();
    fflush(stdin);
    set_text_color(RED);
    create_game_layout();
    draw_game_barrier();
    
    /* Generate food position that doesn't conflict with barriers or snake */
    do {
        food_x = rand() % 76 + 2;  /* Random x from 2 to 77 (need 2 chars width) */
        food_y = rand() % 21 + 2;  /* Random y from 2 to 22 (need 4 chars height) */
    } while (check_position_availability(food_x, food_y) != 1);
    
    draw_food_at_position(food_x, food_y);
    initialize_snake();
    getch();
    
    while (1) {
        hundreds_digit = (score / 100) % 10 + '0';
        tens_digit = (score / 10) % 10 + '0';
        units_digit = score % 10 + '0';
        
        put_character_at_position(54, 1, hundreds_digit, MAKE_COLOR(WHITE, BLACK));
        put_character_at_position(55, 1, tens_digit, MAKE_COLOR(WHITE, BLACK));
        put_character_at_position(56, 1, units_digit, MAKE_COLOR(WHITE, BLACK));
        
        delay(120);
        handle_keyboard_input();
        check_snake_collision();
    }
}

void initialize_snake() {
    int i;
    struct SnakeSegment *current_segment, *new_segment;
    
    set_text_color(YELLOW);
    current_segment = NULL;
    snake_head = (struct SnakeSegment *)malloc(sizeof(struct SnakeSegment));
    snake_head->x = 2;
    snake_head->y = 2;
    snake_head->segment_direction = current_direction;  /* Initialize with current direction */
    current_segment = snake_head;
    
    gotoxy(2, 26);
    printf("Arrow keys - MOVE | R - Restart | Q - Quit | SCORE: ");

    gotoxy(snake_head->x, snake_head->y);
    put_character_at_position(snake_head->x, snake_head->y, '0', SNAKE_COLOR);
    
    for (i = 1; i < 10; i++) {
        new_segment = (struct SnakeSegment *)malloc(sizeof(struct SnakeSegment));
        new_segment->x = current_segment->x + 1;
        new_segment->y = current_segment->y;
        new_segment->segment_direction = 'e';  /* Initial direction is east (right) */
        put_character_at_position(new_segment->x, new_segment->y, 
                                  get_snake_body_character(new_segment->segment_direction), SNAKE_COLOR);
        current_segment->next = new_segment;
        current_segment = new_segment;
    }
    current_segment->next = snake_head;
    snake_head = current_segment;
}

void handle_keyboard_input() {
    char key_pressed;
    char new_direction = current_direction;  /* Keep track of current direction */
    
    /* Process all pending keys, keeping only the last direction change */
    while (kbhit()) {
        key_pressed = getch();
        if (key_pressed == 0) { /* Extended key (arrow keys) */
            key_pressed = getch();  /* Get the actual scan code */
            switch (key_pressed) {
                case 75: /* Left arrow */
                    if (new_direction != 'e') new_direction = 'w';
                    break;
                case 80: /* Down arrow */
                    if (new_direction != 'n') new_direction = 's';
                    break;
                case 77: /* Right arrow */
                    if (new_direction != 'w') new_direction = 'e';
                    break;
                case 72: /* Up arrow */
                    if (new_direction != 's') new_direction = 'n';
                    break;
            }
        } else if (key_pressed == 'r' || key_pressed == 'R') {
            start_new_game();
            return;
        } else if (key_pressed == 'q' || key_pressed == 'Q') {
            exit(1);
        }
    }
    
    /* Apply the final direction */
    current_direction = new_direction;
    move_snake();
}

void move_snake() {
    int new_head_x = snake_head->x, new_head_y = snake_head->y;
    struct SnakeSegment *temp_segment;
    struct SnakeSegment *current_segment;
    char old_head_direction = snake_head->segment_direction;  /* Save current head direction */
    char previous_direction;
    char temp_direction;
    
    switch (current_direction) {
        case 'w':
            new_head_x--;
            break;
        case 's':
            new_head_y++;
            break;
        case 'e':
            new_head_x++;
            break;
        case 'n':
            new_head_y--;
            break;
        default:
            exit(1);
    }
    
    /* Check for collisions BEFORE moving */
    if (new_head_x <= 1 || new_head_x >= 80 || new_head_y <= 1 || new_head_y >= 25) {
        display_game_over();
        return;
    }
    
    if (is_position_on_barrier(new_head_x, new_head_y)) {
        display_game_over();
        return;
    }
    
    /* Check self collision */
    temp_segment = snake_head->next;
    while (temp_segment != snake_head) {
        if (temp_segment->x == new_head_x && temp_segment->y == new_head_y) {
            display_game_over();
            return;
        }
        temp_segment = temp_segment->next;
    }
    
    if (is_position_on_food(new_head_x, new_head_y)) {
        /* Food eaten - snake grows */
        clear_food_at_position(food_x, food_y);
        temp_segment = (struct SnakeSegment *)malloc(sizeof(struct SnakeSegment));
        temp_segment->next = snake_head->next;
        temp_segment->x = new_head_x;
        temp_segment->y = new_head_y;
        temp_segment->segment_direction = current_direction;  /* New head gets current direction */
        snake_head->next = temp_segment;
        snake_head->segment_direction = old_head_direction;  /* Old head becomes body with its direction */
        snake_head = temp_segment;
        
        /* MOVE BARRIER AFTER EATING FOOD */
        move_barrier_to_new_position();
        
        /* Generate new food */
        do {
            food_x = rand() % 76 + 2;  /* Random x from 2 to 77 (need 2 chars width) */
            food_y = rand() % 21 + 2;  /* Random y from 2 to 22 (need 4 chars height) */
        } while (check_position_availability(food_x, food_y) != 1);
        draw_food_at_position(food_x, food_y);
        
        /* Redraw entire snake to ensure visibility over food */
        redraw_entire_snake();
        
        score++;
        gotoxy(9, 27);
        printf("%d   ", score);  /* Update score below game area */
        gotoxy(new_head_x, new_head_y);
    } else {
        /* Normal movement - propagate directions through snake body */
        current_segment = snake_head;
        previous_direction = current_direction;  /* Direction for new head */
        
        /* Propagate directions from head to tail */
        do {
            temp_direction = current_segment->segment_direction;
            current_segment->segment_direction = previous_direction;
            previous_direction = temp_direction;
            current_segment = current_segment->next;
        } while (current_segment != snake_head);
        
        /* Move head, clear tail */
        snake_head = snake_head->next;
        put_character_at_position(snake_head->x, snake_head->y, ' ', MAKE_COLOR(BLACK, BLACK));
        snake_head->x = new_head_x;
        snake_head->y = new_head_y;
        snake_head->segment_direction = current_direction;  /* Head gets new direction */
        put_character_at_position(new_head_x, new_head_y, '0', SNAKE_COLOR);
        
        /* Redraw body segments that might have changed direction */
        temp_segment = snake_head->next;
        while (temp_segment != snake_head) {
            put_character_at_position(temp_segment->x, temp_segment->y, 
                                      get_snake_body_character(temp_segment->segment_direction), SNAKE_COLOR);
            temp_segment = temp_segment->next;
        }
        
        /* If snake overlaps with food area, redraw any overlapping segments */
        temp_segment = snake_head;
        do {
            if (is_position_on_food(temp_segment->x, temp_segment->y)) {
                if (temp_segment == snake_head) {
                    put_character_at_position(temp_segment->x, temp_segment->y, '0', SNAKE_COLOR);
                } else {
                    put_character_at_position(temp_segment->x, temp_segment->y, 
                                              get_snake_body_character(temp_segment->segment_direction), SNAKE_COLOR);
                }
            }
            temp_segment = temp_segment->next;
        } while (temp_segment != snake_head);
        
        gotoxy(new_head_x, new_head_y);
    }
}

void check_snake_collision() {
    /* Collision checking is now done in move_snake() function before actually moving */
    /* This function can be kept for compatibility but is no longer needed */
}

int check_position_availability(int x, int y) {
    struct SnakeSegment *temp_segment = snake_head;
    int i, j;
    
    /* Check if position conflicts with snake */
    if (temp_segment->x == x && temp_segment->y == y) {
        return 0;
    }
    temp_segment = temp_segment->next;
    while (temp_segment != snake_head) {
        if (temp_segment->x == x && temp_segment->y == y) {
            return 0;
        }
        temp_segment = temp_segment->next;
    }
    
    /* Check if position conflicts with barrier */
    if (is_position_on_barrier(x, y)) {
        return 0;
    }
    
    /* Check if the 4x2 food pattern would overlap with snake or barriers */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            /* Check boundaries */
            if (x + j > 79 || y + i > 24) {
                return 0;
            }
                
            /* Check snake collision for food pattern */
            temp_segment = snake_head;
            if (temp_segment->x == x + j && temp_segment->y == y + i) {
                return 0;
            }
            temp_segment = temp_segment->next;
            while (temp_segment != snake_head) {
                if (temp_segment->x == x + j && temp_segment->y == y + i) {
                    return 0;
                }
                temp_segment = temp_segment->next;
            }
            
            /* Check barrier collision for food pattern */
            if (is_position_on_barrier(x + j, y + i)) {
                return 0;
            }
        }
    }
        
    return 1;
}

void create_game_layout() {
    int i, j;
    
    /* Draw top border (row 1) */
    for (i = 60; i <= 80; i++) {
        put_character_at_position(i, 1, '*', WHITE);
    }
    
    /* Draw bottom border (row 25) */
    for (i = 1; i <= 80; i++) {
        put_character_at_position(i, 25, '*', WHITE);
    }
    
    /* Draw left border (column 1) */
    for (j = 2; j <= 24; j++) {
        put_character_at_position(1, j, '*', WHITE);
    }
    
    /* Draw right border (column 80) */
    for (j = 2; j <= 24; j++) {
        put_character_at_position(80, j, '*', WHITE);
    }
}

void display_game_over() {
    char user_choice;
    
    gotoxy(2, 27);
    /* Display blinking game over message */
    printf("GAME OVER !!!");
    
    do {
        user_choice = getch();
        if (user_choice == 'q') {
            exit(1);
        } else if (user_choice == 'r') {
            start_new_game();
        }
    } while (user_choice != 'q' || user_choice != 'r');
}