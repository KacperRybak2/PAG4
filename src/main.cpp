#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>


#if defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#endif

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <Shader.h>
#include <stb_image.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <Camera.h>
#include <vector>
#include <string>
#include <iostream>
#include <Model.h>
#include <Object.h>
#include <chrono>

#define SIZE 20
#define AMOUNT (SIZE*SIZE)
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Object &obj);
unsigned int loadCubemap(std::vector<std::string> faces);
bool controlLocomotive = false;


const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool guiDL = true;
bool guiSL1 = true;
bool guiSL2 = true;
bool guiPL = true;

glm::vec3 guiVecDL = glm::vec3(0.0f, -1.0f, 1.0f);
glm::vec3 guiVecSL1 = glm::vec3(0.0f, -0.356f, 1.0f);
glm::vec3 guiVecSL2 = glm::vec3(0.54f, -1.0f, 0.0f);

glm::vec3 guiColDL = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 guiColSL1 = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 guiColSL2 = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 guiColPL = glm::vec3(0.0f, 1.0f, 1.0f);

glm::vec3 guiAmbDL = glm::vec3(0.1f, 0.1f, 0.1f);
glm::vec3 guiAmbSL1 = glm::vec3(0.1f, 0.1f, 0.1f);
glm::vec3 guiAmbSL2 = glm::vec3(0.1f, 0.1f, 0.1f);
glm::vec3 guiAmbPL = glm::vec3(0.1f, 0.1f, 0.1f);

glm::vec3 guiSpecDL = glm::vec3(0.1f, 0.1f, 0.1f);
glm::vec3 guiSpecSL1 = glm::vec3(0.1f, 0.1f, 0.1f);
glm::vec3 guiSpecSL2 = glm::vec3(0.1f, 0.1f, 0.1f);
glm::vec3 guiSpecPL = glm::vec3(0.1f, 0.1f, 0.1f);

//variables used to control the locomotive
glm::vec3 Front;
float Yaw = 90.0f;
float Pitch = 0.0f;
float movingTime = 0.0f;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void setUpLights(Shader* shader, glm::vec3 PLPos, glm::vec3 SL1Pos, glm::vec3 SL2Pos);

