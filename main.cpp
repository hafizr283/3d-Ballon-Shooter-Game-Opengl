#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "shader.h"
#include "basic_camera.h"
#include "pointLight.h"
#include "sphere.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "cone.h"
#include "stb_image.h"

// Globals
unsigned int texAsphalt, texBark, texBricks, texGrass, texLeaf, texRock, texConcrete, texSail;
bool texturesEnabled = true;

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <array>
#include <string>

//global283
int balloonColor = 0;

using namespace std;

enum class FireMode;

struct Projectile {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::quat rotation;
    bool isArrow;
    bool stuck;
    float stuckTimer;
    bool active;

    Projectile(glm::vec3 pos, glm::vec3 vel, bool arrow)
        : position(pos), velocity(vel), rotation(glm::quatLookAt(glm::normalize(vel), glm::vec3(0.0f, 1.0f, 0.0f))),
          isArrow(arrow), stuck(false), stuckTimer(0.0f), active(true) {}
};

struct Raindrop {
    glm::vec3 position;
    float speed;
    bool active;

    Raindrop(glm::vec3 pos, float spd)
        : position(pos), speed(spd), active(true) {}
};

std::vector<Projectile> projectiles;
float lastShootTime = 0.0f;
float shootCooldown = 0.2f;
std::vector<Raindrop> raindrops;
bool isRaining = false;
float thunderFlashTimer = 0.0f;
const float thunderFlashDuration = 0.12f;

static glm::mat4 rotate_manual(glm::mat4 mat, float deg, glm::vec3 axis)
{
    float const c = cos(deg);
    float const s = sin(deg);

    axis = glm::normalize(axis);
    glm::vec3 temp = (1.0f - c) * axis;

    glm::mat4 rotate = glm::mat4(1.0f);

    rotate[0][0] = c + temp[0] * axis[0];
    rotate[0][1] = temp[0] * axis[1] + s * axis[2];
    rotate[0][2] = temp[0] * axis[2] - s * axis[1];

    rotate[1][0] = temp[1] * axis[0] - s * axis[2];
    rotate[1][1] = c + temp[1] * axis[1];
    rotate[1][2] = temp[1] * axis[2] + s * axis[0];

    rotate[2][0] = temp[2] * axis[0] + s * axis[1];
    rotate[2][1] = temp[2] * axis[1] - s * axis[0];
    rotate[2][2] = c + temp[2] * axis[2];

    return mat * rotate;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const * path);
void fireBullet();
void fireArrow();
void setFireMode(FireMode mode, bool showToast = true);
void updateProjectileSystem(float dt);
void updateAmmoReloadSystem(float dt);
void updateScenarioTour(float dt);
void drawCombatHUD(unsigned int& cubeVAO, Shader& hudSader);
void drawCube(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 color,
    glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0),
    glm::vec3 rot = glm::vec3(0.0, 0.0, 0.0),
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0),
    unsigned int texID = 0, int texMode = 0, float texScale = 1.0f);
void drawBalloon(unsigned int& cubeVAO, Sphere& sphere, Shader& lightingShader, glm::vec3 position, float scaleMult = 1.0f);
void drawTower(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position);
void drawCloud(unsigned int& cubeVAO, Sphere& sphere, Shader& lightingShader, glm::vec3 position);
void drawRoad(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position);
void drawTree(unsigned int& cubeVAO, Sphere& sphere, Shader& lightingShader, glm::vec3 position);
void drawMountain(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position, glm::vec3 scale, glm::vec3 color);
void drawBuilding(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position, float width, float height, float depth, glm::vec3 wallColor, glm::vec3 roofColor);
void drawFencePost(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position);
void drawFractalBranch(unsigned int& cubeVAO, Shader& lightingShader, glm::mat4 model, glm::vec3 color, unsigned int texID, int texMode);
void drawFractalTreeRec(unsigned int& cubeVAO, Sphere& sphere, Shader& lightingShader, glm::mat4 transform, int depth, int maxDepth, float length, float thickness);
void drawFirstPersonArrow(unsigned int& cubeVAO, Shader& lightingShader);
void drawFirstPersonGun(unsigned int& cubeVAO, Shader& lightingShader);
void drawScoreboardHUD(unsigned int& cubeVAO, Shader& hudShader, int score, bool gameOverFlag);
void drawScene(Shader& lightingShader, Shader& ourShader, Sphere& sphere, Cone& cone, unsigned int& cubeVAO, glm::mat4 view, glm::mat4 projection);

unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 720;

float rotateAngle_X = 0.0f;
float rotateAngle_Y = 0.0f;
float rotateAngle_Z = 0.0f;
float rotateAxis_X = 0.0f;
float rotateAxis_Y = 0.0f;
float rotateAxis_Z = 1.0f;
float translate_X = 0.0f;
float translate_Y = 0.0f;
float translate_Z = 0.0f;
float scale_X = 1.0f;
float scale_Y = 1.0f;
float scale_Z = 1.0f;

struct Balloon {
    glm::vec3 position;
    glm::vec3 color;
    float scale;
    float speed;
    bool active;
    float floatOffset;
    
    // Popping animation state
    bool isPopping;
    float popTimer;
    std::vector<glm::vec3> fragments;
    std::vector<glm::vec3> fragVels;
    
    Balloon(glm::vec3 pos, float spd, glm::vec3 col = glm::vec3(1.0f, 0.2f, 0.2f), float s = 1.0f)
        : position(pos), color(col), scale(s), speed(spd), active(true), floatOffset(0.0f), isPopping(false), popTimer(0.0f) {}
};
std::vector<Balloon> balloons;
float balloonSpeed = 0.8f;

// Game State
bool gameOver = false;
bool balloonsFrozen = false;
int score = 0;
float balloonSpawnTimer = 0.0f;
float balloonSpawnInterval = 2.0f;
float groundY = -1.5f;


float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorVisible = false;

float eyeX = 0.0, eyeY = 10.0, eyeZ = 0.0; // Top of the tower (Tower Height = 8, Base = -1.5 -> Top = 6.5. Eye at 10 gives good view down)
float lookAtX = 0.0, lookAtY = 0.0, lookAtZ = -10.0;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);


// Lights
glm::vec3 pointLightPosition = glm::vec3(20.0f, 50.0f, -20.0f);
DirectionalLight dirLight(-0.2f, -1.0f, -0.3f, 0.5f, 0.5f, 0.5f, 0.8f, 0.8f, 0.8f, 1.0f, 1.0f, 1.0f);
SpotLight spotLight(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.8f, 0.8f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 0.09f, 0.032f, 12.5f, 15.0f);
std::vector<PointLight> pointLights;
bool isNight = false;

// Light settings
static void setDayMode() {
    isNight = false;
    // Sun: Bright Directional
    dirLight.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
    dirLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    dirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // Tower & Tree: Off
    for (int i = 0; i < 2; i++) {
        pointLights[i].ambient = glm::vec3(0.0f);
        pointLights[i].diffuse = glm::vec3(0.0f);
        pointLights[i].specular = glm::vec3(0.0f);
    }
}

static void setNightMode() {
    isNight = true;
    // Sun: Dim Blue Directional (Moon)
    dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.1f);
    dirLight.diffuse = glm::vec3(0.1f, 0.1f, 0.1f);
    dirLight.specular = glm::vec3(0.1f, 0.1f, 0.1f);

    // Tower & Tree: On (Warm)
    for (int i = 0; i < 2; i++) {
        pointLights[i].ambient = glm::vec3(0.3f, 0.2f, 0.1f);
        pointLights[i].diffuse = glm::vec3(0.9f, 0.7f, 0.4f);
        pointLights[i].specular = glm::vec3(1.0f, 1.0f, 1.0f);
    }
}

bool dirLightOn = true;
bool pointLightOn = true;
bool spotLightOn = true;
bool pointLight0On = true;
bool pointLight1On = true;

bool ambientToggle = true;
bool diffuseToggle = true;
bool specularToggle = true;
bool splitView = false;
float deltaTime = 0.0f;    
float lastFrame = 0.0f;
bool vertexShaderMode = false;
bool proceduralBalloon = false;

enum class FireMode { Bullet, Arrow };
FireMode fireMode = FireMode::Arrow;

const int maxBulletAmmo = 15;
const int maxArrowAmmo = 5;
int bulletAmmo = maxBulletAmmo;
int arrowAmmo = maxArrowAmmo;

bool bulletReloading = false;
float bulletReloadTimer = 0.0f;
const float bulletReloadTime = 2.0f;

bool arrowReloading = false;
float arrowReloadTimer = 0.0f;
const float arrowReloadTime = 3.0f;

// First-person arrow animation
float fpArrowBobTime = 0.0f;
bool arrowJustFired = false;
float arrowRecoilTimer = 0.0f;
const float arrowRecoilDuration = 0.3f;
bool gunJustFired = false;
float gunRecoilTimer = 0.0f;
const float gunRecoilDuration = 0.18f;

const float bulletSpeed = 38.0f;
const float arrowBaseSpeed = 30.0f;
const float arrowGravity = 6.0f;
glm::vec2 windForce = glm::vec2(1.2f, -0.5f);

static glm::vec2 getEffectiveWindForce()
{
    return isRaining ? windForce * 1.6f : windForce;
}

bool scenarioTourActive = false;
float scenarioTourTimer = 0.0f;
bool scenarioTourInitialized = false;

// Animated cars on road
struct Car {
    float z;          // current Z position on road
    float speed;      // units/sec (positive = moving toward camera, i.e. z increases)
    glm::vec3 color;
    float lane;       // X offset from road center
    Car(float z_, float spd, glm::vec3 col, float lane_)
        : z(z_), speed(spd), color(col), lane(lane_) {}
};
std::vector<Car> cars;

float modeToastTimer = 0.0f;
const float modeToastDuration = 2.0f;
std::string modeToastText;
float cameraSpeed = 2.0f; // Adjustable via +/- keys

void showToast(const std::string& text) {
    modeToastText = text;
    modeToastTimer = modeToastDuration;
}

static bool isPointLightEnabled(int index)
{
    return pointLightOn && ((index == 0) ? pointLight0On : pointLight1On);
}

