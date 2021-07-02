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

enum EntityType { PLAYER, WALL, BOMB, GOAL, DEFAULT };

class Entity {
public:
	EntityType entityType = DEFAULT;
	EntityType lastCollision = DEFAULT;

	glm::mat4 modelMatrix;
	glm::vec3 position;
	glm::vec3 movement;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	GLuint textureID = NULL;

	float width = 1;
	float height = 1;
	float speed = 0;
	float jumpPower = 0;

	bool jump = false;
	bool isActive = true;
	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;

	int animFrames = 0;
	int animIndex = 0;
	float animTime = 0;
	int animCols = 0;
	int animRows = 0;
	int* animRight = NULL;
	int* animLeft = NULL;
	int* animUp = NULL;
	int* animDown = NULL;
	int* animIndices = NULL;

	Entity();
	Entity(glm::vec3, glm::vec3, float);
	bool CheckCollision(Entity*);
	void CheckCollisionsX(Entity*, int);
	void CheckCollisionsY(Entity*, int);
	void DrawSpriteFromTextureAtlas(ShaderProgram*, GLuint, int);
	void Update(float, Entity*, int);
	void Render(ShaderProgram*);
};