int main(int, char**)
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 4.3 + GLSL 430
    const char* glsl_version = "#version 430";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only


    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PAG4", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();

    /////////////////////////////////////////////////////////
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    std::vector<std::string> faces
    {
        "Models/right.jpg",
        "Models/left.jpg",
        "Models/top.jpg",
        "Models/bottom.jpg",
        "Models/front.jpg",
        "Models/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    Object::cubemapTexture = cubemapTexture;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Shader modelShader("Shaders/model.vs", "Shaders/model.fs");
    Shader instanceShader("Shaders/instance.vs", "Shaders/instance.fs");
    Shader shaderRefract("Shaders/refract.vs", "Shaders/refract.fs");
    Shader shaderReflect("Shaders/reflect.vs", "Shaders/reflect.fs");
    Shader lightModelShader("Shaders/lightModel.vs", "Shaders/lightModel.fs");
    Model wall("Models/house.obj");
    Model roof("Models/roof.obj");
    Model plane("Models/plane.obj");
    Model spotLightModel("Models/spotLight2.obj");
    Model pointLightModel("Models/pointLight.obj");
    Model dirLightModel("Models/dirLight2.obj");
    Model modelLokomotive("Models/train2.obj");
    Model modelWheel("Models/wheel2.obj");
    Model modelSmallWheel("Models/smallWheel.obj");
    Model modelPiston1("Models/piston11.obj");
    Model modelPiston2("Models/piston22.obj");
    Model modelPiston3("Models/piston33.obj");
    Model modelSmoke("Models/smoke2.obj");
    
    Object wheels;
    Object objectLocomotive(&modelLokomotive, &modelShader, Other, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectWheel1(&modelWheel, &shaderReflect, UsingSkyBox, glm::vec3(0.2f, 0.2f, 0.721f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectWheel2(&modelWheel, &shaderReflect, UsingSkyBox, glm::vec3(0.2f, 0.2f, 1.25f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectWheel3(&modelWheel, &shaderReflect, UsingSkyBox, glm::vec3(0.2f, 0.2f, 1.64f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectWheel4(&modelWheel, &shaderReflect, UsingSkyBox, glm::vec3(0.2f, 0.2f, 2.15f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectWheel5(&modelWheel, &shaderReflect, UsingSkyBox, glm::vec3(-0.248f, 0.2f, 0.721f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectWheel6(&modelWheel, &shaderReflect, UsingSkyBox, glm::vec3(-0.248f, 0.2f, 1.25f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectWheel7(&modelWheel, &shaderReflect, UsingSkyBox, glm::vec3(-0.248f, 0.2f, 1.64f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectWheel8(&modelWheel, &shaderReflect, UsingSkyBox, glm::vec3(-0.248f, 0.2f, 2.15f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectPiston1(&modelPiston1, &modelShader, Other, glm::vec3(0.272f, 0.188f, 2.23f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectPiston2(&modelPiston2, &modelShader, Other, glm::vec3(0.272f, 0.19f, 2.15f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectPiston3(&modelPiston3, &modelShader, Other, glm::vec3(0.25f, 0.063f, 1.225f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectPiston4(&modelPiston1, &modelShader, Other, glm::vec3(-0.3f, 0.188f, 2.23f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectPiston5(&modelPiston2, &modelShader, Other, glm::vec3(-0.3f, 0.19f, 2.15f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectPiston6(&modelPiston3, &modelShader, Other, glm::vec3(-0.3f, 0.063f, 1.225f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.005f, 0.005f, 0.005f));
    Object objectSmoke1(&modelSmoke, &shaderRefract, UsingSkyBox, glm::vec3(0.0f, 1.4f, 1.4f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    Object objectSmoke2(&modelSmoke, &shaderRefract, UsingSkyBox, glm::vec3(0.0f, 1.9f, 1.4f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    Object objectSmoke3(&modelSmoke, &shaderRefract, UsingSkyBox, glm::vec3(0.0f, 2.4f, 1.4f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    wheels.add_child(&objectWheel1);
    wheels.add_child(&objectWheel2);
    wheels.add_child(&objectWheel3);
    wheels.add_child(&objectWheel4);
    wheels.add_child(&objectWheel5);
    wheels.add_child(&objectWheel6);
    wheels.add_child(&objectWheel7);
    wheels.add_child(&objectWheel8);
    Object root;
    root.objectType = Root;
    Object houses;
    houses.children.reserve(AMOUNT);
    root.add_child(&houses);
    root.add_child(new Object(&plane, &modelShader, Other, glm::vec3(49.0f, 0.0f, 49.0f)));
    root.add_child(&objectLocomotive);
    root.add_child(&objectSmoke1);
    root.add_child(&objectSmoke2);
    root.add_child(&objectSmoke3);
    objectLocomotive.add_child(&wheels);
    objectLocomotive.add_child(&objectPiston1);
    objectLocomotive.add_child(&objectPiston2);
    objectLocomotive.add_child(&objectPiston3);
    objectLocomotive.add_child(&objectPiston4);
    objectLocomotive.add_child(&objectPiston5);
    objectLocomotive.add_child(&objectPiston6);

    //Lights
    Object spotLightObject1(&spotLightModel, &lightModelShader, Other, glm::vec3(2.0f, 2.0f, 2.0f));
    Object spotLightObject2(&spotLightModel, &lightModelShader, Other, glm::vec3(2.0f, 4.0f, 2.0f));
    Object pointLightObject(&pointLightModel, &lightModelShader, Other, glm::vec3(4.0f, 2.0f, 4.0f), glm::vec3(0), glm::vec3(0.3f, 0.3f, 0.3f));
    Object dirLightObject(&dirLightModel, &lightModelShader, Other, glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0), glm::vec3(1.0f, 1.0f, 1.0f));


    root.add_child(&spotLightObject1);
    root.add_child(&spotLightObject2);
    root.add_child(&pointLightObject);
    root.add_child(&dirLightObject);


    for (unsigned int i = 0; i < SIZE; i++) {
        for (unsigned int j = 0; j < SIZE; j++) {
            houses.add_child(new Object(&wall, &instanceShader, Wall, glm::vec3(i * 6, 0, j * 6)));
            houses.children.at(i * SIZE + j)->add_child(new Object(&roof, &instanceShader, Roof));
        }
    }
    //houses.children.at(3)->set_position(houses.children.at(3)->position + glm::vec3(0.0f, 0.5f, 0.0f));
    //houses.set_position(0.0f, 1.0f, 0.0f);
    root.updateAllChildrenTransforms();
    houses.objectType = Houses;

    std::vector<glm::mat4> wallMatrices = root.getAllMatricesOfType(Wall);
    unsigned int instanceWallMatricesBuffer;
    glGenBuffers(1, &instanceWallMatricesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceWallMatricesBuffer);
    glBufferData(GL_ARRAY_BUFFER, AMOUNT * sizeof(glm::mat4), &wallMatrices[0], GL_STATIC_DRAW);

    for (unsigned int i = 0; i < wall.meshes.size(); i++)
    {
        glBindVertexArray(wall.meshes[i].VAO);
        std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);
    }

    std::vector<glm::mat4> roofMatrices = root.getAllMatricesOfType(Roof);
    unsigned int instanceRoofMatricesBuffer;
    glGenBuffers(1, &instanceRoofMatricesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceRoofMatricesBuffer);
    glBufferData(GL_ARRAY_BUFFER, AMOUNT * sizeof(glm::mat4), &roofMatrices[0], GL_STATIC_DRAW);

    for (unsigned int i = 0; i < roof.meshes.size(); i++)
    {
        glBindVertexArray(roof.meshes[i].VAO);
        std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);
    }

    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    float pointLightAngle = 0;
    glm::vec3 pointLightPos(0, 3, 0);
    glEnable(GL_DEPTH_TEST);
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        objectSmoke1.set_position(objectSmoke1.position.x, (objectSmoke1.position.y + 0.01f), objectSmoke1.position.z);
        objectSmoke2.set_position(objectSmoke2.position.x, objectSmoke2.position.y + 0.01f, objectSmoke2.position.z);
        objectSmoke3.set_position(objectSmoke3.position.x, objectSmoke3.position.y + 0.01f, objectSmoke3.position.z);
        if (objectSmoke1.position.y > 3.0f) {
            objectSmoke1.position.y = 1.4f;
            objectSmoke1.position.x = objectLocomotive.position.x + 1.4f * sin(glm::radians(objectLocomotive.rotation.y));
            objectSmoke1.position.z = objectLocomotive.position.z + 1.4f * cos(glm::radians(objectLocomotive.rotation.y));
        }
        if (objectSmoke2.position.y > 3.0f) {
            objectSmoke2.position.y = 1.4f;
            objectSmoke2.position.x = objectLocomotive.position.x + 1.4f * sin(glm::radians(objectLocomotive.rotation.y));
            objectSmoke2.position.z = objectLocomotive.position.z + 1.4f * cos(glm::radians(objectLocomotive.rotation.y));
        }
        if (objectSmoke3.position.y > 3.0f) {
            objectSmoke3.position.y = 1.4f;
            objectSmoke3.position.x = objectLocomotive.position.x + 1.4f * sin(glm::radians(objectLocomotive.rotation.y));
            objectSmoke3.position.z = objectLocomotive.position.z + 1.4f * cos(glm::radians(objectLocomotive.rotation.y));
        }
        {
            ImGui::Begin("Ustawienia");
            ImGui::Checkbox("Sterowanie lokomotywa", &controlLocomotive);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            if (ImGui::CollapsingHeader("Kierunkowe swiatlo"))
            {
                ImGui::Checkbox("Switch1", &guiDL);
                ImGui::SliderFloat3("Kierunek1", &guiVecDL[0], -1.0f, 1.0f);
                ImGui::ColorEdit3("Kolor1Diffuse", &guiColDL[0]);
                ImGui::ColorEdit3("Kolor1Ambient", &guiAmbDL[0]);
                ImGui::ColorEdit3("Kolor1Specular", &guiSpecDL[0]);
            }

            if (ImGui::CollapsingHeader("Reflektor1"))
            {
                ImGui::Checkbox("Switch2", &guiSL1);
                ImGui::SliderFloat3("Kierunek2", &guiVecSL1[0], -1.0f, 1.0f);
                ImGui::ColorEdit3("Kolor2Diffuse", &guiColSL1[0]);
                ImGui::ColorEdit3("Kolor2Ambient", &guiAmbSL1[0]);
                ImGui::ColorEdit3("Kolor2Specular", &guiSpecSL1[0]);
            }

            if (ImGui::CollapsingHeader("Reflektor2"))
            {
                ImGui::Checkbox("Switch3", &guiSL2);
                ImGui::SliderFloat3("Kierunek3", &guiVecSL2[0], -1.0f, 1.0f);
                ImGui::ColorEdit3("Kolor3Diffuse", &guiColSL2[0]);
                ImGui::ColorEdit3("Kolor3Ambient", &guiAmbSL2[0]);
                ImGui::ColorEdit3("Kolor3Specular", &guiSpecSL2[0]);
            }

            if (ImGui::CollapsingHeader("Punktowe swiatlo"))
            {
                ImGui::Checkbox("Switch4", &guiPL);
                ImGui::ColorEdit3("Kolor4Diffuse", &guiColPL[0]);
                ImGui::ColorEdit3("Kolor4Ambient", &guiAmbPL[0]);
                ImGui::ColorEdit3("Kolor4Specular", &guiSpecPL[0]);
            }
            ImGui::End();
        }

        /*wallMatrices = root.getAllMatricesOfType(Wall);
        glBindBuffer(GL_ARRAY_BUFFER, instanceWallMatricesBuffer);
        glBufferData(GL_ARRAY_BUFFER, AMOUNT * sizeof(glm::mat4), &wallMatrices[0], GL_STATIC_DRAW);


        roofMatrices = root.getAllMatricesOfType(Roof);
        glBindBuffer(GL_ARRAY_BUFFER, instanceRoofMatricesBuffer);
        glBufferData(GL_ARRAY_BUFFER, AMOUNT * sizeof(glm::mat4), &roofMatrices[0], GL_STATIC_DRAW);*/

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // per-frame time logic
// --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, objectLocomotive);
        
        pointLightAngle += 0.02f;
        //pointLightPos.x = (0 - SIZE / 2.0f) * cos(pointLightAngle) - (0 - SIZE / 2.0f) * sin(pointLightAngle) + SIZE / 2.0f;
        //pointLightPos.z = (0 - SIZE / 2.0f) * sin(pointLightAngle) + (0 - SIZE / 2.0f) * cos(pointLightAngle) + SIZE / 2.0f;
        pointLightPos.x = (0 - 20 / 2.0f) * cos(pointLightAngle) - (0 - 20 / 2.0f) * sin(pointLightAngle) + 20 / 2.0f;
        pointLightPos.z = (0 - 20 / 2.0f) * sin(pointLightAngle) + (0 - 20 / 2.0f) * cos(pointLightAngle) + 20 / 2.0f;
        pointLightObject.set_position(pointLightPos);

        shaderReflect.use();
        shaderReflect.setVec3("cameraPos", camera.Position);
        shaderRefract.use();
        shaderRefract.setVec3("cameraPos", camera.Position);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        


        ///////////////////////
        glm::quat orientation;
        orientation = glm::quatLookAt(-glm::normalize(guiVecSL1), glm::vec3(0.0f, 0.0f, -1.0f));
        spotLightObject1.setRotationMatrix(glm::toMat4(orientation));
        orientation = glm::quatLookAt(-glm::normalize(guiVecSL2), glm::vec3(0.0f, 0.0f, -1.0f));
        spotLightObject2.setRotationMatrix(glm::toMat4(orientation));
        orientation = glm::quatLookAt(-glm::normalize(guiVecDL), glm::vec3(0.0f, 0.0f, -1.0f));
        dirLightObject.setRotationMatrix(glm::toMat4(orientation));

        setUpLights(&instanceShader, pointLightPos, spotLightObject1.position, spotLightObject2.position);
        setUpLights(&modelShader, pointLightPos, spotLightObject1.position, spotLightObject2.position);
        
        root.updateAllChildrenTransforms();
        root.drawAllObjects(projection, view);

        wall.DrawInstanced(instanceShader, AMOUNT, projection, view);
        roof.DrawInstanced(instanceShader, AMOUNT, projection, view);




        // draw skybox as last
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setInt("skybox", 0);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);


        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


void processInput(GLFWwindow* window, Object& obj)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    /*if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        controlLocomotive = !controlLocomotive;*/
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (!controlLocomotive) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera.ProcessKeyboard(FORWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera.ProcessKeyboard(LEFT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera.ProcessKeyboard(RIGHT, deltaTime);
        }
        else {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                movingTime += deltaTime;
                obj.set_position(obj.position + Front * deltaTime);
                for (auto it = obj.children[0]->children.begin(); it < obj.children[0]->children.end(); it++) {
                    (*it)->rotate((*it)->rotation.x + 3.0f, 0.0f, 0.0f);
                }
                obj.children[1]->set_position(obj.children[1]->position.x, obj.children[1]->position.y, 2.365f + sin(-movingTime*3) * 0.135f);
                obj.children[2]->set_position(obj.children[2]->position.x, obj.children[2]->position.y, 2.21f + sin(-movingTime*3) * 0.135f);
                obj.children[2]->rotate(14 + sin(-90-movingTime * 3) * 14, 0.0f, 0.0f);
                obj.children[3]->set_position(obj.children[3]->position.x, 0.21f + sin(-90-movingTime * 3) * 0.135f, 1.28f + sin(-movingTime * 3) * 0.135f);
                obj.children[4]->set_position(obj.children[4]->position.x, obj.children[4]->position.y, 2.365f + sin(-movingTime * 3) * 0.135f);
                obj.children[5]->set_position(obj.children[5]->position.x, obj.children[5]->position.y, 2.21f + sin(-movingTime * 3) * 0.135f);
                obj.children[5]->rotate(14 + sin(-90-movingTime * 3) * 14, 0.0f, 0.0f);
                obj.children[6]->set_position(obj.children[6]->position.x, 0.21f + sin(-90 - movingTime * 3) * 0.135f, 1.28f + sin(-movingTime * 3) * 0.135f);
            }
                
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                movingTime -= deltaTime;
                obj.set_position(obj.position - Front * deltaTime);
                for (auto it = obj.children[0]->children.begin(); it < obj.children[0]->children.end(); it++) {
                    (*it)->rotate((*it)->rotation.x - 3.0f, 0.0f, 0.0f);
                }
                obj.children[1]->set_position(obj.children[1]->position.x, obj.children[1]->position.y, 2.365f + sin(-movingTime * 3) * 0.135f);
                obj.children[2]->set_position(obj.children[2]->position.x, obj.children[2]->position.y, 2.21f + sin(-movingTime * 3) * 0.135f);
                obj.children[2]->rotate(14 + sin(-90 - movingTime * 3) * 14, 0.0f, 0.0f);
                obj.children[3]->set_position(obj.children[3]->position.x, 0.21f + sin(-90 - movingTime * 3) * 0.135f, 1.28f + sin(-movingTime * 3) * 0.135f);
                obj.children[4]->set_position(obj.children[4]->position.x, obj.children[4]->position.y, 2.365f + sin(-movingTime * 3) * 0.135f);
                obj.children[5]->set_position(obj.children[5]->position.x, obj.children[5]->position.y, 2.21f + sin(-movingTime * 3) * 0.135f);
                obj.children[5]->rotate(14 + sin(-90 - movingTime * 3) * 14, 0.0f, 0.0f);
                obj.children[6]->set_position(obj.children[6]->position.x, 0.21f + sin(-90 - movingTime * 3) * 0.135f, 1.28f + sin(-movingTime * 3) * 0.135f);
            }
                
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                Yaw -= deltaTime * 20;
                glm::vec3 front;
                front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
                front.y = sin(glm::radians(Pitch));
                front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
                Front = glm::normalize(front);
                obj.rotate(obj.rotation.x, obj.rotation.y + deltaTime * 20, obj.rotation.z);
            }
                
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                Yaw += deltaTime * 20;
                glm::vec3 front;
                front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
                front.y = sin(glm::radians(Pitch));
                front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
                Front = glm::normalize(front);
                obj.rotate(obj.rotation.x, obj.rotation.y - deltaTime * 20, obj.rotation.z);
            }
        }

    }

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void setUpLights(Shader* shader, glm::vec3 PLPos, glm::vec3 SL1Pos, glm::vec3 SL2Pos) {
    shader->use();
    shader->setVec3("viewPos", camera.Position);
    shader->setFloat("shininess", 32.0f);
    // directional light
    if (guiDL) {
        shader->setVec3("dirLight.ambient", guiAmbDL);
        shader->setVec3("dirLight.diffuse", guiColDL);
        shader->setVec3("dirLight.specular", guiSpecDL);
    }
    else {
        shader->setVec3("dirLight.ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("dirLight.diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);
    }
    shader->setVec3("dirLight.direction", guiVecDL);

    // point light 1
    if (guiPL) {
        shader->setVec3("pointLights[0].ambient", guiAmbPL);
        shader->setVec3("pointLights[0].diffuse", guiColPL);
        shader->setVec3("pointLights[0].specular", guiSpecPL);
    }
    else {
        shader->setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].specular", 0.0f, 0.0f, 0.0f);
    }
    shader->setVec3("pointLights[0].position", PLPos);
    shader->setFloat("pointLights[0].constant", 1.0f);
    shader->setFloat("pointLights[0].linear", 0.09);
    shader->setFloat("pointLights[0].quadratic", 0.032);


    // spotLight 1
    if (guiSL1) {
        shader->setVec3("spotLight[0].ambient", guiAmbSL1);
        shader->setVec3("spotLight[0].diffuse", guiColSL1);
        shader->setVec3("spotLight[0].specular", guiSpecSL1);
    }
    else {
        shader->setVec3("spotLight[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLight[0].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLight[0].specular", 0.0f, 0.0f, 0.0f);
    }
    shader->setVec3("spotLight[0].position", SL1Pos);
    shader->setVec3("spotLight[0].direction", guiVecSL1);
    shader->setFloat("spotLight[0].constant", 1.0f);
    shader->setFloat("spotLight[0].linear", 0.09);
    shader->setFloat("spotLight[0].quadratic", 0.032);
    shader->setFloat("spotLight[0].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(20.0f)));


    // spotLight 2
    if (guiSL2) {
        shader->setVec3("spotLight[1].ambient", guiAmbSL2);
        shader->setVec3("spotLight[1].diffuse", guiColSL2);
        shader->setVec3("spotLight[1].specular", guiSpecSL2);
    }
    else {
        shader->setVec3("spotLight[1].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLight[1].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLight[1].specular", 0.0f, 0.0f, 0.0f);
    }
    shader->setVec3("spotLight[1].position", SL2Pos);
    shader->setVec3("spotLight[1].direction", guiVecSL2);
    shader->setFloat("spotLight[1].constant", 1.0f);
    shader->setFloat("spotLight[1].linear", 0.09);
    shader->setFloat("spotLight[1].quadratic", 0.032);
    shader->setFloat("spotLight[1].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(20.0f)));
}