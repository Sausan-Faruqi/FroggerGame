#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <vector>
#include <iostream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 750;
const int PLAYER_SIZE = 32;
const int CAR_WIDTH = 64;
const int CAR_HEIGHT = 64;
const float CAR_SPEED = 3.0;
const int FPS = 60;

bool check_collision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {

    return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);

}


class Player {
public:

    float x, y;
    ALLEGRO_BITMAP* frogSprite;
    Player() {
        x = SCREEN_WIDTH / 2 - PLAYER_SIZE / 2;
        y = (SCREEN_HEIGHT - 80) - PLAYER_SIZE;
         frogSprite = al_load_bitmap("frogSprite.png");
         if (!frogSprite) {
             std::cerr << "Failed to load frog sprite.";
         }
    }

    void move(int dx, int dy) {
        x += dx * PLAYER_SIZE;
        y += dy * PLAYER_SIZE;

        if (x < 0) { x = 0; }
        if (y < 0) { y = 0; }
        if (x + PLAYER_SIZE > SCREEN_WIDTH) { x = SCREEN_WIDTH - PLAYER_SIZE; }
        if (y + PLAYER_SIZE > SCREEN_HEIGHT) { y = SCREEN_HEIGHT - PLAYER_SIZE; }
    }

    void draw() {
        al_draw_bitmap(frogSprite, x, y, 0);

       /* al_draw_filled_rectangle(x, y, x + PLAYER_SIZE, y + PLAYER_SIZE, al_map_rgb(0, 255, 0));*/

    }

    ~Player() { al_destroy_bitmap(frogSprite); }
};

class Car {

public:
    float x, y;
    float speed;
    ALLEGRO_BITMAP* sprite;

    Car(float startX, float startY, float speed, ALLEGRO_BITMAP* carSprite) :x(startX), y(startY), speed(speed), sprite(carSprite) {}
    void update() {
        x -= speed;
        if (x + CAR_WIDTH < 0) {
            x = SCREEN_WIDTH;
        }
    }
    void draw() {
        al_draw_bitmap(sprite, x, y, 0);
    }
};

class Log {
public:
    float x, y, speed;
    int width;
    ALLEGRO_BITMAP* sprite;
    Log(float startX, float startY, float speed, int width, ALLEGRO_BITMAP*logSprite) :x(startX), y(startY), speed(speed), width(width), sprite(logSprite) {}
    void update() {
        x += speed;
        if (x > SCREEN_WIDTH && speed > 0) {
            x = -width;
        }
        else if (x + width < 0 && speed < 0) {
            x = SCREEN_WIDTH;
        }
    }
    void draw() {

       /* al_draw_filled_rectangle(x, y, x + width, y + CAR_HEIGHT, al_map_rgb(139, 69, 19));*/
        if (sprite) {
            int spriteWidth = al_get_bitmap_width(sprite);
            int drawWidth;
            for (int i = 0; i < width; i += spriteWidth) {
                if (i + spriteWidth > width) { drawWidth = width - i; }
                else {
                    drawWidth = spriteWidth;
                }
                al_draw_bitmap_region(sprite, 0, 0, drawWidth, CAR_HEIGHT, x + i, y, 0);
            }
        }
        else {
            al_draw_filled_rectangle(x, y, x + width, y + CAR_HEIGHT, al_map_rgb(139, 69, 19));
        }
    }
    bool isPlayerOnLog(float px, float py, float pSize) {
        return ((py + pSize > y) && (py < y + CAR_HEIGHT) && (px + pSize > x) && (px < x + width));

    }
};



class Game {
private:
 
    ALLEGRO_BITMAP* background;
    int lives = 3;
    bool gameOver = false;
    ALLEGRO_DISPLAY* display;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_TIMER* timer;
    ALLEGRO_FONT* font1;
    ALLEGRO_FONT* font2;
    bool running = true;
    bool redraw = true;

    Player player;
    

    std::vector<Car>cars;
    std::vector<ALLEGRO_BITMAP*>carSprites;

    ALLEGRO_BITMAP* getRandomCarSprite() {
        if (carSprites.empty()) {
            return nullptr;
        }
        int index = rand() % carSprites.size();
        return carSprites[index];
    }