static float easeInOut(float t)
{
    t = glm::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

int main()
{
    srand(static_cast<unsigned int>(time(0)));
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CSE 4208: Computer Graphics Laboratory", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

   
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Print Controls
    std::cout << "========= CONTROLS =========" << std::endl;
    std::cout << "WASD: Move Camera (Fly Mode)" << std::endl;
    std::cout << "Q/E: Move Up/Down" << std::endl;
    std::cout << "Mouse: Look Around" << std::endl;
    std::cout << "M: Toggle Mouse/Camera Control" << std::endl;
    std::cout << "Ctrl+M: Switch Bullet/Arrow Mode" << std::endl;
    std::cout << "Space/Right-Click: Shoot" << std::endl;
    std::cout << "N: Toggle Day/Night Mode" << std::endl;
    std::cout << "1: Toggle Directional Light (Sun/Moon)" << std::endl;
    std::cout << "2: Toggle Point Light 0 (Tower Lamp)" << std::endl;
    std::cout << "K: Toggle Point Light 1 (Tree Lamp)" << std::endl;
    std::cout << "8: Toggle Phong / Gouraud Shading" << std::endl;
    std::cout << "9: Toggle Textures" << std::endl;
    std::cout << "3: Toggle Spot Light (Flashlight)" << std::endl;
    std::cout << "4: Freeze/Unfreeze Balloons" << std::endl;
    std::cout << "0: Toggle Auto Scenario Tour" << std::endl;
    std::cout << "5: Toggle Ambient Light" << std::endl;
    std::cout << "6: Toggle Diffuse Light" << std::endl;
    std::cout << "7: Toggle Specular Light" << std::endl;
    std::cout << "F2: Toggle Rain (weather + physics)" << std::endl;
    std::cout << "V: Toggle Split Viewport (4 Parts)" << std::endl;
    std::cout << "R: Restart Game (after Game Over)" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << ">> Shoot balloons before they hit the ground!" << std::endl;

  
    glEnable(GL_DEPTH_TEST);

   
    Shader phongShader("vertexShaderForPhongShading.vs", "fragmentShaderForPhongShading.fs");
    Shader gouraudShader("vertexShaderForGouraudShading.vs", "fragmentShaderForGouraudShading.fs");
    Shader ourShader("vertexShader.vs", "fragmentShader.fs");
    Shader colorShader("colorShader.vs", "colorShader.fs");

   

    float cube_vertices[] = {
        // positions          // normals           // tex coords
        // Back face (z = -0.5)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        // Right face (x = 0.5)
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,

        // Front face (z = 0.5)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

        // Left face (x = -0.5)
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

        // Top face (y = 0.5)
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,

        // Bottom face (y = -0.5)
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f
    };
    unsigned int cube_indices[] = {
        0, 3, 2,
        2, 1, 0,

        4, 5, 7,
        7, 6, 4,

        8, 9, 10,
        10, 11, 8,

        12, 13, 14,
        14, 15, 12,

        16, 17, 18,
        18, 19, 16,

        20, 21, 22,
        22, 23, 20
    };

    unsigned int cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    Sphere sphere = Sphere();
    Cone cone = Cone();

    // Initialize Lights
    // 0: Tower Street Lamp (lamp head at ~(5.5, 5.0, 0.5))
    pointLights.push_back(PointLight(5.5f, 5.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.22f, 0.20f, 1));
    // 2: Tree Light (Range ~20)
    pointLights.push_back(PointLight(15.0f, 2.0f, -15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.22f, 0.20f, 2));

    // Load textures
    texAsphalt = loadTexture("image/Asphalt012_1K-JPG_Color.jpg");
    texBark = loadTexture("image/Bark001_1K-JPG_Color.jpg");
    texBricks = loadTexture("image/Bricks066_1K-JPG_Color.jpg");
    texGrass = loadTexture("image/Grass004_1K-JPG_Color.jpg");
    texLeaf = loadTexture("image/LeafSet004_1K-JPG_Color.jpg");

    texRock = loadTexture("image/rock.jpg");
    texConcrete = loadTexture("image/concrete.jpg");
    texSail = loadTexture("image/sail.jpg");

    setDayMode();

    // Initialize Balloons
    balloons.emplace_back(glm::vec3(5.0f, 15.0f, -5.0f), 0.8f);
    balloons.emplace_back(glm::vec3(-5.0f, 18.0f, -5.0f), 0.6f);

    // Initialize Rain Drops
    raindrops.reserve(500);
    for (int i = 0; i < 500; i++) {
        glm::vec3 pos(
            (float)(rand() % 150 - 75),
            (float)(rand() % 25 + 5),
            (float)(rand() % 150 - 75)
        );
        raindrops.emplace_back(pos, 8.0f + (float)(rand() % 5));
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Initialize cars (road runs along Z from ~0 to ~-130, X ~= ±1.5 lanes)
    cars.emplace_back(-25.0f, 4.5f,  glm::vec3(0.85f, 0.12f, 0.12f), -1.5f); // Red, left lane, moving toward camera
    cars.emplace_back(-70.0f, -3.5f, glm::vec3(0.15f, 0.35f, 0.85f),  1.5f); // Blue, right lane, moving away




    //ourShader.use();
    //lightingShader.use();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (isRaining) {
            for (auto& drop : raindrops) {
                if (!drop.active) continue;
                drop.position.y -= drop.speed * deltaTime;
                if (drop.position.y < groundY) {
                    drop.position.y = 25.0f + (float)(rand() % 8);
                    drop.position.x = (float)(rand() % 150 - 75);
                    drop.position.z = (float)(rand() % 150 - 75);
                }
            }

            if (thunderFlashTimer > 0.0f) {
                thunderFlashTimer -= deltaTime;
                if (thunderFlashTimer < 0.0f) thunderFlashTimer = 0.0f;
            }
            else if ((rand() % 5000) == 0) {
                thunderFlashTimer = thunderFlashDuration;
            }
        }

        if (!gameOver) {
            // Spawn new balloons (only if not frozen)
            if (!balloonsFrozen) {
                balloonSpawnTimer += deltaTime;
                if (balloonSpawnTimer >= balloonSpawnInterval) {
                    float randX = (float)(rand() % 40 - 20);  // -20 to 20
                    float randZ = (float)(rand() % 40 - 20);  // -20 to 20
                    float randSpeed = 0.5f + (float)(rand() % 100) / 200.0f; // 0.5 to 1.0
                    balloons.emplace_back(glm::vec3(randX, 20.0f, randZ), randSpeed);
                    balloonSpawnTimer = 0.0f;
                }
            }

            // Update balloon positions (fall down) — frozen = no movement
            for (auto& b : balloons) {
                if (b.active) {
                    if (b.isPopping) {
                        b.popTimer += deltaTime;
                        if (b.popTimer >= 0.15f) {
                            b.active = false;
                        }
                    } else if (!balloonsFrozen) {
                        float rainWeightMul = isRaining ? 1.35f : 1.0f;
                        b.position.y -= b.speed * rainWeightMul * deltaTime;

                        if (isRaining) {
                            glm::vec2 rainWind = getEffectiveWindForce();
                            b.position.x += rainWind.x * 0.08f * deltaTime;
                            b.position.z += rainWind.y * 0.08f * deltaTime;
                        }
                    }
                }
            }

            // Game over check - balloon hit ground (skip if frozen)
            if (!balloonsFrozen) for (auto& b : balloons) {
                if (b.active && !b.isPopping && b.position.y <= groundY) {
                    gameOver = true;
                    std::cout << "======= GAME OVER! =======" << std::endl;
                    std::cout << "A balloon hit the ground!" << std::endl;
                    std::cout << "Final Score: " << score << std::endl;
                    std::cout << "Press R to restart." << std::endl;
                    std::cout << "===========================" << std::endl;
                    break;
                }
            }

            updateProjectileSystem(deltaTime);
        }

        updateAmmoReloadSystem(deltaTime);

        // input
        // -----
        // Update cars
        for (auto& car : cars) {
            car.z += car.speed * deltaTime;
            if (car.z > 10.0f)  car.z = -120.0f; // wrap: shot out the back, reappear far
            if (car.z < -125.0f) car.z = 8.0f;   // wrap: going away
        }

        processInput(window);
        updateScenarioTour(deltaTime);

        // render
        // ------
        // render
        // ------
        if (isRaining) {
            glClearColor(0.40f, 0.40f, 0.45f, 1.0f);
        } else if (isNight) {
            glClearColor(0.05f, 0.05f, 0.1f, 1.0f); // Dark Night
        } else {
            glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Sky Blue
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Shader& lightingShader = vertexShaderMode ? gouraudShader : phongShader;
        lightingShader.use();
        lightingShader.setVec3("viewPos", basic_camera.eye);

        // Set up fog
        lightingShader.setBool("fogEnabled", true);
        if (isRaining) {
            lightingShader.setVec3("fogColor", glm::vec3(0.42f, 0.42f, 0.45f));
            lightingShader.setFloat("fogDensity", 0.025f);
        } else if (isNight) {
            lightingShader.setVec3("fogColor", glm::vec3(0.05f, 0.05f, 0.1f));
            lightingShader.setFloat("fogDensity", 0.018f);
        } else {
            lightingShader.setVec3("fogColor", glm::vec3(0.65f, 0.78f, 0.90f));
            lightingShader.setFloat("fogDensity", 0.008f);
        }

        if (isRaining) {
            if (thunderFlashTimer > 0.0f) {
                dirLight.ambient = glm::vec3(1.0f);
                dirLight.diffuse = glm::vec3(1.0f);
                dirLight.specular = glm::vec3(1.0f);
            }
            else {
                dirLight.ambient = glm::vec3(0.30f);
                dirLight.diffuse = glm::vec3(0.40f);
                dirLight.specular = glm::vec3(0.45f);
            }
        }
        else if (isNight) {
            dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.1f);
            dirLight.diffuse = glm::vec3(0.1f, 0.1f, 0.1f);
            dirLight.specular = glm::vec3(0.1f, 0.1f, 0.1f);
        }
        else {
            dirLight.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
            dirLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
            dirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
        }

        // Directional Light (Sun/Moon/Cloudy)
        dirLight.setUpDirectionalLight(lightingShader);

        // Update Point Lights
        for (int i = 0; i < pointLights.size(); i++) {
            if (isPointLightEnabled(i)) {
                pointLights[i].turnOn();
            } else {
                pointLights[i].turnOff();
            }
            pointLights[i].setUpPointLight(lightingShader, i);
        }
        
        // Spot Light (Flashlight)
        // Spot Light Position Update
        spotLight.position = basic_camera.eye;
        spotLight.direction = basic_camera.Front;

        // activate shader
        lightingShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(basic_camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        lightingShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = basic_camera.createViewMatrix();
        lightingShader.setMat4("view", view);

        if (splitView) {
            // 1. Top-Left: Combined (All Components On)
            glViewport(0, SCR_HEIGHT / 2, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            
            // Force All Components On for this view
            dirLight.turnOn(); spotLight.turnOn();
            for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].turnOn();
            
            // Send to Shader
            dirLight.setUpDirectionalLight(lightingShader);
            spotLight.setUpSpotLight(lightingShader);
            for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].setUpPointLight(lightingShader, i);
            
            drawScene(lightingShader, ourShader, sphere, cone, cubeVAO, view, projection);


            // 2. Top-Right: Ambient Only
            glViewport(SCR_WIDTH / 2, SCR_HEIGHT / 2, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            
            // Force Ambient On, Others Off
            dirLight.turnAmbientOn(); dirLight.turnDiffuseOff(); dirLight.turnSpecularOff();
            spotLight.turnAmbientOn(); spotLight.turnDiffuseOff(); spotLight.turnSpecularOff();
            for(unsigned int i=0; i<pointLights.size(); i++) { pointLights[i].turnAmbientOn(); pointLights[i].turnDiffuseOff(); pointLights[i].turnSpecularOff(); }
            
             // Send to Shader
            dirLight.setUpDirectionalLight(lightingShader);
            spotLight.setUpSpotLight(lightingShader);
            for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].setUpPointLight(lightingShader, i);
            
            drawScene(lightingShader, ourShader, sphere, cone, cubeVAO, view, projection);


            // 3. Bottom-Left: Diffuse Only
            glViewport(0, 0, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            
            // Force Diffuse On, Others Off
            dirLight.turnAmbientOff(); dirLight.turnDiffuseOn(); dirLight.turnSpecularOff();
            spotLight.turnAmbientOff(); spotLight.turnDiffuseOn(); spotLight.turnSpecularOff();
            for(unsigned int i=0; i<pointLights.size(); i++) { pointLights[i].turnAmbientOff(); pointLights[i].turnDiffuseOn(); pointLights[i].turnSpecularOff(); }
            
             // Send to Shader
            dirLight.setUpDirectionalLight(lightingShader);
            spotLight.setUpSpotLight(lightingShader);
            for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].setUpPointLight(lightingShader, i);

            drawScene(lightingShader, ourShader, sphere, cone, cubeVAO, view, projection);


            // 4. Bottom-Right: Specular Only
            glViewport(SCR_WIDTH / 2, 0, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            
            // Force Specular On, Others Off
            dirLight.turnAmbientOff(); dirLight.turnDiffuseOff(); dirLight.turnSpecularOn();
            spotLight.turnAmbientOff(); spotLight.turnDiffuseOff(); spotLight.turnSpecularOn();
            for(unsigned int i=0; i<pointLights.size(); i++) { pointLights[i].turnAmbientOff(); pointLights[i].turnDiffuseOff(); pointLights[i].turnSpecularOn(); }
            
             // Send to Shader
            dirLight.setUpDirectionalLight(lightingShader);
            spotLight.setUpSpotLight(lightingShader);
            for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].setUpPointLight(lightingShader, i);

            drawScene(lightingShader, ourShader, sphere, cone, cubeVAO, view, projection);

            // Restore Viewport
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            
            // Restore Light State based on Toggles (Best Effort)
             if(ambientToggle) { 
                dirLight.turnAmbientOn(); spotLight.turnAmbientOn(); for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].turnAmbientOn(); 
            } else {
                dirLight.turnAmbientOff(); spotLight.turnAmbientOff(); for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].turnAmbientOff(); 
            }
             if(diffuseToggle) { 
                dirLight.turnDiffuseOn(); spotLight.turnDiffuseOn(); for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].turnDiffuseOn(); 
            } else {
                dirLight.turnDiffuseOff(); spotLight.turnDiffuseOff(); for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].turnDiffuseOff(); 
            }
             if(specularToggle) { 
                dirLight.turnSpecularOn(); spotLight.turnSpecularOn(); for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].turnSpecularOn(); 
            } else {
                dirLight.turnSpecularOff(); spotLight.turnSpecularOff(); for(unsigned int i=0; i<pointLights.size(); i++) pointLights[i].turnSpecularOff(); 
            }

        } else {
            // Normal Render (Single Viewport)
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

            // Send current light state (handled by toggles/logic elsewhere, but ensure it's sent)
            // Note: In original code, we had logic to turnOff() if bool is false.
            
            // Directional
            if (dirLightOn) dirLight.setUpDirectionalLight(lightingShader);
            else { dirLight.turnOff(); dirLight.setUpDirectionalLight(lightingShader); } // Send off state

            // Points
            if (pointLightOn) {
                for (int i = 0; i < pointLights.size(); i++) pointLights[i].setUpPointLight(lightingShader, i);
            } else {
                 for (int i = 0; i < pointLights.size(); i++) {
                    pointLights[i].turnOff();
                    pointLights[i].setUpPointLight(lightingShader, i);
                }
            }
            
            // Spot
            if (spotLightOn) spotLight.setUpSpotLight(lightingShader);
            else { spotLight.turnOff(); spotLight.setUpSpotLight(lightingShader); }

            drawScene(lightingShader, ourShader, sphere, cone, cubeVAO, view, projection);
        }

        // Draw HUD overlay (Scoreboard) at the top-right
        drawScoreboardHUD(cubeVAO, ourShader, score, gameOver);
        drawCombatHUD(cubeVAO, ourShader);

        /*
        drawCube(cubeVAO, lightingShader, glm::vec3(0.7, 0.6, 0.2));

        glm::mat4 modelForSphere = glm::mat4(1.0f);
        modelForSphere = glm::translate(model, glm::vec3(1.75f, 0.0f, -1.5f));
        sphere.drawSphere(lightingShader, modelForSphere);

        // also draw the lamp object(s)
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightCubeVAO);

        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPosition);
        model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        ourShader.setMat4("model", model);
        ourShader.setVec3("color", glm::vec3(0.8f, 0.8f, 0.8f));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        */


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void drawCube(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 color, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, unsigned int texID, int texMode, float texScale)
{
    lightingShader.use();
    // building model matrix
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, model;
    glm::mat4 identity = glm::mat4(1.0f);
    translateMatrix = glm::translate(identity, pos);
    rotateXMatrix = rotate_manual(translateMatrix, glm::radians(rot[0]), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = rotate_manual(rotateXMatrix, glm::radians(rot[1]), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = rotate_manual(rotateYMatrix, glm::radians(rot[2]), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, scale);

    // setting up materialistic property
    lightingShader.setVec3("material.ambient", color * 0.01f);
    lightingShader.setVec3("material.diffuse", color);
    lightingShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    lightingShader.setFloat("material.shininess", 32.0f);

    lightingShader.setMat4("model", model);
    lightingShader.setFloat("texScale", texScale);

    if (texID > 0 && texturesEnabled) {
        lightingShader.setInt("isTextured", 1);
        lightingShader.setInt("textureMode", texMode);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);
        lightingShader.setInt("diffuseMap", 0);
    } else {
        lightingShader.setInt("isTextured", 0);
    }

    glBindVertexArray(cubeVAO);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

// ===================================================================
// drawCar – cute boxy vehicle, centered at (pos.x, groundY, pos.z)
// ===================================================================
void drawCar(unsigned int& cubeVAO, Shader& ls, glm::vec3 pos, glm::vec3 bodyColor, float wheelAngle)
{
    float gY = -1.5f; // ground level
    glm::vec3 p = glm::vec3(pos.x, gY, pos.z);
    ls.setInt("isTextured", 0);

    // Body
    drawCube(cubeVAO, ls, bodyColor,
        p + glm::vec3(0.0f, 0.45f, 0.0f), glm::vec3(0),
        glm::vec3(1.2f, 0.65f, 2.4f));

    // Roof / cabin (shorter and slightly narrower, shifted slightly forward)
    glm::vec3 roofColor = bodyColor * 0.82f;
    drawCube(cubeVAO, ls, roofColor,
        p + glm::vec3(0.0f, 1.0f, -0.15f), glm::vec3(0),
        glm::vec3(0.95f, 0.52f, 1.35f));

    // Windshield front (slightly darker)
    drawCube(cubeVAO, ls, glm::vec3(0.5f, 0.75f, 0.95f),
        p + glm::vec3(0.0f, 0.95f, 0.62f),
        glm::vec3(10.0f, 0.0f, 0.0f),
        glm::vec3(0.88f, 0.42f, 0.06f));

    // Rear window
    drawCube(cubeVAO, ls, glm::vec3(0.4f, 0.65f, 0.85f),
        p + glm::vec3(0.0f, 0.95f, -0.92f),
        glm::vec3(-10.0f, 0.0f, 0.0f),
        glm::vec3(0.88f, 0.35f, 0.06f));

    // Headlights (front, small bright yellow boxes)
    glm::vec3 hLight = isNight ? glm::vec3(1.0f, 0.98f, 0.7f) : glm::vec3(0.95f, 0.9f, 0.6f);
    drawCube(cubeVAO, ls, hLight, p + glm::vec3(-0.45f, 0.45f, 1.22f), glm::vec3(0), glm::vec3(0.18f, 0.12f, 0.06f));
    drawCube(cubeVAO, ls, hLight, p + glm::vec3( 0.45f, 0.45f, 1.22f), glm::vec3(0), glm::vec3(0.18f, 0.12f, 0.06f));
    // Tail lights (red)
    drawCube(cubeVAO, ls, glm::vec3(0.9f, 0.1f, 0.1f), p + glm::vec3(-0.45f, 0.45f, -1.22f), glm::vec3(0), glm::vec3(0.18f, 0.10f, 0.06f));
    drawCube(cubeVAO, ls, glm::vec3(0.9f, 0.1f, 0.1f), p + glm::vec3( 0.45f, 0.45f, -1.22f), glm::vec3(0), glm::vec3(0.18f, 0.10f, 0.06f));

    // 4 Wheels (dark grey cylinders approximated as flat cubes)
    glm::vec3 wheelColor  = glm::vec3(0.15f, 0.15f, 0.15f);
    glm::vec3 hubColor    = glm::vec3(0.70f, 0.70f, 0.72f);
    // Wheel positions: front-left, front-right, rear-left, rear-right
    std::array<glm::vec3, 4> wPos = {
        p + glm::vec3(-0.65f, 0.18f, 0.85f),
        p + glm::vec3( 0.65f, 0.18f, 0.85f),
        p + glm::vec3(-0.65f, 0.18f, -0.85f),
        p + glm::vec3( 0.65f, 0.18f, -0.85f)
    };
    for (auto& wp : wPos) {
        // Tyre
        drawCube(cubeVAO, ls, wheelColor, wp,
            glm::vec3(wheelAngle, 0.0f, 0.0f),
            glm::vec3(0.35f, 0.35f, 0.22f));
        // Hubcap
        drawCube(cubeVAO, ls, hubColor, wp + glm::vec3(0.0f, 0.0f, 0.12f),
            glm::vec3(wheelAngle, 0.0f, 0.0f),
            glm::vec3(0.20f, 0.20f, 0.04f));
    }
}

// ===================================================================
void drawStreetLamp(unsigned int& cubeVAO, Shader& lightingShader, Sphere& sphere,
                    bool lightOn, float pulse, float nightBoost)
{
    glm::vec3 base = glm::vec3(5.0f, -1.5f, 0.5f); // base of pole on ground
    glm::vec3 metalColor   = glm::vec3(0.30f, 0.30f, 0.32f); // dark iron
    glm::vec3 metalSpec    = glm::vec3(0.6f, 0.6f, 0.6f);

    auto setMetal = [&](Shader& s) {
        s.setVec3("material.ambient",  metalColor * 0.3f);
        s.setVec3("material.diffuse",  metalColor);
        s.setVec3("material.specular", metalSpec);
        s.setFloat("material.shininess", 64.0f);
        s.setInt("isTextured", 0);
    };

    // 1. Base plate (wide flat square on ground)
    setMetal(lightingShader);
    drawCube(cubeVAO, lightingShader, metalColor,
        base + glm::vec3(0.0f, 0.05f, 0.0f), glm::vec3(0), glm::vec3(0.5f, 0.10f, 0.5f));

    // 2. Vertical pole (tall thin rod)
    drawCube(cubeVAO, lightingShader, metalColor,
        base + glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0), glm::vec3(0.12f, 6.0f, 0.12f));

    // 3. Neck / curved arm — 3 segments that step outward and curve
    //    Segment A: diagonal upward-outward
    drawCube(cubeVAO, lightingShader, metalColor,
        base + glm::vec3(0.22f, 6.1f, 0.0f),
        glm::vec3(0.0f, 0.0f, -30.0f),
        glm::vec3(0.10f, 0.70f, 0.10f));

    //    Segment B: near-horizontal outward reach
    drawCube(cubeVAO, lightingShader, metalColor,
        base + glm::vec3(0.6f, 6.55f, 0.0f),
        glm::vec3(0.0f, 0.0f, -10.0f),
        glm::vec3(0.10f, 0.55f, 0.10f));

    //    Segment C: small downward hook at tip
    drawCube(cubeVAO, lightingShader, metalColor,
        base + glm::vec3(1.0f, 6.3f, 0.0f),
        glm::vec3(0.0f, 0.0f, 20.0f),
        glm::vec3(0.10f, 0.30f, 0.10f));

    // 4. Lantern housing (rectangular box, dark frame)
    //    Lamp head center is at base + (1.0, 6.0, 0.0) = world (6.0, 4.5, 0.5)
    glm::vec3 headPos = base + glm::vec3(1.0f, 6.0f, 0.0f);

    //    Outer frame (slightly larger, very dark)
    drawCube(cubeVAO, lightingShader, glm::vec3(0.15f, 0.15f, 0.16f),
        headPos, glm::vec3(0), glm::vec3(0.38f, 0.28f, 0.38f));

    //    Inner glow pane / glass (warm yellow, only if light is on)
    if (lightOn) {
        glm::vec3 glassColor = glm::vec3(1.0f, 0.90f, 0.55f) * pulse * nightBoost;
        lightingShader.setVec3("material.ambient",  glassColor);
        lightingShader.setVec3("material.diffuse",  glassColor * 0.25f);
        lightingShader.setVec3("material.specular", glm::vec3(1.0f));
        lightingShader.setFloat("material.shininess", 128.0f);
        lightingShader.setInt("isTextured", 0);
        glm::mat4 glassM = glm::translate(glm::mat4(1.0f), headPos);
        glassM = glm::scale(glassM, glm::vec3(0.28f, 0.20f, 0.28f));
        lightingShader.setMat4("model", glassM);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Glow halo sphere (additive blend)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glm::vec3 hc = glm::vec3(1.0f, 0.65f, 0.20f) * pulse * nightBoost * 0.35f;
        lightingShader.setVec3("material.ambient", hc);
        lightingShader.setVec3("material.diffuse", hc);
        lightingShader.setVec3("material.specular", glm::vec3(0.0f));
        lightingShader.setFloat("material.shininess", 2.0f);
        glm::mat4 haloM = glm::translate(glm::mat4(1.0f), headPos);
        haloM = glm::scale(haloM, glm::vec3(0.70f));
        lightingShader.setMat4("model", haloM);
        glBindVertexArray(sphere.sphereVAO);
        glDrawElements(GL_TRIANGLES, sphere.getIndexCount(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

void drawScene(Shader& lightingShader, Shader& ourShader, Sphere& sphere, Cone& cone, unsigned int& cubeVAO, glm::mat4 view, glm::mat4 projection) {

        lightingShader.use();
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);

        // Draw Sun (Light Source)
        if (dirLightOn) {
            glm::mat4 modelForSun = glm::mat4(1.0f);
            modelForSun = glm::translate(modelForSun, pointLightPosition);
            modelForSun = glm::scale(modelForSun, glm::vec3(5.0f)); 
            
            ourShader.use();
            ourShader.setMat4("model", modelForSun);
            ourShader.setMat4("view", view);
            ourShader.setMat4("projection", projection);
            if (isNight) {
                ourShader.setVec3("color", glm::vec3(0.9f, 0.9f, 1.0f)); // Moon Color
            } else {
                ourShader.setVec3("color", glm::vec3(1.0f, 1.0f, 0.4f)); // Sun Color
            }
            
            glBindVertexArray(sphere.sphereVAO);
            glDrawElements(GL_TRIANGLES, sphere.getIndexCount(), GL_UNSIGNED_INT, (void*)0);
        }

        // Draw Clouds
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(20.0f, 35.0f, -30.0f));
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(-35.0f, 40.0f, -15.0f));
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(15.0f, 38.0f, 20.0f));
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(-40.0f, 45.0f, 35.0f));
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(45.0f, 36.0f, -5.0f));


        // Draw Ground (R1: GL_REPLACE) - Island sized
        // Island is elongated along the road (-z direction) so all road-side
        // buildings/trees actually sit on land instead of floating in water.
        // x: -75..75, z: -130..50
        glm::mat4 modelForGround = glm::mat4(1.0f);
        modelForGround = glm::translate(modelForGround, glm::vec3(0.0f, -1.5f, -40.0f));
        modelForGround = glm::scale(modelForGround, glm::vec3(150.0f, 1.0f, 180.0f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.2f, 0.8f, 0.2f), glm::vec3(0.0f, -1.5f, -40.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(150.0f, 1.0f, 180.0f), texGrass, 1, 16.0f);

        // Draw Road (extended to span the elongated island)
        drawRoad(cubeVAO, lightingShader, glm::vec3(0.0f, -0.95f, -45.0f));

        // Draw Light Bulbs (always visible; brighter at night)
        {
            float nightBoost = isNight ? 1.0f : 0.35f;
            float pulse = 0.88f + 0.12f * sin((float)glfwGetTime() * 3.0f);
            lightingShader.setInt("isTextured", 0);

            // Tower Bulb (sphere) -- draw VAO directly so material isn't overridden
            if (isPointLightEnabled(0)) {
                glm::vec3 bp = pointLights[0].position;
                glm::vec3 bc = glm::vec3(1.0f, 0.92f, 0.55f) * pulse * nightBoost;

                // Inner bright sphere
                glm::mat4 bm = glm::translate(glm::mat4(1.0f), bp);
                bm = glm::scale(bm, glm::vec3(0.30f));
                lightingShader.setVec3("material.ambient",  bc);
                lightingShader.setVec3("material.diffuse",  bc * 0.3f);
                lightingShader.setVec3("material.specular", glm::vec3(1.0f));
                lightingShader.setFloat("material.shininess", 128.0f);
                lightingShader.setMat4("model", bm);
                glBindVertexArray(sphere.sphereVAO);
                glDrawElements(GL_TRIANGLES, sphere.getIndexCount(), GL_UNSIGNED_INT, (void*)0);

                // Outer halo sphere (additive blend)
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDepthMask(GL_FALSE);
                glm::mat4 hm = glm::translate(glm::mat4(1.0f), bp);
                hm = glm::scale(hm, glm::vec3(0.75f));
                glm::vec3 hc = glm::vec3(1.0f, 0.58f, 0.12f) * pulse * nightBoost * 0.4f;
                lightingShader.setVec3("material.ambient",  hc);
                lightingShader.setVec3("material.diffuse",  hc);
                lightingShader.setVec3("material.specular", glm::vec3(0.0f));
                lightingShader.setFloat("material.shininess", 2.0f);
                lightingShader.setMat4("model", hm);
                glDrawElements(GL_TRIANGLES, sphere.getIndexCount(), GL_UNSIGNED_INT, (void*)0);
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);
                glBindVertexArray(0);
            }
            // Tree Bulb (simple cube)
            if (isPointLightEnabled(1)) {
                float tb = isNight ? 1.0f : 0.4f;
                drawCube(cubeVAO, lightingShader, glm::vec3(1.0f, 1.0f, 0.5f) * tb * pulse, pointLights[1].position, glm::vec3(0.0f), glm::vec3(0.22f));
            }
        }

        // Draw Trees
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(15.0f, -1.5f, -15.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-15.0f, -1.5f, -25.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(25.0f, -1.5f, -40.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-30.0f, -1.5f, -10.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(10.0f, -1.5f, -50.0f));

        // Draw Cone
        glm::mat4 modelForCone = glm::mat4(1.0f);
        modelForCone = glm::translate(modelForCone, glm::vec3(-20.0f, -1.5f, -30.0f));
        modelForCone = glm::scale(modelForCone, glm::vec3(2.0f, 4.0f, 2.0f));
        if (texturesEnabled && texBricks > 0) {
            lightingShader.setInt("isTextured", 1);
            lightingShader.setInt("textureMode", 2);
            lightingShader.setFloat("texScale", 2.0f);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texBricks);
            lightingShader.setInt("diffuseMap", 0);
        } else {
            lightingShader.setInt("isTextured", 0);
        }
        cone.drawCone(lightingShader, modelForCone);
        lightingShader.setInt("isTextured", 0);

        // Draw Tower (Platform)
        drawTower(cubeVAO, lightingShader, glm::vec3(0.0f, -1.5f, 0.0f));

        // Draw Street Lamp (next to tower, has the point light)
        {
            float nightBoost = isNight ? 1.0f : 0.35f;
            float pulse = 0.88f + 0.12f * sin((float)glfwGetTime() * 3.0f);
            drawStreetLamp(cubeVAO, lightingShader, sphere, isPointLightEnabled(0), pulse, nightBoost);
        }

        // ===== MOVING CARS =====
        {
            float t = (float)glfwGetTime();
            float wAngle = fmod(t * 120.0f, 360.0f); // wheel spin
            for (const auto& car : cars) {
                drawCar(cubeVAO, lightingShader,
                    glm::vec3(car.lane, 0.0f, car.z),
                    car.color, wAngle);
            }
        }


        // Draw Projectiles
        for (const auto& proj : projectiles) {
            if (proj.active) {
                if (!proj.isArrow) {
                    // Realistic bullet with rotation
                    glm::mat4 bulletBase = glm::translate(glm::mat4(1.0f), proj.position) * glm::toMat4(proj.rotation);

                    // Bullet body (brass casing)
                    glm::mat4 casingM = glm::scale(bulletBase, glm::vec3(0.025f, 0.025f, 0.12f));
                    lightingShader.setMat4("model", casingM);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.19f, 0.12f, 0.03f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.78f, 0.57f, 0.11f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.90f, 0.72f, 0.25f));
                    lightingShader.setFloat("material.shininess", 96.0f);
                    lightingShader.setInt("isTextured", 0);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // Bullet tip (copper/lead � darker, tapered)
                    glm::mat4 tipM = glm::translate(bulletBase, glm::vec3(0.0f, 0.0f, 0.08f));
                    tipM = glm::scale(tipM, glm::vec3(0.020f, 0.020f, 0.06f));
                    lightingShader.setMat4("model", tipM);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.15f, 0.08f, 0.03f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.65f, 0.40f, 0.15f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.70f, 0.55f, 0.20f));
                    lightingShader.setFloat("material.shininess", 80.0f);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // Primer (flat dark circle at back)
                    glm::mat4 primerM = glm::translate(bulletBase, glm::vec3(0.0f, 0.0f, -0.065f));
                    primerM = glm::scale(primerM, glm::vec3(0.018f, 0.018f, 0.005f));
                    lightingShader.setMat4("model", primerM);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.05f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.15f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.3f));
                    lightingShader.setFloat("material.shininess", 32.0f);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                } else {
                    glm::mat4 base = glm::translate(glm::mat4(1.0f), proj.position) * glm::toMat4(proj.rotation);

                    // ======= CHARMING ARROW REDESIGN =======

                    // 1) Arrow shaft — warm honey-amber wooden rod with slight taper
                    glm::mat4 shaftModel = glm::scale(base, glm::vec3(0.032f, 0.032f, 0.95f));
                    lightingShader.setMat4("model", shaftModel);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.20f, 0.14f, 0.05f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.72f, 0.50f, 0.18f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.30f, 0.25f, 0.10f));
                    lightingShader.setFloat("material.shininess", 24.0f);
                    lightingShader.setInt("isTextured", 0);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // Shaft accent stripe (thin golden wrap near center)
                    glm::mat4 stripeModel = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.1f));
                    stripeModel = glm::scale(stripeModel, glm::vec3(0.038f, 0.038f, 0.04f));
                    lightingShader.setMat4("model", stripeModel);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.30f, 0.25f, 0.05f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.90f, 0.75f, 0.20f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.60f, 0.55f, 0.20f));
                    lightingShader.setFloat("material.shininess", 64.0f);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // 2) Arrowhead — sleek elongated bronze/gold diamond blade (horizontal)
                    glm::mat4 tipModel = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.54f));
                    tipModel = glm::scale(tipModel, glm::vec3(0.07f, 0.018f, 0.26f));
                    lightingShader.setMat4("model", tipModel);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.25f, 0.20f, 0.08f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.85f, 0.68f, 0.22f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.80f, 0.70f, 0.30f));
                    lightingShader.setFloat("material.shininess", 96.0f);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // Arrowhead blade 2 (perpendicular cross-blade)
                    glm::mat4 tipModel2 = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.54f));
                    tipModel2 = glm::scale(tipModel2, glm::vec3(0.018f, 0.07f, 0.26f));
                    lightingShader.setMat4("model", tipModel2);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // Arrowhead collar (ring where head meets shaft)
                    glm::mat4 collarModel = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.42f));
                    collarModel = glm::scale(collarModel, glm::vec3(0.048f, 0.048f, 0.03f));
                    lightingShader.setMat4("model", collarModel);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.28f, 0.22f, 0.06f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.95f, 0.78f, 0.15f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.70f, 0.60f, 0.25f));
                    lightingShader.setFloat("material.shininess", 80.0f);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // 3) Fletching — 3 elegant feathers with golden/amber tone
                    // Feather 1 (horizontal — warm amber)
                    glm::mat4 fletch1 = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.40f));
                    fletch1 = glm::scale(fletch1, glm::vec3(0.065f, 0.012f, 0.18f));
                    lightingShader.setMat4("model", fletch1);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.25f, 0.15f, 0.04f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.85f, 0.55f, 0.12f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.20f, 0.15f, 0.05f));
                    lightingShader.setFloat("material.shininess", 8.0f);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // Feather 2 (vertical — rich gold)
                    glm::mat4 fletch2 = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.40f));
                    fletch2 = glm::scale(fletch2, glm::vec3(0.012f, 0.065f, 0.18f));
                    lightingShader.setMat4("model", fletch2);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.28f, 0.22f, 0.05f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.92f, 0.72f, 0.15f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.15f, 0.12f, 0.05f));
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // Feather 3 (diagonal — cream/ivory)
                    glm::mat4 fletch3 = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.40f));
                    fletch3 = glm::rotate(fletch3, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                    fletch3 = glm::scale(fletch3, glm::vec3(0.06f, 0.012f, 0.16f));
                    lightingShader.setMat4("model", fletch3);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.22f, 0.20f, 0.12f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.95f, 0.90f, 0.65f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.10f));
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                    // 4) Nock (tail end cap — small dark ring)
                    glm::mat4 nockModel = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.48f));
                    nockModel = glm::scale(nockModel, glm::vec3(0.04f, 0.04f, 0.025f));
                    lightingShader.setMat4("model", nockModel);
                    lightingShader.setVec3("material.ambient", glm::vec3(0.12f, 0.08f, 0.03f));
                    lightingShader.setVec3("material.diffuse", glm::vec3(0.35f, 0.22f, 0.08f));
                    lightingShader.setVec3("material.specular", glm::vec3(0.40f, 0.35f, 0.15f));
                    lightingShader.setFloat("material.shininess", 48.0f);
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                }
            }
        }
        // Draw Balloons
        for (const auto& balloon : balloons) {
            if (balloon.active) {
                float scaleMult = 1.0f;
                if (balloon.isPopping) {
                    scaleMult = 1.0f + (balloon.popTimer / 0.15f) * 0.5f;
                }
                drawBalloon(cubeVAO, sphere, lightingShader, balloon.position, scaleMult);
            }
        }

        // Draw Game Over indicator (red ground flash)
        if (gameOver) {
            drawCube(cubeVAO, lightingShader, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -0.9f, -40.0f), glm::vec3(0.0f), glm::vec3(150.0f, 0.2f, 180.0f));
        }
        // (Scoreboard is now a fixed HUD overlay drawn after drawScene returns.)

        // ===== ENVIRONMENT: Mountains in background (far away, smaller) =====
        drawMountain(cubeVAO, lightingShader, glm::vec3(-60.0f, -1.5f, -180.0f), glm::vec3(25.0f, 18.0f, 15.0f), glm::vec3(0.45f, 0.50f, 0.42f));
        drawMountain(cubeVAO, lightingShader, glm::vec3(-25.0f, -1.5f, -200.0f), glm::vec3(30.0f, 22.0f, 18.0f), glm::vec3(0.40f, 0.45f, 0.38f));
        drawMountain(cubeVAO, lightingShader, glm::vec3(10.0f, -1.5f, -210.0f), glm::vec3(35.0f, 28.0f, 20.0f), glm::vec3(0.38f, 0.42f, 0.35f));
        drawMountain(cubeVAO, lightingShader, glm::vec3(50.0f, -1.5f, -190.0f), glm::vec3(28.0f, 20.0f, 16.0f), glm::vec3(0.42f, 0.47f, 0.40f));
        drawMountain(cubeVAO, lightingShader, glm::vec3(80.0f, -1.5f, -195.0f), glm::vec3(22.0f, 16.0f, 14.0f), glm::vec3(0.44f, 0.48f, 0.41f));

        // ===== ENVIRONMENT: Buildings along road (all within island x:-75..75, z:-130..50) =====
        // Left side buildings (well clear of road, on grass)
        drawBuilding(cubeVAO, lightingShader, glm::vec3(-15.0f, -1.5f, -20.0f), 4.0f, 6.0f, 5.0f, glm::vec3(0.6f, 0.55f, 0.45f), glm::vec3(0.45f, 0.2f, 0.15f));
        drawBuilding(cubeVAO, lightingShader, glm::vec3(-17.0f, -1.5f, -40.0f), 5.0f, 8.0f, 6.0f, glm::vec3(0.7f, 0.65f, 0.55f), glm::vec3(0.5f, 0.25f, 0.15f));
        drawBuilding(cubeVAO, lightingShader, glm::vec3(-15.0f, -1.5f, -60.0f), 3.5f, 5.0f, 4.0f, glm::vec3(0.55f, 0.5f, 0.4f), glm::vec3(0.4f, 0.18f, 0.12f));
        drawBuilding(cubeVAO, lightingShader, glm::vec3(-18.0f, -1.5f, -78.0f), 4.5f, 7.0f, 5.0f, glm::vec3(0.62f, 0.57f, 0.46f), glm::vec3(0.46f, 0.22f, 0.14f));
        // Right side buildings
        drawBuilding(cubeVAO, lightingShader, glm::vec3(15.0f, -1.5f, -25.0f), 4.5f, 7.0f, 5.5f, glm::vec3(0.65f, 0.6f, 0.5f), glm::vec3(0.48f, 0.22f, 0.14f));
        drawBuilding(cubeVAO, lightingShader, glm::vec3(17.0f, -1.5f, -48.0f), 5.5f, 10.0f, 6.5f, glm::vec3(0.58f, 0.53f, 0.43f), glm::vec3(0.42f, 0.2f, 0.13f));
        drawBuilding(cubeVAO, lightingShader, glm::vec3(15.0f, -1.5f, -68.0f), 4.0f, 5.5f, 5.0f, glm::vec3(0.62f, 0.57f, 0.47f), glm::vec3(0.46f, 0.21f, 0.14f));
        drawBuilding(cubeVAO, lightingShader, glm::vec3(18.0f, -1.5f, -88.0f), 5.0f, 7.5f, 5.5f, glm::vec3(0.6f, 0.55f, 0.45f), glm::vec3(0.44f, 0.2f, 0.13f));

        // ===== ENVIRONMENT: Fence posts along road (extends with longer road) =====
        for (float z = -5.0f; z > -95.0f; z -= 5.0f) {
            drawFencePost(cubeVAO, lightingShader, glm::vec3(7.0f, -1.5f, z));
            drawFencePost(cubeVAO, lightingShader, glm::vec3(-7.0f, -1.5f, z));
        }
        // Fence horizontal rails (connecting posts)
        for (float z = -5.0f; z > -90.0f; z -= 5.0f) {
            drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.3f, 0.2f), glm::vec3(7.0f, -0.3f, z - 2.5f), glm::vec3(0.0f), glm::vec3(0.08f, 0.08f, 5.2f), texBark, 2, 6.0f);
            drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.3f, 0.2f), glm::vec3(-7.0f, -0.3f, z - 2.5f), glm::vec3(0.0f), glm::vec3(0.08f, 0.08f, 5.2f), texBark, 2, 6.0f);
        }

        // ===== More scattered trees (kept inside island bounds) =====
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-40.0f, -1.5f, -30.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(35.0f, -1.5f, -20.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-30.0f, -1.5f, -60.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(40.0f, -1.5f, -55.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-45.0f, -1.5f, -70.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(50.0f, -1.5f, -35.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-25.0f, -1.5f, -90.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(30.0f, -1.5f, -85.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-55.0f, -1.5f, -45.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(55.0f, -1.5f, -75.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-38.0f, -1.5f, -82.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(42.0f, -1.5f, -88.0f));

        // ===== More clouds =====
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(-50.0f, 40.0f, -60.0f));
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(60.0f, 42.0f, -50.0f));
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(10.0f, 48.0f, -90.0f));
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(30.0f, 38.0f, 60.0f));
        drawCloud(cubeVAO, sphere, lightingShader, glm::vec3(-45.0f, 45.0f, 70.0f));

        // ========================================================
        // ===== ISLAND BACKSIDE (+Z direction, behind player) =====
        // ========================================================

        // --- OCEAN / WATER (surrounding the elongated island on all sides) ---
        // Island spans roughly x:-75..75, z:-130..50, so water is pushed outside that.
        // Back water (behind the player, +z)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.1f, 0.35f, 0.6f),
            glm::vec3(0.0f, -2.2f, 130.0f), glm::vec3(0.0f), glm::vec3(500.0f, 0.5f, 160.0f));
        // Front water (in front of the road, far -z, beyond the island)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.1f, 0.35f, 0.6f),
            glm::vec3(0.0f, -2.2f, -220.0f), glm::vec3(0.0f), glm::vec3(500.0f, 0.5f, 180.0f));
        // Left water
        drawCube(cubeVAO, lightingShader, glm::vec3(0.1f, 0.35f, 0.6f),
            glm::vec3(-130.0f, -2.2f, -40.0f), glm::vec3(0.0f), glm::vec3(110.0f, 0.5f, 500.0f));
        // Right water
        drawCube(cubeVAO, lightingShader, glm::vec3(0.1f, 0.35f, 0.6f),
            glm::vec3(130.0f, -2.2f, -40.0f), glm::vec3(0.0f), glm::vec3(110.0f, 0.5f, 500.0f));

        // --- SANDY BEACH BORDER (tan strip at every island edge) ---
        // Back-of-island beach (player side, +z)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.76f, 0.70f, 0.50f),
            glm::vec3(0.0f, -1.45f, 48.0f), glm::vec3(0.0f), glm::vec3(150.0f, 0.15f, 8.0f));
        // Front-of-island beach (far -z edge of island)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.76f, 0.70f, 0.50f),
            glm::vec3(0.0f, -1.45f, -128.0f), glm::vec3(0.0f), glm::vec3(150.0f, 0.15f, 8.0f));
        // Left beach (runs the full length of the island)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.76f, 0.70f, 0.50f),
            glm::vec3(-73.0f, -1.45f, -40.0f), glm::vec3(0.0f), glm::vec3(8.0f, 0.15f, 180.0f));
        // Right beach
        drawCube(cubeVAO, lightingShader, glm::vec3(0.76f, 0.70f, 0.50f),
            glm::vec3(73.0f, -1.45f, -40.0f), glm::vec3(0.0f), glm::vec3(8.0f, 0.15f, 180.0f));

        // --- DOCK / PIER (wooden planks extending into water) ---
        // Main pier platform
        drawCube(cubeVAO, lightingShader, glm::vec3(0.45f, 0.32f, 0.18f),
            glm::vec3(0.0f, -0.8f, 55.0f), glm::vec3(0.0f), glm::vec3(4.0f, 0.3f, 16.0f), texBark, 2);
        // Pier support pillars
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.25f, 0.12f),
            glm::vec3(-1.5f, -1.8f, 50.0f), glm::vec3(0.0f), glm::vec3(0.3f, 2.2f, 0.3f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.25f, 0.12f),
            glm::vec3(1.5f, -1.8f, 50.0f), glm::vec3(0.0f), glm::vec3(0.3f, 2.2f, 0.3f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.25f, 0.12f),
            glm::vec3(-1.5f, -1.8f, 58.0f), glm::vec3(0.0f), glm::vec3(0.3f, 2.2f, 0.3f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.25f, 0.12f),
            glm::vec3(1.5f, -1.8f, 58.0f), glm::vec3(0.0f), glm::vec3(0.3f, 2.2f, 0.3f));
        // Pier railing posts
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.3f, 0.15f),
            glm::vec3(-1.8f, 0.0f, 50.0f), glm::vec3(0.0f), glm::vec3(0.1f, 1.2f, 0.1f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.3f, 0.15f),
            glm::vec3(1.8f, 0.0f, 50.0f), glm::vec3(0.0f), glm::vec3(0.1f, 1.2f, 0.1f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.3f, 0.15f),
            glm::vec3(-1.8f, 0.0f, 62.0f), glm::vec3(0.0f), glm::vec3(0.1f, 1.2f, 0.1f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.3f, 0.15f),
            glm::vec3(1.8f, 0.0f, 62.0f), glm::vec3(0.0f), glm::vec3(0.1f, 1.2f, 0.1f));

        // --- LIGHTHOUSE (tall tower near the water edge) ---
        // Tower body
        drawCube(cubeVAO, lightingShader, glm::vec3(0.9f, 0.9f, 0.85f),
            glm::vec3(35.0f, 5.0f, 40.0f), glm::vec3(0.0f), glm::vec3(2.5f, 14.0f, 2.5f), texConcrete, 2, 2.0f);
        // Red stripe bands
        drawCube(cubeVAO, lightingShader, glm::vec3(0.8f, 0.15f, 0.1f),
            glm::vec3(35.0f, 4.0f, 40.0f), glm::vec3(0.0f), glm::vec3(2.6f, 2.0f, 2.6f), texConcrete, 2, 1.0f);
        drawCube(cubeVAO, lightingShader, glm::vec3(0.8f, 0.15f, 0.1f),
            glm::vec3(35.0f, 8.0f, 40.0f), glm::vec3(0.0f), glm::vec3(2.6f, 2.0f, 2.6f), texConcrete, 2, 1.0f);
        // Lighthouse top platform
        drawCube(cubeVAO, lightingShader, glm::vec3(0.3f, 0.3f, 0.3f),
            glm::vec3(35.0f, 12.5f, 40.0f), glm::vec3(0.0f), glm::vec3(3.5f, 0.4f, 3.5f), texConcrete, 2, 1.0f);
        // Lighthouse lamp (glowing yellow)
        drawCube(cubeVAO, lightingShader, glm::vec3(1.0f, 0.95f, 0.4f),
            glm::vec3(35.0f, 13.5f, 40.0f), glm::vec3(0.0f), glm::vec3(1.5f, 2.0f, 1.5f));
        // Lighthouse roof cap
        drawCube(cubeVAO, lightingShader, glm::vec3(0.2f, 0.2f, 0.2f),
            glm::vec3(35.0f, 14.8f, 40.0f), glm::vec3(0.0f), glm::vec3(2.0f, 0.5f, 2.0f));

        // --- ROCKS / BOULDERS (along shoreline) ---
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.4f, 0.38f),
            glm::vec3(15.0f, -1.0f, 45.0f), glm::vec3(15.0f, 25.0f, 10.0f), glm::vec3(2.0f, 1.5f, 1.8f), texRock, 2);
        drawCube(cubeVAO, lightingShader, glm::vec3(0.45f, 0.42f, 0.4f),
            glm::vec3(-20.0f, -1.0f, 47.0f), glm::vec3(5.0f, 30.0f, 0.0f), glm::vec3(2.5f, 1.2f, 2.0f), texRock, 2);
        drawCube(cubeVAO, lightingShader, glm::vec3(0.38f, 0.37f, 0.35f),
            glm::vec3(25.0f, -0.8f, 43.0f), glm::vec3(20.0f, 10.0f, 5.0f), glm::vec3(1.5f, 1.0f, 1.3f), texRock, 2);
        drawCube(cubeVAO, lightingShader, glm::vec3(0.42f, 0.40f, 0.37f),
            glm::vec3(-35.0f, -0.9f, 44.0f), glm::vec3(8.0f, 0.0f, 12.0f), glm::vec3(3.0f, 1.8f, 2.5f), texRock, 2);
        drawCube(cubeVAO, lightingShader, glm::vec3(0.5f, 0.48f, 0.44f),
            glm::vec3(-10.0f, -1.1f, 49.0f), glm::vec3(0.0f, 15.0f, 0.0f), glm::vec3(1.0f, 0.8f, 1.2f), texRock, 2);
        drawCube(cubeVAO, lightingShader, glm::vec3(0.43f, 0.41f, 0.39f),
            glm::vec3(40.0f, -1.0f, 46.0f), glm::vec3(10.0f, 20.0f, 5.0f), glm::vec3(1.8f, 1.3f, 1.6f), texRock, 2);

        // --- REALISTIC SAILING SHIP (near the dock) ---
        // All parts rotated 15 degrees around Y to match dock angle
        float shipRot = 15.0f;
        glm::vec3 shipPos(5.0f, 0.0f, 60.0f);

        // ====== HULL (layered for realistic curved shape) ======
        // Main hull body (wide bottom)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.28f, 0.16f, 0.06f),
            shipPos + glm::vec3(0.0f, -1.9f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(2.8f, 0.7f, 6.0f), texBark, 2);
        // Hull mid-section (slightly narrower)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.32f, 0.19f, 0.07f),
            shipPos + glm::vec3(0.0f, -1.3f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(2.5f, 0.5f, 5.6f), texBark, 2);
        // Hull upper rim (gunwale)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.22f, 0.09f),
            shipPos + glm::vec3(0.0f, -0.9f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(2.6f, 0.3f, 5.8f));
        // Hull keel (bottom ridge � dark)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.15f, 0.08f, 0.03f),
            shipPos + glm::vec3(0.0f, -2.3f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.4f, 0.15f, 5.5f));

        // Hull waterline stripe (red anti-fouling paint)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.6f, 0.08f, 0.05f),
            shipPos + glm::vec3(0.0f, -2.05f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(2.85f, 0.12f, 6.05f));

        // Bow (pointed front � two angled pieces)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.30f, 0.18f, 0.07f),
            shipPos + glm::vec3(0.0f, -1.5f, 3.3f), glm::vec3(8.0f, shipRot, 0.0f), glm::vec3(1.8f, 0.6f, 1.2f), texBark, 2);
        // Stern (flat back)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.30f, 0.18f, 0.07f),
            shipPos + glm::vec3(0.0f, -1.2f, -3.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(2.4f, 1.0f, 0.3f), texBark, 2);

        // ====== DECK (wooden planks) ======
        drawCube(cubeVAO, lightingShader, glm::vec3(0.52f, 0.38f, 0.22f),
            shipPos + glm::vec3(0.0f, -0.7f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(2.3f, 0.08f, 5.2f), texBark, 2);

        // Deck plank lines (visual detail)
        for (int p = -3; p <= 3; p++) {
            drawCube(cubeVAO, lightingShader, glm::vec3(0.40f, 0.28f, 0.15f),
                shipPos + glm::vec3(p * 0.3f, -0.65f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.04f, 0.02f, 5.0f));
        }

        // ====== CABIN / WHEELHOUSE (on deck, toward stern) ======
        // Cabin walls
        drawCube(cubeVAO, lightingShader, glm::vec3(0.45f, 0.32f, 0.18f),
            shipPos + glm::vec3(0.0f, -0.05f, -1.2f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(1.6f, 1.2f, 1.8f), texBark, 2);
        // Cabin roof
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.22f, 0.10f),
            shipPos + glm::vec3(0.0f, 0.65f, -1.2f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(1.8f, 0.12f, 2.0f));
        // Cabin windows (front � 2 windows)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.65f, 0.85f),
            shipPos + glm::vec3(-0.35f, 0.1f, -0.28f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.35f, 0.35f, 0.04f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.65f, 0.85f),
            shipPos + glm::vec3(0.35f, 0.1f, -0.28f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.35f, 0.35f, 0.04f));
        // Cabin side windows
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.65f, 0.85f),
            shipPos + glm::vec3(0.82f, 0.1f, -1.2f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.04f, 0.30f, 0.45f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.65f, 0.85f),
            shipPos + glm::vec3(-0.82f, 0.1f, -1.2f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.04f, 0.30f, 0.45f));
        // Cabin door
        drawCube(cubeVAO, lightingShader, glm::vec3(0.25f, 0.15f, 0.08f),
            shipPos + glm::vec3(0.0f, -0.15f, -0.28f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.4f, 0.7f, 0.05f));

        // ====== INTERIOR DETAILS (visible through door/windows) ======
        // Floor inside cabin (slightly lower)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.48f, 0.35f, 0.20f),
            shipPos + glm::vec3(0.0f, -0.58f, -1.2f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(1.4f, 0.04f, 1.6f));
        // Steering wheel post
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.25f, 0.12f),
            shipPos + glm::vec3(0.0f, 0.15f, -0.55f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.06f, 0.5f, 0.06f));
        // Steering wheel (horizontal disc)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.40f, 0.28f, 0.12f),
            shipPos + glm::vec3(0.0f, 0.42f, -0.55f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.35f, 0.04f, 0.35f));
        // Steering wheel spokes
        drawCube(cubeVAO, lightingShader, glm::vec3(0.38f, 0.26f, 0.10f),
            shipPos + glm::vec3(0.0f, 0.42f, -0.55f), glm::vec3(0.0f, shipRot, 45.0f), glm::vec3(0.03f, 0.05f, 0.30f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.38f, 0.26f, 0.10f),
            shipPos + glm::vec3(0.0f, 0.42f, -0.55f), glm::vec3(0.0f, shipRot, -45.0f), glm::vec3(0.03f, 0.05f, 0.30f));
        // Navigation table (inside cabin, side)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.42f, 0.30f, 0.16f),
            shipPos + glm::vec3(0.5f, -0.25f, -1.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.5f, 0.04f, 0.6f));
        // Table leg
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.22f, 0.10f),
            shipPos + glm::vec3(0.5f, -0.45f, -1.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.06f, 0.35f, 0.06f));
        // Bench/seat inside
        drawCube(cubeVAO, lightingShader, glm::vec3(0.50f, 0.35f, 0.18f),
            shipPos + glm::vec3(-0.45f, -0.35f, -1.6f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.5f, 0.25f, 0.8f));

        // ====== MAST (tall center mast) ======
        drawCube(cubeVAO, lightingShader, glm::vec3(0.38f, 0.26f, 0.14f),
            shipPos + glm::vec3(0.0f, 1.5f, 0.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.12f, 5.0f, 0.12f), texBark, 2);
        // Crow's nest platform (top of mast)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.40f, 0.28f, 0.14f),
            shipPos + glm::vec3(0.0f, 3.8f, 0.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.6f, 0.06f, 0.6f));
        // Crow's nest railing posts
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.22f, 0.10f),
            shipPos + glm::vec3(0.25f, 4.0f, 0.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.04f, 0.4f, 0.04f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.22f, 0.10f),
            shipPos + glm::vec3(-0.25f, 4.0f, 0.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.04f, 0.4f, 0.04f));

        // Cross beam (yard arm � horizontal)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.36f, 0.24f, 0.12f),
            shipPos + glm::vec3(0.0f, 2.8f, 0.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(3.0f, 0.08f, 0.08f));

        // ====== SAILS ======
        // Main sail (large, on the mast)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.92f, 0.90f, 0.82f),
            shipPos + glm::vec3(0.0f, 1.8f, 0.6f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.05f, 2.5f, 2.2f), texSail, 2, 1.5f);
        // Top sail (smaller, above main)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.94f, 0.92f, 0.85f),
            shipPos + glm::vec3(0.0f, 3.2f, 0.55f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(2.4f, 1.0f, 0.04f), texSail, 2, 1.5f);

        // ====== RAILING / BULWARK (around deck edges) ======
        // Left railing
        drawCube(cubeVAO, lightingShader, glm::vec3(0.38f, 0.25f, 0.12f),
            shipPos + glm::vec3(-1.1f, -0.45f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.06f, 0.45f, 4.8f));
        // Right railing
        drawCube(cubeVAO, lightingShader, glm::vec3(0.38f, 0.25f, 0.12f),
            shipPos + glm::vec3(1.1f, -0.45f, 0.0f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.06f, 0.45f, 4.8f));
        // Railing posts (left side)
        for (float z = -2.0f; z <= 2.0f; z += 1.0f) {
            drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.22f, 0.10f),
                shipPos + glm::vec3(-1.1f, -0.35f, z), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.05f, 0.6f, 0.05f));
            drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.22f, 0.10f),
                shipPos + glm::vec3(1.1f, -0.35f, z), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.05f, 0.6f, 0.05f));
        }

        // ====== DECK DETAILS ======
        // Barrel on deck (near bow)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.40f, 0.25f, 0.12f),
            shipPos + glm::vec3(0.6f, -0.35f, 1.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.3f, 0.45f, 0.3f), texBark, 2);
        // Barrel metal bands
        drawCube(cubeVAO, lightingShader, glm::vec3(0.25f, 0.25f, 0.25f),
            shipPos + glm::vec3(0.6f, -0.20f, 1.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.32f, 0.04f, 0.32f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.25f, 0.25f, 0.25f),
            shipPos + glm::vec3(0.6f, -0.48f, 1.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.32f, 0.04f, 0.32f));

        // Second barrel
        drawCube(cubeVAO, lightingShader, glm::vec3(0.38f, 0.23f, 0.10f),
            shipPos + glm::vec3(0.3f, -0.35f, 1.8f), glm::vec3(0.0f, shipRot + 10.0f, 0.0f), glm::vec3(0.28f, 0.42f, 0.28f), texBark, 2);

        // Coiled rope on deck
        drawCube(cubeVAO, lightingShader, glm::vec3(0.55f, 0.45f, 0.30f),
            shipPos + glm::vec3(-0.7f, -0.60f, 1.2f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.35f, 0.12f, 0.35f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.50f, 0.40f, 0.25f),
            shipPos + glm::vec3(-0.7f, -0.50f, 1.2f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.20f, 0.10f, 0.20f));

        // Anchor (hanging on bow side)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.20f, 0.20f, 0.22f),
            shipPos + glm::vec3(1.2f, -1.2f, 2.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.04f, 0.6f, 0.04f));
        drawCube(cubeVAO, lightingShader, glm::vec3(0.20f, 0.20f, 0.22f),
            shipPos + glm::vec3(1.2f, -1.5f, 2.5f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.25f, 0.04f, 0.04f));

        // ====== FLAG on top of mast ======
        drawCube(cubeVAO, lightingShader, glm::vec3(0.85f, 0.12f, 0.08f),
            shipPos + glm::vec3(0.15f, 4.0f, 0.5f), glm::vec3(0.0f, shipRot + 5.0f, 0.0f), glm::vec3(0.5f, 0.3f, 0.02f));

        // ====== RUDDER (at stern, below waterline) ======
        drawCube(cubeVAO, lightingShader, glm::vec3(0.25f, 0.15f, 0.06f),
            shipPos + glm::vec3(0.0f, -2.0f, -3.2f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.08f, 0.8f, 0.5f));

        // ====== LANTERN (hanging near cabin door) ======
        // Lantern body
        drawCube(cubeVAO, lightingShader, glm::vec3(0.25f, 0.22f, 0.18f),
            shipPos + glm::vec3(0.45f, 0.25f, -0.28f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.08f, 0.12f, 0.08f));
        // Lantern glow (warm yellow)
        drawCube(cubeVAO, lightingShader, glm::vec3(1.0f, 0.85f, 0.3f),
            shipPos + glm::vec3(0.45f, 0.25f, -0.28f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.06f, 0.08f, 0.06f));
        // Lantern hook
        drawCube(cubeVAO, lightingShader, glm::vec3(0.20f, 0.18f, 0.15f),
            shipPos + glm::vec3(0.45f, 0.35f, -0.28f), glm::vec3(0.0f, shipRot, 0.0f), glm::vec3(0.03f, 0.10f, 0.03f));

        // --- TREES along back edges ---
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-30.0f, -1.5f, 35.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-15.0f, -1.5f, 40.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(20.0f, -1.5f, 38.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(42.0f, -1.5f, 30.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(-40.0f, -1.5f, 25.0f));
        drawTree(cubeVAO, sphere, lightingShader, glm::vec3(10.0f, -1.5f, 42.0f));

        // --- SIDE MOUNTAINS (visible from island edges) ---
        drawMountain(cubeVAO, lightingShader, glm::vec3(-90.0f, -1.5f, 50.0f), glm::vec3(20.0f, 12.0f, 15.0f), glm::vec3(0.44f, 0.48f, 0.42f));
        drawMountain(cubeVAO, lightingShader, glm::vec3(90.0f, -1.5f, 40.0f), glm::vec3(18.0f, 14.0f, 12.0f), glm::vec3(0.40f, 0.44f, 0.38f));

        if (isRaining) {
            for (const auto& drop : raindrops) {
                if (!drop.active) continue;
                if (glm::distance(drop.position, basic_camera.eye) > 100.0f) continue;

                drawCube(cubeVAO, lightingShader,
                    glm::vec3(0.6f, 0.7f, 0.9f),
                    drop.position,
                    glm::vec3(14.0f, 0.0f, 0.0f),
                    glm::vec3(0.02f, 0.23f, 0.02f));
            }
        }

        // ===== FIRST PERSON WEAPONS (FPS style) =====
        drawFirstPersonArrow(cubeVAO, lightingShader);
        drawFirstPersonGun(cubeVAO, lightingShader);
}

