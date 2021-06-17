#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;



ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;
glm::mat4 p1Matrix, p2Matrix, ballMatrix;
glm::vec3 p1_position = glm::vec3(-4.8f, 0, 0);
glm::vec3 p2_position = glm::vec3(4.8f, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);
glm::vec3 p1_movement = glm::vec3(0, 0, 0);
glm::vec3 p2_movement = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(1, -1, 0);
float p1speed = 5.0f;
float p2speed = 5.0f;
float ballspeed = 4.0f;


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    ballMatrix = glm::mat4(1.0f);
    p1Matrix = glm::mat4(1.0f);
    p2Matrix = glm::mat4(1.0f);

    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    

    glUseProgram(program.programID);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_UP:
                //p2_movement.y = 1.0f;
                break;

            case SDLK_DOWN:
                //p2_movement.y = -1.0f;
                break;

            case SDLK_w:
                //p1_movement.y = 1.0f;
                break;

            case SDLK_s:
                //p1_movement.y = -1.0f;
                break;
            }
            break; // SDL_KEYDOWN
        }
    }
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_UP]) {
        p2_movement.y = 1.0f;
    }
    
    if (keys[SDL_SCANCODE_DOWN]) {
        p2_movement.y = -1.0f;
    }
    if ((keys[SDL_SCANCODE_UP] == 0) && (keys[SDL_SCANCODE_DOWN] == 0)) {
        p2_movement.y = 0.0f;
    }
    if (keys[SDL_SCANCODE_W]) {
        p1_movement.y = 1.0f;
    }
    if (keys[SDL_SCANCODE_S]) {
        p1_movement.y = -1.0f;
    }
    if ((keys[SDL_SCANCODE_S] ==0) && (keys[SDL_SCANCODE_W] == 0)) {
        p1_movement.y = 0.0f;
    }
    if (glm::length(p1_movement) > 1.0f) {
        p1_movement = glm::normalize(p1_movement);
    }
    if (glm::length(p2_movement) > 1.0f) {
        p2_movement = glm::normalize(p2_movement);
    }

}

float lastTicks = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    p1_position += p1_movement * p1speed * deltaTime;
    if (p1_position.y > 3.1f) {
        p1_position.y = 3.1f;
    }
    if (p1_position.y < -3.1f) {
        p1_position.y = -3.1f;
    }
    p1Matrix = glm::mat4(1.0f);
    p1Matrix = glm::translate(p1Matrix, p1_position);

    // Draw p2
    p2_position += p2_movement * p2speed * deltaTime;
    if (p2_position.y > 3.1f) {
        p2_position.y = 3.1f;
    }
    if (p2_position.y < -3.1f) {
        p2_position.y = -3.1f;
    }
    p2Matrix = glm::mat4(1.0f);
    p2Matrix = glm::translate(p2Matrix, p2_position);

    // Draw the ball
    // Detect whether ball hits the left/right boundary
    if (ball_position.x >= 4.8f || ball_position.x <= -4.8f) {
        ballspeed = 0;
        p1speed = 0;
        p2speed = 0;
    }
    //ball is colliding with p1 on the left
    if ((fabs(ball_position.x-p1_position.x)-0.3)<0 && (fabs(ball_position.y - p1_position.y) - 0.75) < 0) {
        ball_movement.x = 1.0f;
    }
    //ball is colliding with p2 on the right
    if ((fabs(ball_position.x - p2_position.x) - 0.3) < 0 && (fabs(ball_position.y - p2_position.y) - 0.75) < 0) {
        ball_movement.x = -1.0f;
    }
    //ball hits the boundary
    if (ball_position.y >= 3.55f) {
        ball_position.y = 3.55f;
        ball_movement.y = -1.0f;
    } 
    if (ball_position.y <= -3.55f) {
        ball_position.y = -3.55f;
        ball_movement.y = 1.0f;
    }
    ball_position += ball_movement * ballspeed * deltaTime;
    ballMatrix = glm::mat4(1.0f);
    ballMatrix = glm::translate(ballMatrix, ball_position);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw the ball
    float ballvertices[] = { -0.15, -0.15, 0.15, -0.15, 0.15, 0.15, -0.15, -0.15, 0.15, 0.15, -0.15, 0.15 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballvertices);
    glEnableVertexAttribArray(program.positionAttribute);
    program.SetModelMatrix(ballMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw player 1 on the left
    float p1vertices[] = { -0.15, -0.6, 0.15, -0.6, 0.15, 0.6, -0.15, -0.6, 0.15, 0.6, -0.15, 0.6 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, p1vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    program.SetModelMatrix(p1Matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw player 2 on the right
    float p2vertices[] = { -0.15, -0.6, 0.15, -0.6, 0.15, 0.6, -0.15, -0.6, 0.15, 0.6, -0.15, 0.6 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, p2vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    program.SetModelMatrix(p2Matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program.positionAttribute);

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