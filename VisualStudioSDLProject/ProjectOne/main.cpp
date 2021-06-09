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

float cloud_x = 0;
float cloud_y = 0;
float cloudx_direction = 1.0f;
float cloudy_direction = 1.0f;
float star_size = 1.0f;
float star_size_direction = 0;
float star_rotate = 0;


GLuint cloudID, starID;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;
glm::mat4 cloudMatrix, starMatrix;

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

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project One!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    cloudMatrix = glm::mat4(1.0f);
    starMatrix = glm::mat4(1.0f);

    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    //program.SetColor(0.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(program.programID);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    cloudID = LoadTexture("cloud.png");
    starID = LoadTexture("star.png");
}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
}

float lastTicks = 0.0f;

void Update() {
    // Cloud
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    if (cloud_x >= 4.0f) {
        cloudx_direction = -1.0f;
    }
    if (cloud_x <= 0) {
        cloudx_direction = 1.0f;
    }
    if (cloud_y >= 0.2f) {
        cloudy_direction = -0.1f;
    }
    if (cloud_y <= 0.0f) {
        cloudy_direction = 0.1f;
    }
    cloud_x += cloudx_direction * deltaTime;
    cloud_y += cloudy_direction * deltaTime;
    cloudMatrix = glm::mat4(1.0f);
    cloudMatrix = glm::translate(cloudMatrix, glm::vec3(-4.0f, 2.0f, 0.0f));
    cloudMatrix = glm::scale(cloudMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
    cloudMatrix = glm::translate(cloudMatrix, glm::vec3(cloud_x, cloud_y, 0.0f));
    

    // Star
    if (star_size >= 2.0f) {
        star_size_direction = -0.3f;
    }
    if (star_size <= 1.0f) {
        star_size_direction = 0.3f;
    }
    star_rotate += deltaTime * 90.0f;
    star_size += star_size_direction * deltaTime;
    starMatrix = glm::mat4(1.0f);
    starMatrix = glm::translate(starMatrix, glm::vec3(0.0f, 2.0f, 0.0f));
    starMatrix = glm::scale(starMatrix, glm::vec3(star_size, star_size, 1.0f));
    starMatrix = glm::rotate(starMatrix, glm::radians(star_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    
    
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    //Draw cloud
    program.SetModelMatrix(cloudMatrix);
    glBindTexture(GL_TEXTURE_2D, cloudID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //Draw star
    program.SetModelMatrix(starMatrix);
    glBindTexture(GL_TEXTURE_2D, starID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

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