void drawBalloon(unsigned int& cubeVAO, Sphere& sphere, Shader& lightingShader, glm::vec3 position, float scaleMult)
{
    // Draw Balloon Body
    glm::mat4 modelForBalloon = glm::mat4(1.0f);
    modelForBalloon = glm::translate(modelForBalloon, position);
    modelForBalloon = glm::scale(modelForBalloon, glm::vec3(0.6f * scaleMult, 0.8f * scaleMult, 0.6f * scaleMult)); // Oval shape
    glm::vec3 oldAmb = sphere.ambient;
    glm::vec3 oldDiff = sphere.diffuse;
    glm::vec3 oldSpec = sphere.specular;
    float oldShiny = sphere.shininess;
    sphere.diffuse = glm::vec3(0.0, 0.0, 1.0);
    sphere.specular = glm::vec3(1.0, 0, 0);
    if (balloonColor) {
        sphere.diffuse = glm::vec3(0.0, 0.0, 1.0);
        sphere.specular = glm::vec3(1.0, 0, 0);
    }
    else {
        sphere.diffuse = oldDiff;
		sphere.specular = oldSpec;
    }
    if (proceduralBalloon && texturesEnabled) {
        lightingShader.setInt("isProcedural", 1);
    } else {
        lightingShader.setInt("isProcedural", 0);
        lightingShader.setInt("isTextured", 0);
    }

    if (isRaining) {
        sphere.diffuse *= 0.85f;
        sphere.specular = glm::vec3(0.85f, 0.85f, 0.9f);
        sphere.shininess = 72.0f;
    }

    sphere.drawSphere(lightingShader, modelForBalloon);

    sphere.ambient = oldAmb;
    sphere.diffuse = oldDiff;
    sphere.specular = oldSpec;
    sphere.shininess = oldShiny;

    lightingShader.setInt("isProcedural", 0);
    lightingShader.setInt("isTextured", 0);

    drawCube(cubeVAO, lightingShader, glm::vec3(0.0f, 0.0f, 1.0f), position + glm::vec3(0.0f, -0.8f * scaleMult, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.05f * scaleMult, 0.8f * scaleMult, 0.05f * scaleMult));
}

