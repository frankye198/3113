#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

#define FIXED_TIMESTEP 0.007f
#define PLATFORM_COUNT 51


struct GameState {
    Entity* player;
    Entity* bombs;
    Entity* platforms;
    Entity* explosion1;
};



GameState state;
SDL_Window* displayWindow;
ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix;
bool gameIsRunning = true;
float lastTicks = 0;
float accumulator = 0.0f;

bool facingLeft = true;

int stage = 0;


GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_image_free(image);
    return textureID;
}

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position) {
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;
    std::vector<float> vertices;
    std::vector<float> texCoords;
    for (int i = 0; i < text.size(); i++) {
        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        vertices.insert(vertices.end(), {
        offset + (-0.5f * size), 0.5f * size,
        offset + (-0.5f * size), -0.5f * size,
        offset + (0.5f * size), 0.5f * size,
        offset + (0.5f * size), -0.5f * size,
        offset + (0.5f * size), 0.5f * size,
        offset + (-0.5f * size), -0.5f * size,
            });
        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
            });
    }
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Lunar Lander!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 1920, 1080);
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    program.SetViewMatrix(viewMatrix);
    projectionMatrix = glm::ortho(-8.0f, 8.0f, -4.5f, 4.5f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);

    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    glUseProgram(program.programID);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    /*------explosion1 field------*/
    state.explosion1 = new Entity(glm::vec3(0.0f, 3.5f, 0), glm::vec3(1, 0, 0), 0);
    state.explosion1->isActive = false;
    state.explosion1->textureID = LoadTexture("explosion.png");
    state.explosion1->movement.x = 1;
    state.explosion1->animCols = 4;
    state.explosion1->animRows = 4;
    state.explosion1->animFrames = 16;
    state.explosion1->animIndices = new int[16]{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    /*------player field------*/
    state.player = new Entity(glm::vec3(0, 3.5, 0), glm::vec3(0), 1.0f);
    state.player->isActive = false;
    state.player->acceleration.y = -0.3f;
    state.player->textureID = LoadTexture("player.png");
    state.player->height = 1;
    state.player->width = 0.8;
    state.player->entityType = PLAYER;


    /*------platforms field------*/
    state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platformTextureID1 = LoadTexture("engine1.png");
    GLuint platformTextureID2 = LoadTexture("platformPack_tile007.png");
    GLuint platformTextureID3 = LoadTexture("platformPack_tile008.png");
    GLuint platformTextureID4 = LoadTexture("bomb_circle.png");

    state.platforms[0].textureID = platformTextureID1;
    state.platforms[0].position = glm::vec3(0.5, -4, 0);
    state.platforms[0].entityType = GOAL;

    state.platforms[1].textureID = platformTextureID1;
    state.platforms[1].position = glm::vec3(-0.5, -4, 0);
    state.platforms[1].entityType = GOAL;

    state.platforms[2].textureID = platformTextureID1;
    state.platforms[2].position = glm::vec3(-1.5, -4, 0);
    state.platforms[2].entityType = GOAL;

    state.platforms[3].textureID = platformTextureID3;
    state.platforms[3].position = glm::vec3(-7.5, -4, 0);
    state.platforms[3].entityType = WALL;

    state.platforms[4].textureID = platformTextureID2;
    state.platforms[4].position = glm::vec3(-6.5, -4, 0);

    state.platforms[5].textureID = platformTextureID2;
    state.platforms[5].position = glm::vec3(-5.5, -4, 0);

    state.platforms[6].textureID = platformTextureID2;
    state.platforms[6].position = glm::vec3(-4.5, -4, 0);

    state.platforms[7].textureID = platformTextureID2;
    state.platforms[7].position = glm::vec3(-3.5, -4, 0);

    state.platforms[8].textureID = platformTextureID2;
    state.platforms[8].position = glm::vec3(-2.5, -4, 0);

    state.platforms[9].textureID = platformTextureID2;
    state.platforms[9].position = glm::vec3(2.5, -4, 0);

    state.platforms[10].textureID = platformTextureID2;
    state.platforms[10].position = glm::vec3(3.5, -4, 0);

    state.platforms[11].textureID = platformTextureID2;
    state.platforms[11].position = glm::vec3(4.5, -4, 0);

    state.platforms[12].textureID = platformTextureID2;
    state.platforms[12].position = glm::vec3(1.5, -4, 0);

    state.platforms[13].textureID = platformTextureID2;
    state.platforms[13].position = glm::vec3(5.5, -4, 0);

    state.platforms[14].textureID = platformTextureID2;
    state.platforms[14].position = glm::vec3(6.5, -4, 0);

    state.platforms[15].textureID = platformTextureID3;
    state.platforms[15].position = glm::vec3(7.5, -4, 0);
    state.platforms[15].entityType = WALL;

    state.platforms[16].textureID = platformTextureID3;
    state.platforms[16].position = glm::vec3(7.5, -3, 0);
    state.platforms[16].entityType = WALL;

    state.platforms[17].textureID = platformTextureID3;
    state.platforms[17].position = glm::vec3(7.5, -2, 0);
    state.platforms[17].entityType = WALL;

    state.platforms[18].textureID = platformTextureID3;
    state.platforms[18].position = glm::vec3(7.5, -1, 0);
    state.platforms[18].entityType = WALL;

    state.platforms[19].textureID = platformTextureID3;
    state.platforms[19].position = glm::vec3(7.5, 0, 0);
    state.platforms[19].entityType = WALL;

    state.platforms[20].textureID = platformTextureID3;
    state.platforms[20].position = glm::vec3(7.5, 1, 0);
    state.platforms[20].entityType = WALL;

    state.platforms[21].textureID = platformTextureID3;
    state.platforms[21].position = glm::vec3(7.5, 2, 0);
    state.platforms[21].entityType = WALL;

    state.platforms[22].textureID = platformTextureID3;
    state.platforms[22].position = glm::vec3(7.5, 3, 0);
    state.platforms[22].entityType = WALL;

    state.platforms[23].textureID = platformTextureID3;
    state.platforms[23].position = glm::vec3(7.5, 4, 0);
    state.platforms[23].entityType = WALL;

    state.platforms[24].textureID = platformTextureID3;
    state.platforms[24].position = glm::vec3(-7.5, -3, 0);
    state.platforms[24].entityType = WALL;

    state.platforms[25].textureID = platformTextureID3;
    state.platforms[25].position = glm::vec3(-7.5, -2, 0);
    state.platforms[25].entityType = WALL;

    state.platforms[26].textureID = platformTextureID3;
    state.platforms[26].position = glm::vec3(-7.5, -1, 0);
    state.platforms[26].entityType = WALL;

    state.platforms[27].textureID = platformTextureID3;
    state.platforms[27].position = glm::vec3(-7.5, 0, 0);
    state.platforms[27].entityType = WALL;

    state.platforms[28].textureID = platformTextureID3;
    state.platforms[28].position = glm::vec3(-7.5, 1, 0);
    state.platforms[28].entityType = WALL;

    state.platforms[29].textureID = platformTextureID3;
    state.platforms[29].position = glm::vec3(-7.5, 2, 0);
    state.platforms[29].entityType = WALL;

    state.platforms[30].textureID = platformTextureID3;
    state.platforms[30].position = glm::vec3(-7.5, 3, 0);
    state.platforms[30].entityType = WALL;

    state.platforms[31].textureID = platformTextureID3;
    state.platforms[31].position = glm::vec3(-7.5, 4, 0);
    state.platforms[31].entityType = WALL;

    state.platforms[32].textureID = platformTextureID4;
    state.platforms[32].position = glm::vec3(-6.5, -3.5, 0);
    state.platforms[32].entityType = BOMB;

    state.platforms[33].textureID = platformTextureID4;
    state.platforms[33].position = glm::vec3(-5.5, -3.5, 0);
    state.platforms[33].entityType = BOMB;

    state.platforms[34].textureID = platformTextureID4;
    state.platforms[34].position = glm::vec3(-4.5, -3.5, 0);
    state.platforms[34].entityType = BOMB;

    state.platforms[35].textureID = platformTextureID4;
    state.platforms[35].position = glm::vec3(-3.5, -3.5, 0);
    state.platforms[35].entityType = BOMB;

    state.platforms[36].textureID = platformTextureID4;
    state.platforms[36].position = glm::vec3(-2.5, -3.5, 0);
    state.platforms[36].entityType = BOMB;

    state.platforms[37].textureID = platformTextureID4;
    state.platforms[37].position = glm::vec3(2.5, -3.5, 0);
    state.platforms[37].entityType = BOMB;

    state.platforms[38].textureID = platformTextureID4;
    state.platforms[38].position = glm::vec3(3.5, -3.5, 0);
    state.platforms[38].entityType = BOMB;

    state.platforms[39].textureID = platformTextureID4;
    state.platforms[39].position = glm::vec3(4.5, -3.5, 0);
    state.platforms[39].entityType = BOMB;

    state.platforms[40].textureID = platformTextureID4;
    state.platforms[40].position = glm::vec3(1.5, -3.5, 0);
    state.platforms[40].entityType = BOMB;

    state.platforms[41].textureID = platformTextureID4;
    state.platforms[41].position = glm::vec3(5.5, -3.5, 0);
    state.platforms[41].entityType = BOMB;

    state.platforms[42].textureID = platformTextureID4;
    state.platforms[42].position = glm::vec3(6.5, -3.5, 0);
    state.platforms[42].entityType = BOMB;

    state.platforms[43].textureID = platformTextureID3;
    state.platforms[43].position = glm::vec3(-1, 0, 0);
    state.platforms[43].entityType = WALL;

    state.platforms[44].textureID = platformTextureID3;
    state.platforms[44].position = glm::vec3(0, 0, 0);
    state.platforms[44].entityType = WALL;

    state.platforms[45].textureID = platformTextureID3;
    state.platforms[45].position = glm::vec3(1, 2, 0);
    state.platforms[45].entityType = WALL;

    state.platforms[46].textureID = platformTextureID3;
    state.platforms[46].position = glm::vec3(2, 2, 0);
    state.platforms[46].entityType = WALL;

    state.platforms[47].textureID = platformTextureID4;
    state.platforms[47].position = glm::vec3(-1, 0.5, 0);
    state.platforms[47].entityType = BOMB;

    state.platforms[48].textureID = platformTextureID4;
    state.platforms[48].position = glm::vec3(0, 0.5, 0);
    state.platforms[48].entityType = BOMB;

    state.platforms[49].textureID = platformTextureID4;
    state.platforms[49].position = glm::vec3(1, 2.5, 0);
    state.platforms[49].entityType = BOMB;

    state.platforms[50].textureID = platformTextureID4;
    state.platforms[50].position = glm::vec3(2, 2.5, 0);
    state.platforms[50].entityType = BOMB;

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Update(0, NULL, 0);
    }

}

