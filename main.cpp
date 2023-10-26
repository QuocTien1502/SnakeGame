#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for(unsigned int i = 0; i < deque.size(); i++)
    {
        if(Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval)
{
    double currentTime = GetTime();
    if(currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
    Vector2 direction = {1,0};
    bool addSement = false;

    void Draw()
    {
        for(unsigned i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset+x*cellSize, offset+y*cellSize, cellSize, cellSize};
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if(addSement)
        {
           addSement = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {
        body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
        direction = {1,0};
    }
};

class Food
{
public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset+position.x * cellSize, offset+position.y * cellSize, WHITE);
    }

    Vector2 GenarateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x,y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenarateRandomCell();
        while(ElementInDeque(position, snakeBody))
        {
            position = GenarateRandomCell();
        }
        return position;

    }

};


class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    int eatCount = 0;
    int level = 1;
    float levelUpTextDisplayTime = 0.0f;
    string levelUpText = "";
    Vector2 levelUpTextPos;
    Sound eatSound;
    Sound wallSound;
    Sound endgameSound;
    Sound jumpSound;
    Sound levelupSound;
    Music backgroundMusic;


    bool isBackgroundMusicPlaying = true;

    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.wav");
        wallSound = LoadSound("Sounds/wall.mp3");
        jumpSound = LoadSound("Sounds/jump.wav");
        endgameSound = LoadSound("Sounds/endgame.mp3");
        levelupSound = LoadSound("Sounds/levelup.wav");

        backgroundMusic = LoadMusicStream("Sounds/mario.mp3");
        PlayMusicStream(backgroundMusic);

    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        UnloadSound(endgameSound);
        UnloadSound(jumpSound);
        UnloadSound(levelupSound);

        UnloadMusicStream(backgroundMusic);

        CloseAudioDevice();
    }
    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if(running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }

    }

    void CheckCollisionWithFood()
    {
        if(Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSement = true;
            eatCount++;
            if(eatCount >= 3)
            {
                eatCount = 0;
                level++;
                PlaySound(levelupSound);
                SetLevelUpText();
            }
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges()
    {
        if(snake.body[0].x == cellCount || snake.body[0].x == -1)
        {
            GameOver();
            //StopBackgroundMusic();
        }
        if(snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
            //StopBackgroundMusic();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
        PlaySound(endgameSound);
        StopBackgroundMusic();
        level = 1;
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if(ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
            //StopBackgroundMusic();
        }
    }

    void StopBackgroundMusic()
    {
        StopMusicStream(backgroundMusic);
        /*if(isBackgroundMusicPlaying)
        {
            StopMusicStream(backgroundMusic);
            isBackgroundMusicPlaying = false;
        }*/
    }

    void StartBackgroundMusic()
    {
        if (!IsMusicStreamPlaying(backgroundMusic))
        {
            PlayMusicStream(backgroundMusic);
        }
    }

    /*int LevelUp()
    {
        eatCount++;
        if(eatCount >= 3)
        {
            eatCount = 0;
            level++;
        }
        return level;
    }*/
    void SetLevelUpText()
    {
        levelUpText = "Level Up";
        levelUpTextPos.x = (GetScreenWidth() - MeasureText(levelUpText.c_str(), 30)) / 2;
        levelUpTextPos.y = GetScreenHeight() / 2;
        levelUpTextDisplayTime = 2.0f;
    }

};

int main () {
    cout<<"Starting the game ..."<<endl;
    InitWindow(2*offset + cellSize*cellCount, 2*offset + cellSize*cellCount, "Snake Game");
    SetTargetFPS(60);

    Game game = Game();

    while(WindowShouldClose()== false)
    {
        BeginDrawing();

        if(eventTriggered(1.3))
        {
            game.Update();
        }

        if(IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
        {
            //PlaySound(game.jumpSound);
            game.snake.direction = {0,-1};
            game.running = true;
        }

        if(IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
        {
           //PlaySound(game.jumpSound);
            game.snake.direction = {0,1};
            game.running = true;
        }

        if(IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
        {
            //PlaySound(game.jumpSound);;
            game.snake.direction = {-1,0};
            game.running = true;
        }

        if(IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
        {
            //PlaySound(game.jumpSound);
            game.snake.direction = {1,0};
            game.running = true;
        }
        if (game.running)
        {
            game.StartBackgroundMusic();
        }

        UpdateMusicStream(game.backgroundMusic);
        if (game.levelUpTextDisplayTime > 0)
        {
            DrawText(game.levelUpText.c_str(), game.levelUpTextPos.x, game.levelUpTextPos.y, 30, DARKGRAY);
            game.levelUpTextDisplayTime -= GetFrameTime();
        }

        //Drawing
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset-5, (float)offset-5, (float)cellSize*cellCount+10, (float)cellSize*cellCount+10},5,darkGreen);
        DrawText(TextFormat("Cowboy Snake       Level : %i",game.level), offset-5, 20, 40, darkGreen);


        DrawText(TextFormat("Score : %i",game.score), offset-5, offset+cellSize*cellCount+10, 40, darkGreen);

        game.Draw();



        EndDrawing();
    }


    CloseWindow();
    return 0;
}