void drawTower(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position)
{
    // R2: Blended texture Bricks (GL_MODULATE)
    glm::vec3 towerBaseScale(2.0f, 8.0f, 2.0f); // 2x8x2
    glm::vec3 basePos = position + glm::vec3(0.0f, 4.0f, 0.0f); 
    drawCube(cubeVAO, lightingShader, glm::vec3(0.4f, 0.4f, 0.4f), basePos, glm::vec3(0.0f), towerBaseScale, texBricks, 2);
    
    // Platform
    glm::vec3 platformScale(4.0f, 0.2f, 4.0f);
    glm::vec3 platformPos = position + glm::vec3(0.0f, 8.0f, 0.0f); 
    drawCube(cubeVAO, lightingShader, glm::vec3(0.3f, 0.3f, 0.3f), platformPos, glm::vec3(0.0f), platformScale, texBricks, 2);
    
    // Railings (Simple)
    // 4 posts
    drawCube(cubeVAO, lightingShader, glm::vec3(0.5f, 0.5f, 0.5f), platformPos + glm::vec3(1.8f, 0.5f, 1.8f), glm::vec3(0.0f), glm::vec3(0.2f, 1.0f, 0.2f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.5f, 0.5f, 0.5f), platformPos + glm::vec3(-1.8f, 0.5f, 1.8f), glm::vec3(0.0f), glm::vec3(0.2f, 1.0f, 0.2f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.5f, 0.5f, 0.5f), platformPos + glm::vec3(1.8f, 0.5f, -1.8f), glm::vec3(0.0f), glm::vec3(0.2f, 1.0f, 0.2f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.5f, 0.5f, 0.5f), platformPos + glm::vec3(-1.8f, 0.5f, -1.8f), glm::vec3(0.0f), glm::vec3(0.2f, 1.0f, 0.2f), texBricks, 2);
}

void drawCloud(unsigned int& cubeVAO, Sphere& sphere, Shader& lightingShader, glm::vec3 position)
{
    // Backup and set white properties
    glm::vec3 oldAmb = sphere.ambient;
    glm::vec3 oldDiff = sphere.diffuse;
    glm::vec3 oldSpec = sphere.specular;
    float oldShiny = sphere.shininess;

    // Cloud properties: high ambient, low diffuse, no specular, slightly off-white
    glm::vec3 cloudColor = glm::vec3(0.95f, 0.97f, 1.0f);
    glm::vec3 cAmb = cloudColor * 0.8f;
    glm::vec3 cDiff = cloudColor * 0.3f;
    glm::vec3 cSpec = glm::vec3(0.0f);

    sphere.set(sphere.radius, sphere.sectorCount, sphere.stackCount, cAmb, cDiff, cSpec, 1.0f);

    struct Puff { glm::vec3 pos; glm::vec3 scale; };
    Puff puffs[] = {
        { glm::vec3( 0.0f,  0.0f,  0.0f), glm::vec3(3.0f, 2.5f, 2.8f) }, // Center
        { glm::vec3( 1.5f, -0.2f,  0.5f), glm::vec3(2.2f, 1.8f, 2.0f) }, // Right
        { glm::vec3(-1.8f, -0.1f, -0.3f), glm::vec3(2.4f, 1.9f, 2.2f) }, // Left
        { glm::vec3( 0.8f,  0.8f, -0.6f), glm::vec3(2.0f, 1.7f, 2.0f) }, // Top right
        { glm::vec3(-1.0f,  0.6f,  0.4f), glm::vec3(2.1f, 1.6f, 1.9f) }, // Top left
        { glm::vec3( 0.0f, -0.5f,  1.2f), glm::vec3(2.0f, 1.5f, 2.0f) }, // Front bottom
        { glm::vec3( 2.5f, -0.4f, -0.2f), glm::vec3(1.5f, 1.2f, 1.5f) }, // Far right
        { glm::vec3(-2.8f, -0.3f,  0.1f), glm::vec3(1.6f, 1.3f, 1.6f) }  // Far left
    };

    for (int i = 0; i < 8; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position + puffs[i].pos * 2.5f);
        model = glm::scale(model, puffs[i].scale * 2.5f);
        sphere.drawSphere(lightingShader, model);
    }

    // Restore
    sphere.set(sphere.radius, sphere.sectorCount, sphere.stackCount, oldAmb, oldDiff, oldSpec, oldShiny);
}

void drawRoad(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position)
{
    // R1: Asphalt Road (GL_REPLACE) - longer to span elongated island
    drawCube(cubeVAO, lightingShader, glm::vec3(0.15f, 0.15f, 0.15f), position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(8.0f, 0.1f, 90.0f), texAsphalt, 1, 9.0f);
}

void drawTree(unsigned int& cubeVAO, Sphere& sphere, Shader& lightingShader, glm::vec3 position)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
    int maxDepth = 4;
    drawFractalTreeRec(cubeVAO, sphere, lightingShader, transform, maxDepth, maxDepth, 2.5f, 0.8f);
}

// ===== ENVIRONMENT HELPER FUNCTIONS =====