    ALLEGRO_BITMAP* logSprite;
    std::vector<Log>logs;
    bool onLog = false;


public:
    Game() {
        al_init();
        al_init_image_addon();
        al_init_font_addon();
        al_init_ttf_addon();
        font1 = al_load_ttf_font("PressStart2P-Regular.ttf", 15, 0);
        if (!font1) {
            std::cerr << "Failed to load font" << std::endl;
        }
        font2 = al_load_ttf_font("PressStart2P-Regular.ttf", 25, 0);
        if (!font2) {
            std::cerr << "Failed to load font" << std::endl;
        }

        al_init_primitives_addon();
        al_install_keyboard();

        display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
        queue = al_create_event_queue();
        timer = al_create_timer(1.0 / FPS);

        al_register_event_source(queue, al_get_display_event_source(display));
        al_register_event_source(queue, al_get_timer_event_source(timer));
        al_register_event_source(queue, al_get_keyboard_event_source());

        background = al_load_bitmap("background.png");
        if (!background) {
            std::cerr << "Failed to load background image." << std::endl;
        }


        carSprites.push_back(al_load_bitmap("car1.png"));
        carSprites.push_back(al_load_bitmap("car2.png"));
        carSprites.push_back(al_load_bitmap("car3.png"));
        carSprites.push_back(al_load_bitmap("car4.png"));
        for (auto& sprite : carSprites) {
            if (!sprite) {
                std::cerr << "Failed to load car sprite." << std::endl;
            }
        }


        logSprite = al_load_bitmap("log2.png");
        if (!logSprite) {
            std::cerr << "Failed to load log sprite." << std::endl;
        }
        
        cars.emplace_back(800, 530, CAR_SPEED, getRandomCarSprite());
        cars.emplace_back(400, 400, CAR_SPEED, getRandomCarSprite());
        cars.emplace_back(600, 420, CAR_SPEED, getRandomCarSprite());
        cars.emplace_back(500, 480, CAR_SPEED, getRandomCarSprite());


        logs.emplace_back(100, 150, 2.0, 150, logSprite);
        logs.emplace_back(400, 300, 2.0, 150, logSprite);
        logs.emplace_back(200, 200, -2.5, 180, logSprite);
        logs.emplace_back(600, 250, -2.5, 180, logSprite);




    }

    ~Game() {
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        al_destroy_font(font1);
        al_destroy_font(font2);
        if (background) {
            al_destroy_bitmap(background);
        }


        for (auto& sprite : carSprites) {
            al_destroy_bitmap(sprite);
        }
    }

    void run() {
        al_start_timer(timer);

        while (running) {
            ALLEGRO_EVENT ev;
            al_wait_for_event(queue, &ev);

            if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                running = false;
            }
            else if (ev.type == ALLEGRO_EVENT_TIMER) {
                update();
                redraw = true;
            }
            else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                handle_input(ev.keyboard.keycode);
            }

            if (redraw && al_is_event_queue_empty(queue)) {
                redraw = false;
                draw();
            }
        }
    }

    void handle_input(int keycode) {
        if (gameOver) { return; }
        switch (keycode) {
        case ALLEGRO_KEY_UP:
            player.move(0, -1);
            break;
        case ALLEGRO_KEY_DOWN:
            player.move(0, 1);
            break;
        case ALLEGRO_KEY_LEFT:
            player.move(-1, 0);
            break;
        case ALLEGRO_KEY_RIGHT:
            player.move(1, 0);
            break;
        case ALLEGRO_KEY_ESCAPE:
            running = false;
            break;
        }
    }


    void resetPlayer() {
        player.x = SCREEN_WIDTH / 2 - PLAYER_SIZE / 2;
        player.y = SCREEN_HEIGHT - PLAYER_SIZE - 10;
    }




    void update() {

        for (auto& log : logs) {
            log.update();
        }

        onLog = false;

        if (player.y < 350) {
            onLog = false;
            for (auto& log : logs) {
                if (log.isPlayerOnLog(player.x, player.y, PLAYER_SIZE)) {
                    onLog = true;
                    player.x += log.speed;

                    if (player.x < 0) {
                        player.x = 0;
                    }
                    if (player.x + PLAYER_SIZE > SCREEN_WIDTH) {
                        player.x = SCREEN_WIDTH - PLAYER_SIZE;
                    }
                    break;
                }
            }
            if (!onLog) {
                lives--;
                resetPlayer();
                if (lives <= 0) {
                    gameOver = true;
                }
            }


        }



        if (gameOver) { return; }

        for (auto& car : cars) {
            car.update();


            if (check_collision(player.x, player.y, PLAYER_SIZE, PLAYER_SIZE, car.x, car.y, CAR_WIDTH, CAR_HEIGHT)) {
                std::cout << "Player hit by car! Resetting position." << std::endl;
                player = Player();
                lives--;

                if (lives <= 0) {
                    gameOver = true;
                    std::cout << "Game Over" << std::endl;
                }
                else {
                    player = Player();
                }
                break;
            }
        }

    }

    void draw() {
        al_clear_to_color(al_map_rgb(0, 0, 0));

        
        if (background) {
            al_draw_bitmap(background, 0, 0, 0);
        }


        /* al_draw_filled_rectangle(0, 0, SCREEN_WIDTH, 200, al_map_rgb(0, 3, 67))*/;

        for (auto& log : logs) {
            log.draw();
        }

        for (auto& car : cars) {
            car.draw();
        }

        al_draw_textf(font1, al_map_rgb(255, 255, 255), 10, 10, 0, "Lives: %d", lives);
        if (gameOver) {
            al_draw_textf(font2, al_map_rgb(255, 0, 0), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "Game Over");

        }
        player.draw();
        al_flip_display();
    }
};

int main() {
    al_init();
    al_init_image_addon();
    srand(time(NULL));


    Game game;
    game.run();
    return 0;

}
