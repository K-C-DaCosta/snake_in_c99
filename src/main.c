#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <math.h>

// I make a ring buffer to represent the snake
typedef struct _RingBuffer
{
    int len;
    int capacity;
    int front;
    int rear;
    void *buffer;
} RingBuffer;

RingBuffer rbCreatei32(int capacity)
{
    return (RingBuffer){
        0,
        capacity,
        0,
        0,
        (int *)calloc(capacity, sizeof(int))};
}

bool rbIsFull(RingBuffer *rb)
{
    return rb->len == rb->capacity;
}

int rbLen(RingBuffer *rb)
{
    return rb->len;
}

bool rbEnqueuei32(RingBuffer *rb, int item)
{
    if (rbIsFull(rb))
    {
        return false;
    }
    int *buffer = (int *)rb->buffer;
    buffer[rb->rear] = item;
    rb->rear = (rb->rear + 1) % rb->capacity;
    rb->len += 1;
    return true;
}

int rbDequeuei32(RingBuffer *rb)
{
    int *buffer = (int *)rb->buffer;
    if (rb->len > 0)
    {
        int old_front = buffer[rb->front];
        rb->front = (rb->front + 1) % rb->capacity;
        rb->len -= 1;
        return old_front;
    }
    return 0;
}

int rbGetFronti32(RingBuffer *rb)
{
    int *buffer = (int *)rb->buffer;
    return buffer[rb->front];
}

int rbGetReari32(RingBuffer *rb)
{
    int *buffer = (int *)rb->buffer;
    int rear = mod(rb->rear - 1, rb->capacity);
    return buffer[rear];
}

void rbFree(RingBuffer *rb)
{
    free(rb->buffer);
}
int mod(int a, int b)
{
    int r = a % b;
    return (r < 0) ? r + b : r;
}

enum TileType
{
    EMPTY_SPACE = 0,
    SNAKE = 1,
    FOOD = 2,
};

typedef struct _SnakeGrid
{
    int rows, cols;
    int tile_w, tile_h;
    char *board_map;
} SnakeGrid;

SnakeGrid sgNew(int rows, int cols, int tile_w, int tile_h)
{
    return (SnakeGrid){
        rows,
        cols,
        tile_w,
        tile_h,
        calloc(rows * cols, sizeof(char)),
    };
}

void sgDrawGrid(SnakeGrid grid, SDL_Renderer *r)
{
    SDL_SetRenderDrawColor(r, 255, 128, 64, 255);
    SDL_RenderDrawRect(r, &(SDL_Rect){0, 0, grid.tile_w * grid.cols, grid.tile_h * grid.rows});
    int color_table[4][4] = {
        {0, 0, 0, 0},
        {0, 255, 0, 255},
        {255, 255, 255, 255},
    };
    for (int i = 0; i < grid.rows; i++)
    {
        for (int j = 0; j < grid.cols; j++)
        {
            char tile_val = grid.board_map[i * grid.cols + j];
            if (tile_val >= EMPTY_SPACE && tile_val <= FOOD)
            {
                int *color = color_table[tile_val];
                SDL_SetRenderDrawColor(r, color[0], color[1], color[2], color[3]);
            }
            SDL_RenderFillRect(r, &(SDL_Rect){j * grid.tile_w + 1, i * grid.tile_h + 1, grid.tile_w - 2, grid.tile_h - 2});
        }
    }
}

void sgFree(SnakeGrid sg)
{
    free(sg.board_map);
}

int gi(int x, int y, int cols)
{
    return y * cols + x;
}

bool move_snake(RingBuffer *snake, SnakeGrid grid, int dir[2], int *step_time);

void set_dir(int entry, int dir_table[256][2], int dir[2])
{
    for (int k = 0; k < 2; k++)
        dir_table[entry][k] = dir[k];
}

int main()
{

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_Window *window = SDL_CreateWindow("snake_game",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          800,
                                          600,
                                          SDL_WINDOW_RESIZABLE);

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED);

    SDL_Event event;

    bool running = true;
    SnakeGrid grid = sgNew(20, 20, 30, 30);
    RingBuffer snake = rbCreatei32(20 * 20);
    int first_snake_block = gi(grid.cols / 2, grid.rows / 2, grid.cols);
    rbEnqueuei32(&snake, first_snake_block);
    grid.board_map[first_snake_block] = SNAKE;
    grid.board_map[50] = FOOD;
    int snake_dir[] = {1, 0};

    int direction_table[255][2] = {{0, 0}};
    set_dir('w', direction_table, &(int[2]){0, -1});
    set_dir('s', direction_table, &(int[2]){0, 1});
    set_dir('a', direction_table, &(int[2]){-1, 0});
    set_dir('d', direction_table, &(int[2]){1, 0});
    int step_time = 100;
    bool changed_dir = false;
    int new_dir[2] = {0};
    while (running)
    {
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.quit.type == SDL_QUIT)
            {
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN)
            {

                changed_dir = true;
                for (int k = 0; k < 2; k++)
                    new_dir[k] = direction_table[event.key.keysym.sym % 256][k];
            }
        }
        if (changed_dir)
        {
            int cross = new_dir[0] * snake_dir[1] - new_dir[1] * snake_dir[0];
            if (cross != 0)
            {
                snake_dir[0] = new_dir[0];
                snake_dir[1] = new_dir[1];
            }
            changed_dir = false;
        }

        if (move_snake(&snake, grid, snake_dir, &step_time) == false)
        {
            printf("You died\n");
            running = false;
        }

        printf("snake len = %d\n", snake.len);

        //draw scene
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        sgDrawGrid(grid, renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(step_time);
    }

    sgFree(grid);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

bool move_snake(RingBuffer *snake, SnakeGrid grid, int dir[2], int *step_time)
{
    //get current snake head
    int current_snake_head = rbGetReari32(snake);

    // compute new_snake head and add it
    int head_pos[2] = {
        (current_snake_head % grid.cols) + dir[0],
        (current_snake_head / grid.cols) + dir[1],
    };

    // Out-of-bounds check
    if (head_pos[0] < 0 || head_pos[0] >= grid.cols ||
        head_pos[1] < 0 || head_pos[1] >= grid.rows)
    {
        return false;
    }

    int new_snake_head = gi(head_pos[0], head_pos[1], grid.cols);
    // the 'snake' is represented in memory as a ring buffer and this
    // for-loop searches through ring buffer to check if snake is eating itself.
    for (int k = 0; k < rbLen(snake); k += 1)
    {
        int snake_body = ((int *)snake->buffer)[(snake->front + k) % snake->capacity];
        if (new_snake_head == snake_body)
        {
            return false;
        }
    }

    char old_tile_pos = grid.board_map[new_snake_head];

    grid.board_map[new_snake_head] = SNAKE;
    rbEnqueuei32(snake, new_snake_head);

    //remove tail of snake (to preserve length)
    if (old_tile_pos != FOOD)
    {
        int old_snake_tail = rbDequeuei32(snake);
        grid.board_map[old_snake_tail] = EMPTY_SPACE;
    }

    if (old_tile_pos == FOOD)
    {
        int x = grid.cols / 2;
        int y = grid.rows / 2;
        // if space is occupied roll random position
        while (grid.board_map[gi(x, y, grid.cols)] != EMPTY_SPACE)
        {
            x = rand() % grid.cols;
            y = rand() % grid.rows;
        }
        grid.board_map[gi(x, y, grid.cols)] = FOOD;
        if (step_time)
        {
            // step_time[0]=(step_time[0]*7)/10;
            // step_time[0]= (step_time[0]>60)?step_time[0]: 60;
        }
    }

    return true;
}