void drawMountain(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position, glm::vec3 scale, glm::vec3 color)
{
    // Atmospheric perspective: tint the given color slightly towards sky blue/cyan to make it look distant.
    glm::vec3 atmosphericTint = glm::vec3(0.4f, 0.5f, 0.65f);
    glm::vec3 finalColor = glm::mix(color, atmosphericTint, 0.40f); // Blend 40% with sky hue

    glm::vec3 darkRock = finalColor * 0.75f;
    glm::vec3 midRock = finalColor * 0.90f;
    glm::vec3 lightRock = glm::clamp(finalColor * 1.15f, 0.0f, 1.0f);
    glm::vec3 snow = glm::vec3(0.92f, 0.95f, 1.0f); // Bright crisp snow

    // 1. Massive Base / Foundation (Wide)
    drawCube(cubeVAO, lightingShader, darkRock,
        position + glm::vec3(0.0f, scale.y * 0.2f, 0.0f),
        glm::vec3(0.0f, 15.0f, 0.0f), // Slightly twisted
        glm::vec3(scale.x * 1.3f, scale.y * 0.4f, scale.z * 1.1f), texRock, 2, 6.0f);

    // 2. Main Central Peak (Tall and majestic)
    drawCube(cubeVAO, lightingShader, midRock,
        position + glm::vec3(scale.x * 0.05f, scale.y * 0.45f, -scale.z * 0.1f),
        glm::vec3(0.0f, 45.0f, 0.0f), // Diamond shape profile creates sharp edges
        glm::vec3(scale.x * 0.75f, scale.y * 0.9f, scale.z * 0.7f), texRock, 2, 5.0f);

    // 3. Central Peak Snowcap (Slightly larger, sitting on top)
    drawCube(cubeVAO, lightingShader, snow,
        position + glm::vec3(scale.x * 0.05f, scale.y * 0.84f, -scale.z * 0.1f),
        glm::vec3(0.0f, 45.0f, 0.0f),
        glm::vec3(scale.x * 0.76f, scale.y * 0.14f, scale.z * 0.71f));

    // 4. Rugged Left Peak
    drawCube(cubeVAO, lightingShader, lightRock,
        position + glm::vec3(-scale.x * 0.4f, scale.y * 0.35f, scale.z * 0.15f),
        glm::vec3(0.0f, -25.0f, 0.0f),
        glm::vec3(scale.x * 0.55f, scale.y * 0.7f, scale.z * 0.5f), texRock, 2, 4.0f);

    // Left Peak Snowcap
    drawCube(cubeVAO, lightingShader, snow,
        position + glm::vec3(-scale.x * 0.4f, scale.y * 0.65f, scale.z * 0.15f),
        glm::vec3(0.0f, -25.0f, 0.0f),
        glm::vec3(scale.x * 0.56f, scale.y * 0.12f, scale.z * 0.51f));

    // 5. Jagged Right Peak (Further back, sharp)
    drawCube(cubeVAO, lightingShader, darkRock,
        position + glm::vec3(scale.x * 0.45f, scale.y * 0.3f, -scale.z * 0.25f),
        glm::vec3(0.0f, 65.0f, 0.0f),
        glm::vec3(scale.x * 0.45f, scale.y * 0.6f, scale.z * 0.5f), texRock, 2, 3.5f);

    // Right Peak Snowcap
    drawCube(cubeVAO, lightingShader, snow,
        position + glm::vec3(scale.x * 0.45f, scale.y * 0.57f, -scale.z * 0.25f),
        glm::vec3(0.0f, 65.0f, 0.0f),
        glm::vec3(scale.x * 0.46f, scale.y * 0.08f, scale.z * 0.51f));

    // 6. Front Rocky Outcropping (Breaks up the flatness facing the player)
    drawCube(cubeVAO, lightingShader, midRock,
        position + glm::vec3(scale.x * 0.1f, scale.y * 0.25f, scale.z * 0.45f),
        glm::vec3(0.0f, 12.0f, 0.0f),
        glm::vec3(scale.x * 0.8f, scale.y * 0.5f, scale.z * 0.4f), texRock, 2, 4.0f);

    // 7. Small spires (Tiny sharp rocks around base)
    drawCube(cubeVAO, lightingShader, lightRock,
        position + glm::vec3(-scale.x * 0.75f, scale.y * 0.15f, -scale.z * 0.1f),
        glm::vec3(0.0f, 80.0f, 0.0f),
        glm::vec3(scale.x * 0.2f, scale.y * 0.3f, scale.z * 0.2f), texRock, 2, 2.0f);

    drawCube(cubeVAO, lightingShader, darkRock,
        position + glm::vec3(scale.x * 0.8f, scale.y * 0.12f, scale.z * 0.2f),
        glm::vec3(0.0f, -30.0f, 0.0f),
        glm::vec3(scale.x * 0.25f, scale.y * 0.25f, scale.z * 0.2f), texRock, 2, 2.0f);
}

void drawBuilding(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position, float width, float height, float depth, glm::vec3 wallColor, glm::vec3 roofColor)
{
    // ===== OUTER WALLS =====
    // Left wall
    drawCube(cubeVAO, lightingShader, wallColor, position + glm::vec3(-width * 0.5f + 0.1f, height * 0.5f, 0.0f), glm::vec3(0.0f), glm::vec3(0.2f, height, depth), texBricks, 2, 2.0f);
    // Right wall
    drawCube(cubeVAO, lightingShader, wallColor, position + glm::vec3(width * 0.5f - 0.1f, height * 0.5f, 0.0f), glm::vec3(0.0f), glm::vec3(0.2f, height, depth), texBricks, 2, 2.0f);
    // Back wall
    drawCube(cubeVAO, lightingShader, wallColor, position + glm::vec3(0.0f, height * 0.5f, -depth * 0.5f + 0.1f), glm::vec3(0.0f), glm::vec3(width, height, 0.2f), texBricks, 2, 2.0f);
    
    // Front wall (with door cutout)
    float doorW = 1.2f;
    float doorH = 2.4f;
    float leftPartW = (width - doorW) * 0.5f;
    
    // Left front wall
    glm::vec3 lFrontPos = position + glm::vec3(-width * 0.5f + leftPartW * 0.5f, height * 0.5f, depth * 0.5f - 0.1f);
    drawCube(cubeVAO, lightingShader, wallColor, lFrontPos, glm::vec3(0.0f), glm::vec3(leftPartW, height, 0.2f), texBricks, 2, 2.0f);
    
    // Right front wall
    glm::vec3 rFrontPos = position + glm::vec3(width * 0.5f - leftPartW * 0.5f, height * 0.5f, depth * 0.5f - 0.1f);
    drawCube(cubeVAO, lightingShader, wallColor, rFrontPos, glm::vec3(0.0f), glm::vec3(leftPartW, height, 0.2f), texBricks, 2, 2.0f);
    
    // Top front wall (above door)
    float topPartH = height - doorH;
    if (topPartH > 0.0f) {
        glm::vec3 tFrontPos = position + glm::vec3(0.0f, doorH + topPartH * 0.5f, depth * 0.5f - 0.1f);
        drawCube(cubeVAO, lightingShader, wallColor, tFrontPos, glm::vec3(0.0f), glm::vec3(doorW, topPartH, 0.2f), texBricks, 2, 2.0f);
    }

    // ===== ROOF & FLOOR =====
    // Roof slab (slightly overhanging)
    glm::vec3 roofPos = position + glm::vec3(0.0f, height + 0.15f, 0.0f);
    drawCube(cubeVAO, lightingShader, roofColor, roofPos, glm::vec3(0.0f), glm::vec3(width + 0.8f, 0.4f, depth + 0.8f));
    
    // Support beams under roof
    drawCube(cubeVAO, lightingShader, glm::vec3(0.2f, 0.12f, 0.08f), position + glm::vec3(-width * 0.5f - 0.1f, height - 0.1f, 0.0f), glm::vec3(0.0f), glm::vec3(0.2f, 0.2f, depth + 0.6f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.2f, 0.12f, 0.08f), position + glm::vec3(width * 0.5f + 0.1f, height - 0.1f, 0.0f), glm::vec3(0.0f), glm::vec3(0.2f, 0.2f, depth + 0.6f));

    // Floor (wooden planks)
    drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.22f, 0.10f), position + glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(0.0f), glm::vec3(width - 0.4f, 0.2f, depth - 0.4f), texBark, 2);

    // ===== DOORS & WINDOWS =====
    // Front Door (swung open slightly inwards)
    glm::vec3 doorPos = position + glm::vec3(-doorW * 0.5f + 0.08f, doorH * 0.5f, depth * 0.5f - 0.5f);
    drawCube(cubeVAO, lightingShader, glm::vec3(0.3f, 0.18f, 0.10f), doorPos, glm::vec3(0.0f, -45.0f, 0.0f), glm::vec3(doorW - 0.1f, doorH - 0.1f, 0.08f), texBark, 2);
    // Door handle
    drawCube(cubeVAO, lightingShader, glm::vec3(0.8f, 0.7f, 0.2f), doorPos + glm::vec3(0.3f, 0.0f, -0.4f), glm::vec3(0.0f, -45.0f, 0.0f), glm::vec3(0.12f, 0.04f, 0.04f));

    glm::vec3 winColor = glm::vec3(0.4f, 0.6f, 0.8f);
    // Ground floor windows
    drawCube(cubeVAO, lightingShader, winColor, position + glm::vec3(-width * 0.32f, 1.4f, depth * 0.5f + 0.02f), glm::vec3(0.0f), glm::vec3(0.6f, 0.8f, 0.05f));
    drawCube(cubeVAO, lightingShader, winColor, position + glm::vec3(width * 0.32f, 1.4f, depth * 0.5f + 0.02f), glm::vec3(0.0f), glm::vec3(0.6f, 0.8f, 0.05f));
    
    if (height > 5.0f) {
        // Upper floor windows
        drawCube(cubeVAO, lightingShader, winColor, position + glm::vec3(-width * 0.32f, 4.0f, depth * 0.5f + 0.02f), glm::vec3(0.0f), glm::vec3(0.6f, 0.8f, 0.05f));
        drawCube(cubeVAO, lightingShader, winColor, position + glm::vec3(width * 0.32f, 4.0f, depth * 0.5f + 0.02f), glm::vec3(0.0f), glm::vec3(0.6f, 0.8f, 0.05f));
        drawCube(cubeVAO, lightingShader, winColor, position + glm::vec3(0.0f, 4.0f, depth * 0.5f + 0.02f), glm::vec3(0.0f), glm::vec3(0.6f, 0.8f, 0.05f));
    }

    // ===== INTERIOR (Bar, Table, Chairs) =====
    // Main Bar Counter (inside, right side)
    drawCube(cubeVAO, lightingShader, glm::vec3(0.25f, 0.15f, 0.08f), position + glm::vec3(width * 0.2f, 0.55f, -depth * 0.1f), glm::vec3(0.0f), glm::vec3(width * 0.45f, 0.9f, 0.8f), texBark, 2);
    // Counter top
    drawCube(cubeVAO, lightingShader, glm::vec3(0.40f, 0.25f, 0.12f), position + glm::vec3(width * 0.2f, 1.05f, -depth * 0.1f), glm::vec3(0.0f), glm::vec3(width * 0.5f, 0.1f, 0.9f));
    
    // Bar Stools
    float stoolY = 0.45f;
    for (int i = -1; i <= 1; i++) {
        float sx = (width * 0.2f) + i * 0.55f;
        // Stool leg
        drawCube(cubeVAO, lightingShader, glm::vec3(0.15f), position + glm::vec3(sx, stoolY, -depth * 0.1f + 0.7f), glm::vec3(0.0f), glm::vec3(0.06f, 0.7f, 0.06f));
        // Stool seat (round approximated)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.6f, 0.2f, 0.15f), position + glm::vec3(sx, 0.85f, -depth * 0.1f + 0.7f), glm::vec3(0.0f), glm::vec3(0.35f, 0.1f, 0.35f));
    }

    // Drink glasses on bar
    drawCube(cubeVAO, lightingShader, glm::vec3(0.8f, 0.9f, 1.0f), position + glm::vec3(width * 0.1f, 1.18f, -depth * 0.1f - 0.2f), glm::vec3(0.0f), glm::vec3(0.08f, 0.16f, 0.08f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.9f, 0.5f, 0.2f), position + glm::vec3(width * 0.3f, 1.15f, -depth * 0.1f), glm::vec3(0.0f), glm::vec3(0.12f, 0.10f, 0.12f));

    // Wooden Table (inside, left side)
    float tx = -width * 0.25f;
    float tz = depth * 0.15f;
    // Table leg (thick center post)
    drawCube(cubeVAO, lightingShader, glm::vec3(0.25f, 0.15f, 0.08f), position + glm::vec3(tx, 0.45f, tz), glm::vec3(0.0f), glm::vec3(0.15f, 0.9f, 0.15f));
    // Table top
    drawCube(cubeVAO, lightingShader, glm::vec3(0.45f, 0.30f, 0.15f), position + glm::vec3(tx, 0.95f, tz), glm::vec3(0.0f), glm::vec3(1.1f, 0.1f, 1.1f), texBark, 2);
    
    // Chairs around table
    auto drawChair = [&](glm::vec3 cPos, float yRot) {
        // Seat
        drawCube(cubeVAO, lightingShader, glm::vec3(0.35f, 0.20f, 0.10f), cPos + glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, yRot, 0.0f), glm::vec3(0.45f, 0.08f, 0.45f));
        // Backrest
        glm::vec3 backOffset = glm::vec3(sin(glm::radians(yRot)) * -0.2f, 0.9f, cos(glm::radians(yRot)) * -0.2f);
        drawCube(cubeVAO, lightingShader, glm::vec3(0.30f, 0.18f, 0.08f), cPos + backOffset, glm::vec3(0.0f, yRot, 0.0f), glm::vec3(0.45f, 0.8f, 0.06f));
        // Base / Legs (simplified central block)
        drawCube(cubeVAO, lightingShader, glm::vec3(0.20f, 0.12f, 0.05f), cPos + glm::vec3(0.0f, 0.25f, 0.0f), glm::vec3(0.0f), glm::vec3(0.2f, 0.5f, 0.2f));
    };

    drawChair(position + glm::vec3(tx - 0.8f, 0.0f, tz), 90.0f);  // Left chair
    drawChair(position + glm::vec3(tx + 0.8f, 0.0f, tz), -90.0f); // Right chair
    drawChair(position + glm::vec3(tx, 0.0f, tz - 0.8f), 0.0f);   // Back chair

    // Shelves on back wall (behind bar)
    drawCube(cubeVAO, lightingShader, glm::vec3(0.3f, 0.2f, 0.1f), position + glm::vec3(width * 0.25f, 1.8f, -depth * 0.5f + 0.25f), glm::vec3(0.0f), glm::vec3(1.5f, 0.06f, 0.3f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.3f, 0.2f, 0.1f), position + glm::vec3(width * 0.25f, 2.4f, -depth * 0.5f + 0.25f), glm::vec3(0.0f), glm::vec3(1.5f, 0.06f, 0.3f));
    
    // Bottles on shelves
    drawCube(cubeVAO, lightingShader, glm::vec3(0.2f, 0.8f, 0.3f), position + glm::vec3(width * 0.1f, 2.0f, -depth * 0.5f + 0.25f), glm::vec3(0.0f), glm::vec3(0.1f, 0.3f, 0.1f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.8f, 0.2f, 0.2f), position + glm::vec3(width * 0.2f, 2.05f, -depth * 0.5f + 0.25f), glm::vec3(0.0f), glm::vec3(0.12f, 0.4f, 0.12f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.9f, 0.8f, 0.2f), position + glm::vec3(width * 0.35f, 2.6f, -depth * 0.5f + 0.25f), glm::vec3(0.0f), glm::vec3(0.15f, 0.35f, 0.15f));
    
    // Light / Lantern hanging from ceiling
    glm::vec3 lightPos = position + glm::vec3(0.0f, height - 0.5f, 0.0f);
    drawCube(cubeVAO, lightingShader, glm::vec3(1.0f, 0.9f, 0.5f), lightPos, glm::vec3(0.0f), glm::vec3(0.2f, 0.2f, 0.2f));
    drawCube(cubeVAO, lightingShader, glm::vec3(0.1f), lightPos + glm::vec3(0.0f, 0.3f, 0.0f), glm::vec3(0.0f), glm::vec3(0.04f, 0.4f, 0.04f)); // chain
}

void drawFencePost(unsigned int& cubeVAO, Shader& lightingShader, glm::vec3 position)
{
    // Vertical post
    drawCube(cubeVAO, lightingShader, glm::vec3(0.45f, 0.35f, 0.2f), position + glm::vec3(0.0f, 0.6f, 0.0f), glm::vec3(0.0f), glm::vec3(0.12f, 1.2f, 0.12f), texBark, 2);
}

// ===== FRACTAL TREE (Koch-style recursive self-similarity) =====
// Each call draws one branch (a thin elongated cube) using a precomputed
// model matrix, so the recursive caller can chain rotations/translations
// without losing the parent transform.
void drawFractalBranch(unsigned int& cubeVAO, Shader& lightingShader, glm::mat4 model, glm::vec3 color, unsigned int texID, int texMode)
{
    lightingShader.use();
    lightingShader.setVec3("material.ambient",  color * 0.25f);
    lightingShader.setVec3("material.diffuse",  color);
    lightingShader.setVec3("material.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    lightingShader.setFloat("material.shininess", 16.0f);
    lightingShader.setMat4("model", model);
    lightingShader.setFloat("texScale", 1.0f);

    if (texID > 0 && texturesEnabled) {
        lightingShader.setInt("isTextured", 1);
        lightingShader.setInt("textureMode", texMode);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);
        lightingShader.setInt("diffuseMap", 0);
    } else {
        lightingShader.setInt("isTextured", 0);
    }

    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

// Recursively draws a fractal tree. At each step we draw the current
// branch from local origin upward by `length`, then at the tip we recurse
// `subBranches` times with rotated, smaller copies of the same shape.
// This is exact self-similarity (per the Koch-curve recursive scheme):
//   if (depth == 0) draw a leaf;
//   else { draw branch; for each child: rotate, scale, recurse; }
void drawFractalTreeRec(unsigned int& cubeVAO, Sphere& sphere, Shader& lightingShader, glm::mat4 transform, int depth, int maxDepth, float length, float thickness)
{
    if (depth <= 0) {
        glm::mat4 leafModel = glm::translate(transform, glm::vec3(0.0f, length * 0.5f, 0.0f));
        leafModel = glm::scale(leafModel, glm::vec3(thickness * 6.0f));

        glm::vec3 oldAmb = sphere.ambient;
        glm::vec3 oldDiff = sphere.diffuse;
        glm::vec3 oldSpec = sphere.specular;
        float oldShiny = sphere.shininess;

        sphere.set(sphere.radius, sphere.sectorCount, sphere.stackCount, glm::vec3(0.0f, 0.6f, 0.0f), glm::vec3(0.0f, 0.6f, 0.0f), glm::vec3(0.1f, 0.1f, 0.1f), 16.0f);

        if (texturesEnabled && texLeaf > 0) {
            lightingShader.setInt("isTextured", 1);
            lightingShader.setInt("textureMode", 2);
            lightingShader.setFloat("texScale", 2.0f);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texLeaf);
            lightingShader.setInt("diffuseMap", 0);
        } else {
            lightingShader.setInt("isTextured", 0);
        }

        sphere.drawSphere(lightingShader, leafModel);
        lightingShader.setInt("isTextured", 0);
        lightingShader.setFloat("texScale", 1.0f);

        sphere.set(sphere.radius, sphere.sectorCount, sphere.stackCount, oldAmb, oldDiff, oldSpec, oldShiny);
        return;
    }

    glm::mat4 branchModel = glm::translate(transform, glm::vec3(0.0f, length * 0.5f, 0.0f));
    branchModel = glm::scale(branchModel, glm::vec3(thickness, length, thickness));

    float t = 1.0f - (float)depth / maxDepth;
    glm::vec3 branchColor = glm::mix(glm::vec3(0.40f, 0.22f, 0.10f), glm::vec3(0.15f, 0.45f, 0.15f), t);

    drawFractalBranch(cubeVAO, lightingShader, branchModel, branchColor, texBark, 2);

    glm::mat4 endTransform = glm::translate(transform, glm::vec3(0.0f, length, 0.0f));

    float r1 = (float)((depth * 13) % 11 - 5);
    float r2 = (float)((depth * 17) % 11 - 5);
    float r3 = (float)((depth * 19) % 11 - 5);

    float newThickness = thickness * (0.65f);

    // Left branch (+25 deg approx)
    float yaw1 = (float)((depth * 23) % 20 - 10);
    glm::mat4 child1 = glm::rotate(endTransform, glm::radians(0.0f + yaw1), glm::vec3(0.0f, 1.0f, 0.0f));
    child1 = glm::rotate(child1, glm::radians(25.0f + r1), glm::vec3(0.0f, 0.0f, 1.0f));
    drawFractalTreeRec(cubeVAO, sphere, lightingShader, child1, depth - 1, maxDepth, length * 0.65f, newThickness);

    // Right branch (-25 deg approx)
    float yaw2 = (float)((depth * 29) % 20 - 10);
    glm::mat4 child2 = glm::rotate(endTransform, glm::radians(180.0f + yaw2), glm::vec3(0.0f, 1.0f, 0.0f));
    child2 = glm::rotate(child2, glm::radians(25.0f + r2), glm::vec3(0.0f, 0.0f, 1.0f));
    drawFractalTreeRec(cubeVAO, sphere, lightingShader, child2, depth - 1, maxDepth, length * 0.65f, newThickness);

    // Straight-ish branch
    float yaw3 = (float)((depth * 31) % 360);
    glm::mat4 child3 = glm::rotate(endTransform, glm::radians(yaw3), glm::vec3(0.0f, 1.0f, 0.0f));
    child3 = glm::rotate(child3, glm::radians(5.0f + r3), glm::vec3(0.0f, 0.0f, 1.0f));
    drawFractalTreeRec(cubeVAO, sphere, lightingShader, child3, depth - 1, maxDepth, length * 0.70f, newThickness);
}