void ProcessInput() {
    state.player->movement = glm::vec3(0);
    state.player->acceleration.x = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_SPACE:
                if (state.player->collidedBottom)
                    state.player->jump = true;
                break;
            }
            break;
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_SPACE]) {

    }

    if (keys[SDL_SCANCODE_A]) {
        state.player->acceleration.x = -2;
    }
    else if (keys[SDL_SCANCODE_D]) {
        state.player->acceleration.x = 2;
    }

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void Update() {

    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {

        switch (stage) {
        case 0:

            state.player->isActive = true;
            state.player->Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
            if (state.player->lastCollision == WALL) stage = 2;
            else if (state.player->lastCollision == GOAL) stage = 3;
            else if (state.player->lastCollision == BOMB) stage = 1;
            break;
        case 1:
            state.player->isActive = false;
            state.explosion1->position = state.player->position;
            state.explosion1->isActive = true;
            state.explosion1->Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
            if (state.explosion1->animIndex == 15) stage = 2;
            break;
        }

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    GLuint font = LoadTexture("font1.png");

    if (stage == 1 || stage == 2) DrawText(&program, font, "GG!", 1.25, -0.25, glm::vec3(-1, 1, 0));

    if (stage == 3) DrawText(&program, font, "Good job!", 1.25, -0.25, glm::vec3(-4, 1, 0));

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Render(&program);
    }

    state.player->Render(&program);

    state.explosion1->Render(&program);

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}