// ===== HUD SCOREBOARD (top-right corner, fixed in screen space) =====
// Drawn after the 3D scene with an orthographic projection in pixel
// coordinates so that camera movement does not shift it. Uses the simple
// flat-color shader (vertexShader.vs / fragmentShader.fs) to bypass lighting.
void drawScoreboardHUD(unsigned int& cubeVAO, Shader& hudShader, int score, bool gameOverFlag)
{
    hudShader.use();
    glm::mat4 hudProjection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
    glm::mat4 hudView = glm::mat4(1.0f);
    hudShader.setMat4("projection", hudProjection);
    hudShader.setMat4("view", hudView);

    // HUD must draw on top of the 3D scene, so disable depth testing.
    GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    // Pixel-space helper: draw a flat rectangle (Z=0) with the given color.
    auto drawRect = [&](glm::vec3 color, float alpha, float cx, float cy, float w, float h) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(cx, cy, 0.0f));
        m = glm::scale(m, glm::vec3(w, h, 1.0f));
        hudShader.setMat4("model", m);
        hudShader.setVec3("color", color);
        hudShader.setFloat("alpha", alpha);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    };

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Anchor in the top-right corner.
    const float pad     = 15.0f;
    const float panelW  = 140.0f;
    const float panelH  = 55.0f;
    const float panelCx = (float)SCR_WIDTH  - pad - panelW * 0.5f;
    const float panelCy = (float)SCR_HEIGHT - pad - panelH * 0.5f;

    glm::vec3 borderColor = gameOverFlag ? glm::vec3(0.90f, 0.10f, 0.05f)
                                         : glm::vec3(0.85f, 0.65f, 0.20f);
    drawRect(borderColor, 0.4f,                         panelCx, panelCy, panelW + 7.0f, panelH + 7.0f);
    drawRect(glm::vec3(0.04f, 0.04f, 0.07f), 0.3f,      panelCx, panelCy, panelW,         panelH);

    // Split score into 4 digits (clamped 0..9999).
    int s = score;
    if (s < 0) s = 0;
    if (s > 9999) s = 9999;
    int d[4] = { (s / 1000) % 10, (s / 100) % 10, (s / 10) % 10, s % 10 };

    // 7-segment lookup: T(64) TL(32) TR(16) M(8) BL(4) BR(2) B(1)
    static const int segs[10] = { 119, 18, 93, 91, 58, 107, 111, 82, 127, 123 };

    glm::vec3 digitColor = gameOverFlag ? glm::vec3(1.0f, 0.15f, 0.05f)
                                        : glm::vec3(0.15f, 1.0f, 0.40f);

    const float digitW       = 19.0f;
    const float digitH       = 32.0f;
    const float digitSpacing = digitW + 11.0f;
    const float t            = 4.0f; // segment thickness in pixels
    const float halfW        = digitW * 0.5f;
    const float halfH        = digitH * 0.5f;

    float startX = panelCx - 1.5f * digitSpacing;
    float dy     = panelCy;

    for (int i = 0; i < 4; i++) {
        float dx  = startX + i * digitSpacing;
        int   seg = segs[d[i]];
        if (seg & 64) drawRect(digitColor, 1.0f, dx,            dy + halfH,        digitW + t, t);
        if (seg & 32) drawRect(digitColor, 1.0f, dx - halfW,    dy + halfH * 0.5f, t,          halfH);
        if (seg & 16) drawRect(digitColor, 1.0f, dx + halfW,    dy + halfH * 0.5f, t,          halfH);
        if (seg &  8) drawRect(digitColor, 1.0f, dx,            dy,                digitW + t, t);
        if (seg &  4) drawRect(digitColor, 1.0f, dx - halfW,    dy - halfH * 0.5f, t,          halfH);
        if (seg &  2) drawRect(digitColor, 1.0f, dx + halfW,    dy - halfH * 0.5f, t,          halfH);
        if (seg &  1) drawRect(digitColor, 1.0f, dx,            dy - halfH,        digitW + t, t);
    }

    glDisable(GL_BLEND);
    if (depthWasOn) glEnable(GL_DEPTH_TEST);
}

void setFireMode(FireMode mode, bool showToast)
{
    if (fireMode == mode) return;
    fireMode = mode;
    if (showToast) {
        modeToastText = (mode == FireMode::Bullet) ? "BULLET" : "ARROW";
        modeToastTimer = modeToastDuration;
    }
}

void fireBullet()
{
    if (bulletAmmo <= 0) {
        return;
    }
    // Spawn bullet from gun barrel position
    glm::vec3 spawnPos = basic_camera.eye + basic_camera.Front * 0.5f + basic_camera.Right * 0.15f;
    projectiles.emplace_back(spawnPos, basic_camera.Front * bulletSpeed, false);
    bulletAmmo--;
    gunJustFired = true;
    gunRecoilTimer = 0.0f;
    if (bulletAmmo <= 0) {
        bulletReloading = true;
        bulletReloadTimer = 0.0f;
    }
}

void fireArrow()
{
    if (arrowAmmo <= 0 || arrowReloading) {
        return;
    }
    // Gun-style: arrow fires straight forward from camera position
    glm::vec3 spawnPos = basic_camera.eye + basic_camera.Front * 0.5f + basic_camera.Right * 0.15f;
    glm::vec3 initialVel = basic_camera.Front * arrowBaseSpeed;
    projectiles.emplace_back(spawnPos, initialVel, true);
    arrowAmmo--;
    arrowJustFired = true;
    arrowRecoilTimer = 0.0f;
    if (arrowAmmo <= 0) {
        arrowReloading = true;
        arrowReloadTimer = 0.0f;
    }
}

void updateProjectileSystem(float dt)
{
    for (auto& proj : projectiles) {
        if (!proj.active) {
            continue;
        }

        glm::vec3 prevPos = proj.position; // Save for swept collision

        if (!proj.isArrow) {
            proj.position += proj.velocity * dt;
            if (glm::length(proj.position) > 100.0f) {
                proj.active = false;
            }
        }
        else {
            if (proj.stuck) {
                proj.stuckTimer += dt;
                if (proj.stuckTimer >= 3.0f) {
                    proj.active = false;
                }
            }
            else {
                glm::vec2 currentWind = getEffectiveWindForce();
                proj.velocity.y -= arrowGravity * dt;
                proj.velocity.x += currentWind.x * dt;
                proj.velocity.z += currentWind.y * dt;
                proj.velocity *= 0.99f;
                proj.position += proj.velocity * dt;

                if (glm::length(proj.velocity) > 0.001f) {
                    proj.rotation = glm::quatLookAt(glm::normalize(proj.velocity), glm::vec3(0.0f, 1.0f, 0.0f));
                }

                if (proj.position.y <= groundY || glm::length(proj.position) > 140.0f) {
                    proj.stuck = true;
                    proj.stuckTimer = 0.0f;
                    if (proj.position.y < groundY) {
                        proj.position.y = groundY;
                    }
                }
            }
        }

        if (!proj.active || (proj.isArrow && proj.stuck)) {
            continue;
        }

        // Swept collision: check at previous position, midpoint, and current position
        // Arrow has larger hit radius because of arc trajectory
        float hitRadius = proj.isArrow ? 2.5f : 1.2f;
        glm::vec3 midPos = (prevPos + proj.position) * 0.5f;

        for (auto& b : balloons) {
            if (!b.active || b.isPopping) {
                continue;
            }
            // Check 3 points along the path to prevent tunneling
            float d1 = glm::length(prevPos - b.position);
            float d2 = glm::length(midPos - b.position);
            float d3 = glm::length(proj.position - b.position);
            float minDist = glm::min(d1, glm::min(d2, d3));

            if (minDist < hitRadius) {
                b.isPopping = true;
                b.popTimer = 0.0f;
                proj.active = false;
                score++;
                std::cout << "Balloon popped! Score: " << score << std::endl;
                break;
            }
        }
    }
}

void updateAmmoReloadSystem(float dt)
{
    if (modeToastTimer > 0.0f) {
        modeToastTimer -= dt;
        if (modeToastTimer < 0.0f) modeToastTimer = 0.0f;
    }

    // Update first-person arrow animation
    fpArrowBobTime += dt;
    if (gunJustFired) {
        gunRecoilTimer += dt;
        if (gunRecoilTimer >= gunRecoilDuration) {
            gunJustFired = false;
            gunRecoilTimer = 0.0f;
        }
    }
    if (arrowJustFired) {
        arrowRecoilTimer += dt;
        if (arrowRecoilTimer >= arrowRecoilDuration) {
            arrowJustFired = false;
            arrowRecoilTimer = 0.0f;
        }
    }

    if (bulletReloading) {
        bulletReloadTimer += dt;
        if (bulletReloadTimer >= bulletReloadTime) {
            bulletAmmo = maxBulletAmmo;
            bulletReloadTimer = 0.0f;
            bulletReloading = false;
        }
    }

    if (fireMode == FireMode::Arrow && arrowAmmo <= 0 && !arrowReloading) {
        arrowReloading = true;
        arrowReloadTimer = 0.0f;
    }

    if (arrowReloading) {
        arrowReloadTimer += dt;
        if (arrowReloadTimer >= arrowReloadTime) {
            arrowAmmo = maxArrowAmmo;
            arrowReloadTimer = 0.0f;
            arrowReloading = false;
        }
    }
}

void drawFirstPersonArrow(unsigned int& cubeVAO, Shader& lightingShader)
{
    if (fireMode != FireMode::Arrow) return;
    if (arrowAmmo <= 0) return; // Hide when out of ammo (reloading)

    lightingShader.use();

    // ===== Position arrow relative to camera (FPS weapon style) =====
    float bobY = sin(fpArrowBobTime * 2.5f) * 0.012f;   // Gentle breathing bob
    float bobX = sin(fpArrowBobTime * 1.5f) * 0.005f;

    // Recoil animation: arrow pulls back then returns
    float recoilOffset = 0.0f;
    if (arrowJustFired) {
        float t = arrowRecoilTimer / arrowRecoilDuration;
        recoilOffset = sin(t * 3.14159f) * 0.15f;
    }

    glm::vec3 arrowOrigin = basic_camera.eye
        + basic_camera.Front * (0.6f - recoilOffset)   // Forward (with recoil)
        + basic_camera.Right * (0.22f + bobX)            // Right offset
        + basic_camera.Up * (-0.15f + bobY);             // Below center

    glm::vec3 arrowDir = glm::normalize(basic_camera.Front);
    glm::vec3 arrowUp = glm::normalize(basic_camera.Up);

    glm::quat arrowRot = glm::quatLookAt(arrowDir, arrowUp);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), arrowOrigin) * glm::toMat4(arrowRot);

    // ===== ARROW SHAFT � Cubic Bezier curve for realistic slight flex =====
    // Control points: slight upward bow like a real wooden arrow
    glm::vec3 cp0(0.0f, 0.0f, -0.35f);      // Nock end (back)
    glm::vec3 cp1(0.0f, 0.006f, -0.12f);     // Control 1 (upward bow)
    glm::vec3 cp2(0.0f, 0.004f, 0.12f);      // Control 2
    glm::vec3 cp3(0.0f, 0.0f, 0.35f);        // Tip end (front)

    const int shaftSegments = 14;
    for (int i = 0; i < shaftSegments; i++) {
        float t1 = (float)i / shaftSegments;
        float t2 = (float)(i + 1) / shaftSegments;
        float tMid = (t1 + t2) * 0.5f;

        // Cubic Bezier: B(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)*t^2*P2 + t^3*P3
        auto bezier = [](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, float t) -> glm::vec3 {
            float u = 1.0f - t;
            return u*u*u*a + 3.0f*u*u*t*b + 3.0f*u*t*t*c + t*t*t*d;
        };

        glm::vec3 pos1 = bezier(cp0, cp1, cp2, cp3, t1);
        glm::vec3 pos2 = bezier(cp0, cp1, cp2, cp3, t2);
        glm::vec3 posMid = bezier(cp0, cp1, cp2, cp3, tMid);
        glm::vec3 tangent = glm::normalize(pos2 - pos1);
        float segLen = glm::length(pos2 - pos1);

        // Taper: thicker in middle, thinner at ends (realistic wood shaft)
        float halfDist = glm::abs(tMid - 0.5f) * 2.0f;
        float thickness = 0.013f * (1.0f - 0.25f * halfDist);

        // Transform to world space via the base matrix
        glm::vec3 worldPos = glm::vec3(base * glm::vec4(posMid, 1.0f));
        glm::vec3 worldTangent = glm::normalize(glm::vec3(base * glm::vec4(tangent, 0.0f)));

        glm::quat segRot = glm::quatLookAt(worldTangent, arrowUp);
        glm::mat4 segModel = glm::translate(glm::mat4(1.0f), worldPos)
                           * glm::toMat4(segRot)
                           * glm::scale(glm::mat4(1.0f), glm::vec3(thickness, thickness, segLen));

        lightingShader.setMat4("model", segModel);
        // Warm wood color with slight gradient along length
        float cv = tMid * 0.12f;
        lightingShader.setVec3("material.ambient", glm::vec3(0.18f + cv, 0.12f + cv * 0.5f, 0.04f));
        lightingShader.setVec3("material.diffuse", glm::vec3(0.68f + cv, 0.45f + cv * 0.5f, 0.15f));
        lightingShader.setVec3("material.specular", glm::vec3(0.35f, 0.28f, 0.12f));
        lightingShader.setFloat("material.shininess", 28.0f);
        lightingShader.setInt("isTextured", 0);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // ===== ARROWHEAD � Sharp broadhead style (metallic steel) =====
    // Main blade (horizontal)
    glm::mat4 tipH = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.42f));
    tipH = glm::scale(tipH, glm::vec3(0.045f, 0.009f, 0.15f));
    lightingShader.setMat4("model", tipH);
    lightingShader.setVec3("material.ambient", glm::vec3(0.15f, 0.15f, 0.15f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.60f, 0.60f, 0.60f));
    lightingShader.setVec3("material.specular", glm::vec3(0.90f, 0.90f, 0.90f));
    lightingShader.setFloat("material.shininess", 128.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Cross blade (vertical)
    glm::mat4 tipV = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.42f));
    tipV = glm::scale(tipV, glm::vec3(0.009f, 0.045f, 0.15f));
    lightingShader.setMat4("model", tipV);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Collar (gold ring where head meets shaft)
    glm::mat4 collarM = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.355f));
    collarM = glm::scale(collarM, glm::vec3(0.024f, 0.024f, 0.018f));
    lightingShader.setMat4("model", collarM);
    lightingShader.setVec3("material.ambient", glm::vec3(0.22f, 0.18f, 0.06f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.80f, 0.65f, 0.18f));
    lightingShader.setVec3("material.specular", glm::vec3(0.8f, 0.7f, 0.3f));
    lightingShader.setFloat("material.shininess", 80.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== FLETCHING � 3 feathers at the back =====
    // Feather 1 (horizontal � crimson red)
    glm::mat4 f1 = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.26f));
    f1 = glm::scale(f1, glm::vec3(0.042f, 0.006f, 0.11f));
    lightingShader.setMat4("model", f1);
    lightingShader.setVec3("material.ambient", glm::vec3(0.15f, 0.03f, 0.02f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.80f, 0.12f, 0.06f));
    lightingShader.setVec3("material.specular", glm::vec3(0.12f));
    lightingShader.setFloat("material.shininess", 6.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Feather 2 (vertical � crimson red)
    glm::mat4 f2 = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.26f));
    f2 = glm::scale(f2, glm::vec3(0.006f, 0.042f, 0.11f));
    lightingShader.setMat4("model", f2);
    lightingShader.setVec3("material.diffuse", glm::vec3(0.80f, 0.12f, 0.06f));
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Feather 3 (diagonal � ivory/cream)
    glm::mat4 f3 = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.26f));
    f3 = glm::rotate(f3, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    f3 = glm::scale(f3, glm::vec3(0.038f, 0.006f, 0.10f));
    lightingShader.setMat4("model", f3);
    lightingShader.setVec3("material.diffuse", glm::vec3(0.92f, 0.88f, 0.72f));
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== NOCK � tail end cap =====
    glm::mat4 nockM = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.35f));
    nockM = glm::scale(nockM, glm::vec3(0.018f, 0.018f, 0.014f));
    lightingShader.setMat4("model", nockM);
    lightingShader.setVec3("material.ambient", glm::vec3(0.10f, 0.08f, 0.05f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.30f, 0.20f, 0.10f));
    lightingShader.setVec3("material.specular", glm::vec3(0.45f));
    lightingShader.setFloat("material.shininess", 48.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void drawFirstPersonGun(unsigned int& cubeVAO, Shader& lightingShader)
{
    if (fireMode != FireMode::Bullet) return;

    lightingShader.use();

    // ===== Position gun relative to camera (FPS weapon style) =====
    float bobY = sin(fpArrowBobTime * 2.0f) * 0.008f;
    float bobX = sin(fpArrowBobTime * 1.2f) * 0.004f;

    // Recoil: gun kicks back and up, then returns
    float recoilBack = 0.0f;
    float recoilUp = 0.0f;
    if (gunJustFired) {
        float t = gunRecoilTimer / gunRecoilDuration;
        recoilBack = sin(t * 3.14159f) * 0.08f;
        recoilUp = sin(t * 3.14159f) * 0.03f;
    }

    glm::vec3 gunOrigin = basic_camera.eye
        + basic_camera.Front * (0.45f - recoilBack)
        + basic_camera.Right * (0.28f + bobX)
        + basic_camera.Up * (-0.22f + bobY + recoilUp);

    glm::vec3 gunDir = glm::normalize(basic_camera.Front);
    glm::vec3 gunUp = glm::normalize(basic_camera.Up);
    glm::vec3 gunRight = glm::normalize(basic_camera.Right);

    glm::quat gunRot = glm::quatLookAt(gunDir, gunUp);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), gunOrigin) * glm::toMat4(gunRot);

    // Dark gunmetal color
    glm::vec3 gunmetalAmb(0.04f, 0.04f, 0.05f);
    glm::vec3 gunmetalDiff(0.18f, 0.18f, 0.20f);
    glm::vec3 gunmetalSpec(0.55f, 0.55f, 0.60f);
    float gunmetalShine = 64.0f;

    // ===== BARREL (long cylinder approximation) =====
    glm::mat4 barrelM = glm::translate(base, glm::vec3(0.0f, 0.015f, 0.18f));
    barrelM = glm::scale(barrelM, glm::vec3(0.022f, 0.022f, 0.28f));
    lightingShader.setMat4("model", barrelM);
    lightingShader.setVec3("material.ambient", gunmetalAmb);
    lightingShader.setVec3("material.diffuse", gunmetalDiff);
    lightingShader.setVec3("material.specular", gunmetalSpec);
    lightingShader.setFloat("material.shininess", gunmetalShine);
    lightingShader.setInt("isTextured", 0);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Barrel inner bore (dark hole at front)
    glm::mat4 boreM = glm::translate(base, glm::vec3(0.0f, 0.015f, 0.325f));
    boreM = glm::scale(boreM, glm::vec3(0.014f, 0.014f, 0.005f));
    lightingShader.setMat4("model", boreM);
    lightingShader.setVec3("material.ambient", glm::vec3(0.01f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.03f));
    lightingShader.setVec3("material.specular", glm::vec3(0.0f));
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== RECEIVER / SLIDE (main body � upper part) =====
    glm::mat4 slideM = glm::translate(base, glm::vec3(0.0f, 0.015f, 0.02f));
    slideM = glm::scale(slideM, glm::vec3(0.034f, 0.038f, 0.22f));
    lightingShader.setMat4("model", slideM);
    lightingShader.setVec3("material.ambient", gunmetalAmb * 1.2f);
    lightingShader.setVec3("material.diffuse", gunmetalDiff * 1.1f);
    lightingShader.setVec3("material.specular", gunmetalSpec);
    lightingShader.setFloat("material.shininess", 80.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Slide serrations (decorative lines at back of slide)
    for (int s = 0; s < 4; s++) {
        float zOff = -0.06f - s * 0.015f;
        glm::mat4 serrM = glm::translate(base, glm::vec3(0.0f, 0.015f, zOff));
        serrM = glm::scale(serrM, glm::vec3(0.036f, 0.040f, 0.004f));
        lightingShader.setMat4("model", serrM);
        lightingShader.setVec3("material.ambient", glm::vec3(0.02f));
        lightingShader.setVec3("material.diffuse", glm::vec3(0.10f, 0.10f, 0.11f));
        lightingShader.setVec3("material.specular", glm::vec3(0.3f));
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // ===== FRAME / LOWER RECEIVER =====
    glm::mat4 frameM = glm::translate(base, glm::vec3(0.0f, -0.008f, -0.01f));
    frameM = glm::scale(frameM, glm::vec3(0.030f, 0.016f, 0.18f));
    lightingShader.setMat4("model", frameM);
    lightingShader.setVec3("material.ambient", glm::vec3(0.03f, 0.03f, 0.04f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.15f, 0.15f, 0.17f));
    lightingShader.setVec3("material.specular", glm::vec3(0.40f));
    lightingShader.setFloat("material.shininess", 48.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== GRIP / HANDLE (angled downward) =====
    glm::mat4 gripM = glm::translate(base, glm::vec3(0.0f, -0.055f, -0.065f));
    gripM = glm::rotate(gripM, glm::radians(12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    gripM = glm::scale(gripM, glm::vec3(0.028f, 0.075f, 0.04f));
    lightingShader.setMat4("model", gripM);
    // Grip texture � slightly rough dark polymer
    lightingShader.setVec3("material.ambient", glm::vec3(0.02f, 0.02f, 0.02f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.08f, 0.08f, 0.09f));
    lightingShader.setVec3("material.specular", glm::vec3(0.10f));
    lightingShader.setFloat("material.shininess", 8.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Grip texture lines (stippling effect)
    for (int g = 0; g < 3; g++) {
        float yOff = -0.04f - g * 0.018f;
        glm::mat4 stipM = glm::translate(base, glm::vec3(0.0f, yOff, -0.065f));
        stipM = glm::scale(stipM, glm::vec3(0.030f, 0.005f, 0.042f));
        lightingShader.setMat4("model", stipM);
        lightingShader.setVec3("material.diffuse", glm::vec3(0.05f));
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // ===== TRIGGER GUARD =====
    // Bottom bar
    glm::mat4 tgBot = glm::translate(base, glm::vec3(0.0f, -0.025f, 0.01f));
    tgBot = glm::scale(tgBot, glm::vec3(0.020f, 0.005f, 0.05f));
    lightingShader.setMat4("model", tgBot);
    lightingShader.setVec3("material.ambient", gunmetalAmb);
    lightingShader.setVec3("material.diffuse", gunmetalDiff);
    lightingShader.setVec3("material.specular", gunmetalSpec);
    lightingShader.setFloat("material.shininess", gunmetalShine);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    // Front post
    glm::mat4 tgFront = glm::translate(base, glm::vec3(0.0f, -0.018f, 0.035f));
    tgFront = glm::scale(tgFront, glm::vec3(0.018f, 0.020f, 0.005f));
    lightingShader.setMat4("model", tgFront);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== TRIGGER =====
    glm::mat4 trigM = glm::translate(base, glm::vec3(0.0f, -0.018f, 0.012f));
    trigM = glm::rotate(trigM, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    trigM = glm::scale(trigM, glm::vec3(0.008f, 0.015f, 0.004f));
    lightingShader.setMat4("model", trigM);
    lightingShader.setVec3("material.ambient", glm::vec3(0.06f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.25f, 0.22f, 0.20f));
    lightingShader.setVec3("material.specular", glm::vec3(0.4f));
    lightingShader.setFloat("material.shininess", 32.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== FRONT SIGHT =====
    glm::mat4 fsM = glm::translate(base, glm::vec3(0.0f, 0.040f, 0.28f));
    fsM = glm::scale(fsM, glm::vec3(0.006f, 0.014f, 0.006f));
    lightingShader.setMat4("model", fsM);
    lightingShader.setVec3("material.ambient", gunmetalAmb);
    lightingShader.setVec3("material.diffuse", gunmetalDiff);
    lightingShader.setVec3("material.specular", gunmetalSpec);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    // Front sight dot (white/green)
    glm::mat4 fsDot = glm::translate(base, glm::vec3(0.0f, 0.048f, 0.28f));
    fsDot = glm::scale(fsDot, glm::vec3(0.004f, 0.004f, 0.004f));
    lightingShader.setMat4("model", fsDot);
    lightingShader.setVec3("material.ambient", glm::vec3(0.1f, 0.4f, 0.1f));
    lightingShader.setVec3("material.diffuse", glm::vec3(0.3f, 1.0f, 0.3f));
    lightingShader.setVec3("material.specular", glm::vec3(0.8f));
    lightingShader.setFloat("material.shininess", 96.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== REAR SIGHT =====
    // Left post
    glm::mat4 rsL = glm::translate(base, glm::vec3(-0.010f, 0.040f, -0.04f));
    rsL = glm::scale(rsL, glm::vec3(0.006f, 0.012f, 0.012f));
    lightingShader.setMat4("model", rsL);
    lightingShader.setVec3("material.ambient", gunmetalAmb);
    lightingShader.setVec3("material.diffuse", gunmetalDiff);
    lightingShader.setVec3("material.specular", gunmetalSpec);
    lightingShader.setFloat("material.shininess", gunmetalShine);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    // Right post
    glm::mat4 rsR = glm::translate(base, glm::vec3(0.010f, 0.040f, -0.04f));
    rsR = glm::scale(rsR, glm::vec3(0.006f, 0.012f, 0.012f));
    lightingShader.setMat4("model", rsR);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== MAGAZINE (bottom of grip) =====
    glm::mat4 magM = glm::translate(base, glm::vec3(0.0f, -0.098f, -0.065f));
    magM = glm::rotate(magM, glm::radians(12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    magM = glm::scale(magM, glm::vec3(0.024f, 0.012f, 0.035f));
    lightingShader.setMat4("model", magM);
    lightingShader.setVec3("material.ambient", gunmetalAmb);
    lightingShader.setVec3("material.diffuse", glm::vec3(0.12f, 0.12f, 0.13f));
    lightingShader.setVec3("material.specular", glm::vec3(0.35f));
    lightingShader.setFloat("material.shininess", 40.0f);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ===== MUZZLE FLASH (only briefly after firing) =====
    if (gunJustFired && gunRecoilTimer < gunRecoilDuration * 0.3f) {
        float flashIntensity = 1.0f - (gunRecoilTimer / (gunRecoilDuration * 0.3f));
        glm::mat4 flashM = glm::translate(base, glm::vec3(0.0f, 0.015f, 0.34f));
        flashM = glm::scale(flashM, glm::vec3(0.04f * flashIntensity, 0.04f * flashIntensity, 0.02f));
        lightingShader.setMat4("model", flashM);
        lightingShader.setVec3("material.ambient", glm::vec3(1.0f, 0.8f, 0.2f) * flashIntensity);
        lightingShader.setVec3("material.diffuse", glm::vec3(1.0f, 0.9f, 0.3f) * flashIntensity);
        lightingShader.setVec3("material.specular", glm::vec3(1.0f));
        lightingShader.setFloat("material.shininess", 128.0f);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
}

void drawCombatHUD(unsigned int& cubeVAO, Shader& hudShader)
{
    hudShader.use();
    glm::mat4 hudProjection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
    hudShader.setMat4("projection", hudProjection);
    hudShader.setMat4("view", glm::mat4(1.0f));

    GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto drawRect = [&](glm::vec3 color, float alpha, float cx, float cy, float w, float h) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(cx, cy, 0.0f));
        m = glm::scale(m, glm::vec3(w, h, 1.0f));
        hudShader.setMat4("model", m);
        hudShader.setVec3("color", color);
        hudShader.setFloat("alpha", alpha);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    };

    // ===== CROSSHAIR =====
    float cx = SCR_WIDTH * 0.5f;
    float cy = SCR_HEIGHT * 0.5f;

    glm::vec3 crossColor = (fireMode == FireMode::Arrow) ? glm::vec3(1.0f, 0.85f, 0.2f) : glm::vec3(1.0f);
    // Outer ring for arrow mode
    if (fireMode == FireMode::Arrow) {
        float ringSize = 28.0f;
        float t = 2.5f;
        drawRect(crossColor, 0.5f, cx, cy + ringSize * 0.5f, t, ringSize * 0.35f);
        drawRect(crossColor, 0.5f, cx, cy - ringSize * 0.5f, t, ringSize * 0.35f);
        drawRect(crossColor, 0.5f, cx + ringSize * 0.5f, cy, ringSize * 0.35f, t);
        drawRect(crossColor, 0.5f, cx - ringSize * 0.5f, cy, ringSize * 0.35f, t);
    }
    // Center cross
    drawRect(crossColor, 0.95f, cx, cy, 2.0f, 18.0f);
    drawRect(crossColor, 0.95f, cx, cy, 18.0f, 2.0f);
    // Center dot
    drawRect(crossColor, 1.0f, cx, cy, 4.0f, 4.0f);

    // ===== AMMO ICONS (bottom-left) =====
    int maxAmmo = (fireMode == FireMode::Arrow) ? maxArrowAmmo : maxBulletAmmo;
    int curAmmo = (fireMode == FireMode::Arrow) ? arrowAmmo : bulletAmmo;
    float startX = 24.0f;
    float startY = 30.0f;
    float w = (fireMode == FireMode::Arrow) ? 14.0f : 8.0f;
    float gap = (fireMode == FireMode::Arrow) ? 5.0f : 2.0f;
    for (int i = 0; i < maxAmmo; i++) {
        bool filled = i < curAmmo;
        drawRect(filled ? glm::vec3(0.2f, 0.95f, 0.35f) : glm::vec3(0.25f), 0.9f, startX + i * (w + gap), startY, w, 8.0f);
    }

    // ===== RELOAD BAR (center bottom, only when reloading) =====
    bool isReloading = (fireMode == FireMode::Arrow) ? arrowReloading : bulletReloading;
    if (isReloading) {
        float maxTime = (fireMode == FireMode::Arrow) ? arrowReloadTime : bulletReloadTime;
        float curTime = (fireMode == FireMode::Arrow) ? arrowReloadTimer : bulletReloadTimer;
        float progress = curTime / maxTime;
        drawRect(glm::vec3(0.12f, 0.10f, 0.08f), 0.75f, SCR_WIDTH * 0.5f, 40.0f, 224.0f, 18.0f);
        drawRect(glm::vec3(0.6f, 0.5f, 0.2f), 0.5f, SCR_WIDTH * 0.5f, 40.0f, 228.0f, 22.0f);
        if (progress > 0.01f) {
            drawRect(glm::vec3(1.0f, 0.85f, 0.2f), 0.95f,
                SCR_WIDTH * 0.5f - 110.0f + 110.0f * progress, 40.0f, 220.0f * progress, 14.0f);
        }
    }

    // ===== WIND INDICATOR (top-left) =====
    glm::vec2 windDir = glm::normalize(getEffectiveWindForce());
    drawRect(glm::vec3(0.2f, 0.85f, 1.0f), 0.9f, 55.0f, SCR_HEIGHT - 35.0f, 40.0f, 4.0f);
    drawRect(glm::vec3(0.2f, 0.85f, 1.0f), 0.9f, 55.0f + 20.0f * windDir.x, SCR_HEIGHT - 35.0f + 12.0f * windDir.y, 8.0f, 8.0f);

    // ===== MODE SWITCH TOAST (top-right, short duration) =====
    auto glyphRows = [&](char ch) {
        switch (ch) {
        case 'A': return std::array<std::string, 7>{ "01110","10001","10001","11111","10001","10001","10001" };
        case 'B': return std::array<std::string, 7>{ "11110","10001","10001","11110","10001","10001","11110" };
        case 'C': return std::array<std::string, 7>{ "01111","10000","10000","10000","10000","10000","01111" };
        case 'D': return std::array<std::string, 7>{ "11110","10001","10001","10001","10001","10001","11110" };
        case 'E': return std::array<std::string, 7>{ "11111","10000","10000","11110","10000","10000","11111" };
        case 'F': return std::array<std::string, 7>{ "11111","10000","10000","11110","10000","10000","10000" };
        case 'G': return std::array<std::string, 7>{ "01110","10000","10000","10111","10001","10001","01110" };
        case 'H': return std::array<std::string, 7>{ "10001","10001","10001","11111","10001","10001","10001" };
        case 'I': return std::array<std::string, 7>{ "01110","00100","00100","00100","00100","00100","01110" };
        case 'J': return std::array<std::string, 7>{ "00001","00001","00001","00001","10001","10001","01110" };
        case 'K': return std::array<std::string, 7>{ "10001","10010","10100","11000","10100","10010","10001" };
        case 'L': return std::array<std::string, 7>{ "10000","10000","10000","10000","10000","10000","11111" };
        case 'M': return std::array<std::string, 7>{ "10001","11011","10101","10001","10001","10001","10001" };
        case 'N': return std::array<std::string, 7>{ "10001","11001","10101","10011","10001","10001","10001" };
        case 'O': return std::array<std::string, 7>{ "01110","10001","10001","10001","10001","10001","01110" };
        case 'P': return std::array<std::string, 7>{ "11110","10001","10001","11110","10000","10000","10000" };
        case 'Q': return std::array<std::string, 7>{ "01110","10001","10001","10001","10101","10010","01101" };
        case 'R': return std::array<std::string, 7>{ "11110","10001","10001","11110","10100","10010","10001" };
        case 'S': return std::array<std::string, 7>{ "01111","10000","10000","01110","00001","00001","11110" };
        case 'T': return std::array<std::string, 7>{ "11111","00100","00100","00100","00100","00100","00100" };
        case 'U': return std::array<std::string, 7>{ "10001","10001","10001","10001","10001","10001","01110" };
        case 'V': return std::array<std::string, 7>{ "10001","10001","10001","10001","10001","01010","00100" };
        case 'W': return std::array<std::string, 7>{ "10001","10001","10001","10101","10101","11011","10001" };
        case 'X': return std::array<std::string, 7>{ "10001","10001","01010","00100","01010","10001","10001" };
        case 'Y': return std::array<std::string, 7>{ "10001","10001","01010","00100","00100","00100","00100" };
        case 'Z': return std::array<std::string, 7>{ "11111","00001","00010","00100","01000","10000","11111" };
        case '0': return std::array<std::string, 7>{ "01110","10001","10011","10101","11001","10001","01110" };
        case '1': return std::array<std::string, 7>{ "00100","01100","00100","00100","00100","00100","01110" };
        case '2': return std::array<std::string, 7>{ "01110","10001","00001","00110","01000","10000","11111" };
        case '3': return std::array<std::string, 7>{ "11111","00001","00010","00110","00001","10001","01110" };
        case '4': return std::array<std::string, 7>{ "00010","00110","01010","10010","11111","00010","00010" };
        case '5': return std::array<std::string, 7>{ "11111","10000","11110","00001","00001","10001","01110" };
        case '6': return std::array<std::string, 7>{ "00110","01000","10000","11110","10001","10001","01110" };
        case '7': return std::array<std::string, 7>{ "11111","00001","00010","00100","00100","00100","00100" };
        case '8': return std::array<std::string, 7>{ "01110","10001","10001","01110","10001","10001","01110" };
        case '9': return std::array<std::string, 7>{ "01110","10001","10001","01111","00001","00010","01100" };
        case ':': return std::array<std::string, 7>{ "00000","00100","00100","00000","00100","00100","00000" };
        case '/': return std::array<std::string, 7>{ "00001","00001","00010","00100","01000","10000","10000" };
        case ' ': return std::array<std::string, 7>{ "00000","00000","00000","00000","00000","00000","00000" };
        default:  return std::array<std::string, 7>{ "00000","00000","00000","00000","00000","00000","00000" };
        }
    };

    auto drawText = [&](const std::string& text, float startX, float startY, float px, glm::vec3 tColor, float alpha) {
        float gap = 2.0f;
        float charW = 5.0f * px;
        for (size_t i = 0; i < text.size(); i++) {
            auto rows = glyphRows(text[i]);
            float cxBase = startX + i * (charW + 2.0f * px);
            for (int r = 0; r < 7; r++) {
                for (int c = 0; c < 5; c++) {
                    if (rows[r][c] == '1') {
                        float cxp = cxBase + c * (px + gap);
                        float cyp = startY + (6 - r) * (px + gap);
                        drawRect(tColor, alpha, cxp + px * 0.5f, cyp + px * 0.5f, px, px);
                    }
                }
            }
        }
        return text.size() * (charW + 2.0f * px); // return total width
    };

    // ===== GAME OVER SCREEN =====
    if (gameOver) {
        // Dark screen overlay
        drawRect(glm::vec3(0.0f), 0.75f, SCR_WIDTH * 0.5f, SCR_HEIGHT * 0.5f, SCR_WIDTH, SCR_HEIGHT);

        // Text settings
        std::string t1 = "GAME OVER";
        std::string t2 = "SCORE: " + std::to_string(score);
        std::string t3 = "PRESS R TO RESTART";

        float px1 = 8.0f; // Big font for GAME OVER
        float px2 = 6.0f; // Medium font for Score
        float px3 = 4.0f; // Small font for Restart

        float w1 = t1.size() * (5.0f * px1 + 2.0f * px1);
        float w2 = t2.size() * (5.0f * px2 + 2.0f * px2);
        float w3 = t3.size() * (5.0f * px3 + 2.0f * px3);

        float cx = SCR_WIDTH * 0.5f;

        drawText(t1, cx - w1 * 0.5f, SCR_HEIGHT * 0.5f + 80.0f, px1, glm::vec3(1.0f, 0.2f, 0.2f), 1.0f);
        drawText(t2, cx - w2 * 0.5f, SCR_HEIGHT * 0.5f,        px2, glm::vec3(1.0f, 0.8f, 0.2f), 1.0f);
        drawText(t3, cx - w3 * 0.5f, SCR_HEIGHT * 0.5f - 80.0f, px3, glm::vec3(0.8f, 0.8f, 0.8f), 1.0f);
    }
    // ===== MODE SWITCH TOAST (bottom-centre, short duration) =====
    else if (modeToastTimer > 0.0f && !modeToastText.empty()) {
        float alpha = glm::clamp(modeToastTimer / modeToastDuration, 0.0f, 1.0f);
        float px = 3.0f;
        // Per-char advance inside drawText: charW(=5*px) + 2*px gap = 7*px
        float charAdv = 7.0f * px;
        float totalW  = modeToastText.size() * charAdv - 2.0f * px; // subtract trailing gap
        float glyphH  = 6.0f * (px + 2.0f) + px;                    // 7 rows × (px+gap) – 1 gap

        // Centre horizontally, sit near the bottom
        float startX = (SCR_WIDTH  - totalW)  * 0.5f;
        float startY = 28.0f;                         // bottom of glyph baseline

        // Pill-shaped background — generous padding
        float boxW = totalW + 40.0f;
        float boxH = glyphH + 18.0f;
        float boxCx = SCR_WIDTH  * 0.5f;
        float boxCy = startY + glyphH * 0.5f + 4.0f;

        // Outer glow ring
        drawRect(glm::vec3(0.20f, 0.75f, 1.0f),  0.18f * alpha, boxCx, boxCy, boxW + 8.0f,  boxH + 8.0f);
        // Dark fill
        drawRect(glm::vec3(0.04f, 0.06f, 0.12f), 0.82f * alpha, boxCx, boxCy, boxW,          boxH);
        // Thin top accent line
        drawRect(glm::vec3(0.20f, 0.75f, 1.0f),  0.80f * alpha, boxCx, boxCy + boxH * 0.5f - 1.5f, boxW, 3.0f);

        // Choose text colour: cyan-white for most messages, gold for shading/texture changes
        bool isWarm = (modeToastText.find("GOURAUD") != std::string::npos ||
                       modeToastText.find("PHONG")   != std::string::npos ||
                       modeToastText.find("TEXTURE") != std::string::npos ||
                       modeToastText.find("NIGHT")   != std::string::npos);
        glm::vec3 toastColor = isWarm ? glm::vec3(1.0f, 0.88f, 0.30f)
                                      : glm::vec3(0.30f, 0.92f, 1.0f);

        drawText(modeToastText, startX, startY, px, toastColor, 0.98f * alpha);
    }

    glDisable(GL_BLEND);
    if (depthWasOn) glEnable(GL_DEPTH_TEST);
}

void updateScenarioTour(float dt)
{
    if (!scenarioTourActive || gameOver) return;

    if (!scenarioTourInitialized) {
        scenarioTourInitialized = true;
        scenarioTourTimer = 0.0f;
        splitView = false;
        balloonsFrozen = true;
        balloons.clear();
        projectiles.clear();
        // Spawn fixed balloons for shooting demos
        balloons.emplace_back(glm::vec3(-8.0f, 12.0f, -38.0f), 0.0f, glm::vec3(1.0f, 0.2f, 0.25f), 1.0f);
        balloons.emplace_back(glm::vec3(0.0f, 14.0f, -45.0f),  0.0f, glm::vec3(0.2f, 0.7f, 1.0f),  1.0f);
        balloons.emplace_back(glm::vec3(10.0f, 11.0f, -52.0f), 0.0f, glm::vec3(1.0f, 0.7f, 0.2f),  1.0f);
        balloons.emplace_back(glm::vec3(-4.0f, 16.0f, -58.0f), 0.0f, glm::vec3(0.8f, 0.3f, 1.0f),  1.0f);
        balloons.emplace_back(glm::vec3(6.0f, 13.0f, -64.0f),  0.0f, glm::vec3(0.4f, 1.0f, 0.4f),  1.0f);
        // Reset all toggles to clean state
        setDayMode();
        isRaining = false;
        texturesEnabled = true;
        vertexShaderMode = false;
        ambientToggle = true;  diffuseToggle = true;  specularToggle = true;
        dirLight.turnAmbientOn(); dirLight.turnDiffuseOn(); dirLight.turnSpecularOn();
        for (auto& pl : pointLights) { pl.turnAmbientOn(); pl.turnDiffuseOn(); pl.turnSpecularOn(); }
        spotLight.turnAmbientOn(); spotLight.turnDiffuseOn(); spotLight.turnSpecularOn();
        setFireMode(FireMode::Bullet, true);
        showToast("TOUR STARTED");
    }

    scenarioTourTimer += dt;
    float t = scenarioTourTimer;

    auto setCamera = [](glm::vec3 eye, glm::vec3 target) {
        basic_camera.eye = eye;
        basic_camera.Front = glm::normalize(target - eye);
        basic_camera.Right = glm::normalize(glm::cross(basic_camera.Front, basic_camera.WorldUp));
        basic_camera.Up    = glm::normalize(glm::cross(basic_camera.Right, basic_camera.Front));
    };
    auto sample = [&](const std::array<glm::vec3, 4>& cps, float u) {
        return catmullRom(cps[0], cps[1], cps[2], cps[3], easeInOut(u));
    };
    // Helper: fire only once per N seconds
    static float lastTourShot = -10.0f;

    // ────────────────────────────────────────────────────
    // 0–12s  SCENE 1 · Wide cinematic island overview (DAY)
    // ────────────────────────────────────────────────────
    if (t < 12.0f) {
        float u = t / 12.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(-75,28,20), glm::vec3(-55,24,-10), glm::vec3(35,22,-70), glm::vec3(75,26,-25) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(-15,3,-35), glm::vec3(0,4,-40), glm::vec3(8,4,-48), glm::vec3(0,4,-42) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        if (t < 0.5f) showToast("DAY MODE");
    }
    // ────────────────────────────────────────────────────
    // 12–22s  SCENE 2 · Tree / Street Lamp close-up
    // ────────────────────────────────────────────────────
    else if (t < 22.0f) {
        float u = (t-12.0f)/10.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(-42,11,-8), glm::vec3(-20,9,-20), glm::vec3(12,10,-38), glm::vec3(24,8,-58) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(-20,2,-15), glm::vec3(-6,4,-18), glm::vec3(0,7,-2), glm::vec3(12,3,-48) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        if (t < 12.5f) showToast("TREES AND LAMP");
    }
    // ────────────────────────────────────────────────────
    // 22–32s  SCENE 3 · Buildings walkthrough → NIGHT MODE
    // ────────────────────────────────────────────────────
    else if (t < 32.0f) {
        float u = (t-22.0f)/10.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(20,3,-18), glm::vec3(16,2.2f,-38), glm::vec3(15,1.9f,-57), glm::vec3(16,2.3f,-80) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(15,2,-28), glm::vec3(16,2,-47), glm::vec3(15,2.2f,-60), glm::vec3(16,2.2f,-86) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        if (t > 27.0f && !isNight) { setNightMode(); showToast("NIGHT MODE"); }
    }
    // ────────────────────────────────────────────────────
    // 32–44s  SCENE 4 · Shading toggle in NIGHT — Phong → Gouraud → Phong
    // ────────────────────────────────────────────────────
    else if (t < 44.0f) {
        float u=(t-32.0f)/12.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(16,2.3f,-80), glm::vec3(10,8,-60), glm::vec3(-2,10,-40), glm::vec3(-6,10,-20) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(16,2.2f,-86), glm::vec3(0,5,-50), glm::vec3(0,5,-44), glm::vec3(0,5,-36) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        if (t < 32.5f) showToast("PHONG SHADING");
        if (t > 34.5f && t < 34.7f && !vertexShaderMode) { vertexShaderMode=true;  showToast("SHADING: GOURAUD"); }
        if (t > 40.5f && t < 40.7f &&  vertexShaderMode) { vertexShaderMode=false; showToast("SHADING: PHONG");   }
    }
    // ────────────────────────────────────────────────────
    // 44–54s  SCENE 5 · Night lighting showcase (point/spot toggle)
    // ────────────────────────────────────────────────────
    else if (t < 54.0f) {
        float u=(t-44.0f)/10.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(-6,10,-20), glm::vec3(-8,16,20), glm::vec3(-2,14,16), glm::vec3(12,14,8) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(0,5,-36), glm::vec3(0,4,-40), glm::vec3(0,4,-42), glm::vec3(2,4,-48) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        if (t > 46.0f && t < 46.2f && pointLight0On)  { pointLight0On=false; showToast("TOWER LIGHT: OFF"); }
        if (t > 48.5f && t < 48.7f && !pointLight0On) { pointLight0On=true;  showToast("TOWER LIGHT: ON"); }
        if (t > 50.5f && t < 50.7f && !spotLightOn)   { spotLightOn=true;  spotLight.turnOn();  showToast("FLASHLIGHT: ON"); }
        if (t > 52.5f && t < 52.7f &&  spotLightOn)   { spotLightOn=false; spotLight.turnOff(); showToast("FLASHLIGHT: OFF"); }
    }
    // ────────────────────────────────────────────────────
    // 54–64s  SCENE 6 · RAIN storm demo
    // ────────────────────────────────────────────────────
    else if (t < 64.0f) {
        float u=(t-54.0f)/10.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(30,18,10), glm::vec3(20,16,0), glm::vec3(5,14,-10), glm::vec3(-8,13,-5) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(0,4,-30), glm::vec3(0,4,-35), glm::vec3(0,4,-40), glm::vec3(0,4,-42) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        if (t > 54.5f && t < 54.7f && !isRaining) { isRaining=true;  showToast("RAIN: ON"); }
        if (t > 61.5f && t < 61.7f &&  isRaining) { isRaining=false; showToast("RAIN: OFF"); }
    }
    // ────────────────────────────────────────────────────
    // 64–70s  SCENE 7 · DAY MODE restore
    // ────────────────────────────────────────────────────
    else if (t < 70.0f) {
        float u=(t-64.0f)/6.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(-8,13,-5), glm::vec3(-5,15,5), glm::vec3(5,20,10), glm::vec3(15,22,18) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(0,4,-42), glm::vec3(0,4,-40), glm::vec3(0,4,-35), glm::vec3(0,4,-30) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        if (t > 64.5f && isNight) { setDayMode(); showToast("DAY MODE"); }
    }
    // ────────────────────────────────────────────────────
    // 70–80s  SCENE 8 · Texture toggle demo
    // ────────────────────────────────────────────────────
    else if (t < 80.0f) {
        float u=(t-70.0f)/10.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(15,22,18), glm::vec3(10,18,12), glm::vec3(2,14,6), glm::vec3(-5,12,2) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(0,4,-30), glm::vec3(0,4,-36), glm::vec3(0,4,-40), glm::vec3(0,4,-44) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        if (t > 71.5f && t < 71.7f &&  texturesEnabled) { texturesEnabled=false; showToast("TEXTURES: OFF"); }
        if (t > 75.5f && t < 75.7f && !texturesEnabled) { texturesEnabled=true;  showToast("TEXTURES: ON");  }
    }
    // ────────────────────────────────────────────────────
    // 80–90s  SCENE 9 · Ambient / Diffuse / Specular toggle demo
    // ────────────────────────────────────────────────────
    else if (t < 90.0f) {
        float u=(t-80.0f)/10.0f;
        std::array<glm::vec3,4> eyeCP { glm::vec3(-5,12,2), glm::vec3(-4,10,-10), glm::vec3(4,10,-22), glm::vec3(2,10,-14) };
        std::array<glm::vec3,4> lookCP{ glm::vec3(0,5,-36), glm::vec3(0,5,-40), glm::vec3(0,5,-38), glm::vec3(0,5,-32) };
        setCamera(sample(eyeCP,u), sample(lookCP,u));
        // Ambient off → on
        if (t > 81.0f && t < 81.2f && ambientToggle) {
            ambientToggle=false; dirLight.turnAmbientOff();
            for (auto& pl: pointLights) pl.turnAmbientOff(); spotLight.turnAmbientOff();
            showToast("AMBIENT: OFF");
        }
        if (t > 82.5f && t < 82.7f && !ambientToggle) {
            ambientToggle=true; dirLight.turnAmbientOn();
            for (auto& pl: pointLights) pl.turnAmbientOn(); spotLight.turnAmbientOn();
            showToast("AMBIENT: ON");
        }
        // Diffuse off → on
        if (t > 84.0f && t < 84.2f && diffuseToggle) {
            diffuseToggle=false; dirLight.turnDiffuseOff();
            for (auto& pl: pointLights) pl.turnDiffuseOff(); spotLight.turnDiffuseOff();
            showToast("DIFFUSE: OFF");
        }
        if (t > 85.5f && t < 85.7f && !diffuseToggle) {
            diffuseToggle=true; dirLight.turnDiffuseOn();
            for (auto& pl: pointLights) pl.turnDiffuseOn(); spotLight.turnDiffuseOn();
            showToast("DIFFUSE: ON");
        }
        // Specular off → on
        if (t > 87.0f && t < 87.2f && specularToggle) {
            specularToggle=false; dirLight.turnSpecularOff();
            for (auto& pl: pointLights) pl.turnSpecularOff(); spotLight.turnSpecularOff();
            showToast("SPECULAR: OFF");
        }
        if (t > 88.5f && t < 88.7f && !specularToggle) {
            specularToggle=true; dirLight.turnSpecularOn();
            for (auto& pl: pointLights) pl.turnSpecularOn(); spotLight.turnSpecularOn();
            showToast("SPECULAR: ON");
        }
    }
    // ────────────────────────────────────────────────────
    // 90s+  DONE — restore everything
    // ────────────────────────────────────────────────────
    else {
        isRaining = false;
        texturesEnabled = true;
        vertexShaderMode = false;
        ambientToggle = true; diffuseToggle = true; specularToggle = true;
        dirLight.turnAmbientOn(); dirLight.turnDiffuseOn(); dirLight.turnSpecularOn();
        for (auto& pl: pointLights) { pl.turnAmbientOn(); pl.turnDiffuseOn(); pl.turnSpecularOn(); }
        spotLight.turnAmbientOn(); spotLight.turnDiffuseOn(); spotLight.turnSpecularOn();
        scenarioTourActive = false;
        scenarioTourInitialized = false;
        splitView = false;
        balloonsFrozen = false;
        setDayMode();
        setFireMode(FireMode::Bullet, true);
        showToast("TOUR: COMPLETE");
    }
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
//#p

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (scenarioTourActive) {
        return;
    }

    float speedMultiplier = cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        basic_camera.move(FORWARD, deltaTime * speedMultiplier);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        basic_camera.move(BACKWARD, deltaTime * speedMultiplier);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        basic_camera.move(LEFT, deltaTime * speedMultiplier);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        basic_camera.move(RIGHT, deltaTime * speedMultiplier);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (rotateAxis_X) rotateAngle_X -= 0.1;
        else if (rotateAxis_Y) rotateAngle_Y -= 0.1;
        else rotateAngle_Z -= 0.1;
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) translate_Y += 0.001;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) translate_Y -= 0.001;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) translate_X += 0.001;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) translate_X -= 0.001;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) translate_Z += 0.001;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) translate_Z -= 0.001;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) scale_X += 0.001;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) scale_X -= 0.001;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) scale_Y += 0.001;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) scale_Y -= 0.001;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) scale_Z += 0.001;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) scale_Z -= 0.001;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        rotateAngle_X += 0.1;
        rotateAxis_X = 1.0;
        rotateAxis_Y = 0.0;
        rotateAxis_Z = 0.0;
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        rotateAngle_Y += 0.1;
        rotateAxis_X = 0.0;
        rotateAxis_Y = 1.0;
        rotateAxis_Z = 0.0;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        rotateAngle_Z += 0.1;
        rotateAxis_X = 0.0;
        rotateAxis_Y = 0.0;
        rotateAxis_Z = 1.0;
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        eyeX += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        eyeX -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        eyeZ += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        eyeZ -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        eyeY += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        eyeY -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }

    //practiceprocess
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
		balloonColor = (balloonColor + 1) % 3;
    }
    
    // Gun-style fire for both modes (Space key)
    if (!gameOver && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        float currentTime = static_cast<float>(glfwGetTime());
        if (fireMode == FireMode::Bullet) {
            if (!bulletReloading && bulletAmmo > 0 && currentTime - lastShootTime >= shootCooldown) {
                fireBullet();
                lastShootTime = currentTime;
            }
        } else {
            float arrowCooldown = 0.5f;
            if (!arrowReloading && arrowAmmo > 0 && currentTime - lastShootTime >= arrowCooldown) {
                fireArrow();
                lastShootTime = currentTime;
            }
        }
    }

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        scenarioTourActive = !scenarioTourActive;
        scenarioTourTimer = 0.0f;
        scenarioTourInitialized = false;
        splitView = false;
        if (scenarioTourActive) {
            showToast("TOUR: STARTED");
        } else {
            balloonsFrozen = false;
            showToast("TOUR: STOPPED");
        }
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL))
    {
        setFireMode((fireMode == FireMode::Bullet) ? FireMode::Arrow : FireMode::Bullet, true);
        arrowJustFired = false;
        arrowRecoilTimer = 0.0f;
        std::cout << "Fire Mode: " << (fireMode == FireMode::Bullet ? "BULLET" : "ARROW") << std::endl;
        return;
    }

    // Spacebar fire is handled in processInput() for both modes

    // +/- : Adjust camera movement speed
    if ((key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) && action == GLFW_PRESS)
    {
        cameraSpeed = glm::min(cameraSpeed + 1.0f, 20.0f);
        showToast("SPEED: " + std::to_string((int)cameraSpeed));
    }
    if ((key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) && action == GLFW_PRESS)
    {
        cameraSpeed = glm::max(cameraSpeed - 1.0f, 1.0f);
        showToast("SPEED: " + std::to_string((int)cameraSpeed));
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS && fireMode == FireMode::Arrow && !gameOver)
    {
        if (!arrowReloading && arrowAmmo < maxArrowAmmo) {
            arrowReloading = true;
            arrowReloadTimer = 0.0f;
        }
    }

    // 4: Freeze/Unfreeze Balloons
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        balloonsFrozen = !balloonsFrozen;
        showToast(balloonsFrozen ? "BALLOONS: FROZEN" : "BALLOONS: MOVING");
    }

    // 8: Toggle Phong / Gouraud Shading
    if (key == GLFW_KEY_8 && action == GLFW_PRESS)
    {
        vertexShaderMode = !vertexShaderMode;
        showToast(vertexShaderMode ? "SHADING: GOURAUD" : "SHADING: PHONG");
    }
    
    // 9: Toggle Textures
    if (key == GLFW_KEY_9 && action == GLFW_PRESS)
    {
        texturesEnabled = !texturesEnabled;
        showToast(texturesEnabled ? "TEXTURES: ON" : "TEXTURES: OFF");
    }

    // 1: Directional Light Toggle
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        dirLightOn = !dirLightOn;
        if (dirLightOn) dirLight.turnOn();
        else dirLight.turnOff();
        showToast(dirLightOn ? "LIGHT: SUN/MOON ON" : "LIGHT: SUN/MOON OFF");
    }

    // 2: Point Light 0 (Tower) Toggle
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        pointLight0On = !pointLight0On;
        showToast(pointLight0On ? "TOWER LIGHT: ON" : "TOWER LIGHT: OFF");
    }

    // K: Point Light 1 (Tree) Toggle
    if (key == GLFW_KEY_K && action == GLFW_PRESS)
    {
        pointLight1On = !pointLight1On;
        showToast(pointLight1On ? "TREE LIGHT: ON" : "TREE LIGHT: OFF");
    }

    // 3: Spot Light Toggle
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        spotLightOn = !spotLightOn;
        if (spotLightOn) spotLight.turnOn();
        else spotLight.turnOff();
        showToast(spotLightOn ? "FLASHLIGHT: ON" : "FLASHLIGHT: OFF");
    }
    
    // V: Split View Toggle
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        splitView = !splitView;
    }
    
    // 5: Ambient Toggle
    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        ambientToggle = !ambientToggle;
        if (ambientToggle) {
            dirLight.turnAmbientOn();
            spotLight.turnAmbientOn();
            for (unsigned int i = 0; i < pointLights.size(); i++) pointLights[i].turnAmbientOn();
            showToast("AMBIENT LIGHT: ON");
        } else {
            dirLight.turnAmbientOff();
            spotLight.turnAmbientOff();
            for (unsigned int i = 0; i < pointLights.size(); i++) pointLights[i].turnAmbientOff();
            showToast("AMBIENT LIGHT: OFF");
        }
    }
    
    // 6: Diffuse Toggle
    if (key == GLFW_KEY_6 && action == GLFW_PRESS)
    {
        diffuseToggle = !diffuseToggle;
        if (diffuseToggle) {
            dirLight.turnDiffuseOn();
            spotLight.turnDiffuseOn();
            for (unsigned int i = 0; i < pointLights.size(); i++) pointLights[i].turnDiffuseOn();
            showToast("DIFFUSE LIGHT: ON");
        } else {
            dirLight.turnDiffuseOff();
            spotLight.turnDiffuseOff();
            for (unsigned int i = 0; i < pointLights.size(); i++) pointLights[i].turnDiffuseOff();
            showToast("DIFFUSE LIGHT: OFF");
        }
    }
    
     // 7: Specular Toggle
    if (key == GLFW_KEY_7 && action == GLFW_PRESS)
    {
        specularToggle = !specularToggle;
        if (specularToggle) {
            dirLight.turnSpecularOn();
            spotLight.turnSpecularOn();
            for (unsigned int i = 0; i < pointLights.size(); i++) pointLights[i].turnSpecularOn();
            showToast("SPECULAR LIGHT: ON");
        } else {
            dirLight.turnSpecularOff();
            spotLight.turnSpecularOff();
            for (unsigned int i = 0; i < pointLights.size(); i++) pointLights[i].turnSpecularOff();
            showToast("SPECULAR LIGHT: OFF");
        }
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        cursorVisible = !cursorVisible;
        if (cursorVisible)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }
    
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        if (isNight) {
            setDayMode();
            showToast("DAY MODE");
        } else {
            setNightMode();
            showToast("NIGHT MODE");
        }
    }

    if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
    {
        isRaining = !isRaining;
        thunderFlashTimer = 0.0f;
        showToast(isRaining ? "RAIN: ON" : "RAIN: OFF");
    }

    // R: Restart Game
    if (key == GLFW_KEY_R && action == GLFW_PRESS && gameOver)
    {
        gameOver = false;
        score = 0;
        balloonSpawnTimer = 0.0f;
        bulletAmmo = maxBulletAmmo;
        arrowAmmo = maxArrowAmmo;
        bulletReloading = false;
        arrowReloading = false;
        arrowJustFired = false;
        arrowRecoilTimer = 0.0f;
        gunJustFired = false;
        gunRecoilTimer = 0.0f;
        balloonsFrozen = false;
        balloons.clear();
        projectiles.clear();
        balloons.emplace_back(glm::vec3(5.0f, 15.0f, -5.0f), 0.8f);
        balloons.emplace_back(glm::vec3(-5.0f, 18.0f, -5.0f), 0.6f);
        std::cout << "======= GAME RESTARTED! =======" << std::endl;
    }

    // Toggle Textures (All features)
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        texturesEnabled = !texturesEnabled;
        std::cout << "Textures Enabled: " << (texturesEnabled ? "ON" : "OFF") << std::endl;
    }

    // Toggle Procedural Balloon gradient (R4 parameter)
    if (key == GLFW_KEY_U && action == GLFW_PRESS)
    {
        proceduralBalloon = !proceduralBalloon;
        std::cout << "Procedural Balloon Shading: " << (proceduralBalloon ? "ON" : "OFF") << std::endl;
    }

    // Toggle Sky mode: Texture(R1) vs Vertex Colors(R3)
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        vertexShaderMode = !vertexShaderMode;
        std::cout << "Sky Mode: " << (vertexShaderMode ? "Vertex Color (R3)" : "Texture (R1)") << std::endl;
    }

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Right click fires arrow in Arrow mode (alternative to Space)
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && fireMode == FireMode::Arrow && !gameOver) {
        float currentTime = static_cast<float>(glfwGetTime());
        if (!arrowReloading && arrowAmmo > 0 && currentTime - lastShootTime >= shootCooldown) {
            fireArrow();
            lastShootTime = currentTime;
        }
    }

    // Existing left-click ray-picking code
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && cursorVisible)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // 1. NDC
        float x = (2.0f * xpos) / SCR_WIDTH - 1.0f;
        float y = 1.0f - (2.0f * ypos) / SCR_HEIGHT;
        float z = 1.0f;
        glm::vec3 ray_nds = glm::vec3(x, y, z);

        // 2. Homogeneous Clip Coordinates
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

        // 3. Eye Coordinates
        glm::mat4 projection = glm::perspective(glm::radians(basic_camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // 4. World Coordinates
        glm::mat4 view = basic_camera.createViewMatrix();
        glm::vec3 ray_wor = (glm::inverse(view) * ray_eye);
        ray_wor = glm::normalize(ray_wor);

        // 5. Ray-Plane Intersection (Ground at y = -1.5)
        // Ray Origin = Camera Position
        glm::vec3 ray_origin = basic_camera.eye;
        glm::vec3 plane_normal = glm::vec3(0.0f, 1.0f, 0.0f);
        float plane_y = -1.5f;

        float denom = glm::dot(plane_normal, ray_wor);
        if (abs(denom) > 1e-6) {
            glm::vec3 p0 = glm::vec3(0.0f, plane_y, 0.0f);
            float t = glm::dot(p0 - ray_origin, plane_normal) / denom;
            if (t >= 0) {
                glm::vec3 intersection = ray_origin + t * ray_wor;
                std::cout << "Clicked Ground at: (" << intersection.x << ", " << intersection.y << ", " << intersection.z << ")" << std::endl;
            }
        }
    }
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (cursorVisible) return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;


    basic_camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    basic_camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
