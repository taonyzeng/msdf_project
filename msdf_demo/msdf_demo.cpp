// msdf_demo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdlib.h>
#include <stdio.h>
#include "stb_image.h"

#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <shader_m.h>

using json = nlohmann::json;

/*static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};*/

/*struct GlyphData {
    char character;
    float x, y, width, height;
    float advance;
};*/

GLuint vertex_buffer, vertex_shader;
Shader* p_shader;
GLint mvp_location, vpos_location, vtex_location;
//std::map<char, GlyphData> glyphs;
GLuint textureID, VAO;
int width, height;

/*std::map<char, GlyphData> loadGlyphData(const std::string& jsonPath) {
    std::map<char, GlyphData> glyphs;
    std::ifstream inputFile(jsonPath);
    if (inputFile.is_open()) {
        json metadata;
        inputFile >> metadata;

        for (auto& glyph : metadata["glyphs"]) {
            char character = glyph["unicode"].get<char>();
            if (glyph.contains("atlasBounds"))
            {
                GlyphData glyphData = {
                character,
                glyph["atlasBounds"]["left"],
                glyph["atlasBounds"]["bottom"],
                glyph["atlasBounds"]["right"] - glyph["atlasBounds"]["left"],
                glyph["atlasBounds"]["top"] - glyph["atlasBounds"]["bottom"],
                glyph["advance"]
                };
                glyphs[character] = glyphData;
            }

        }
    }
    return glyphs;
}*/


GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 3) ? GL_RGB : GL_RED;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}


static void startup() {

    // Load glyph data
    //glyphs = loadGlyphData("msdf_test2.json");
    // Load the texture
    textureID = loadTexture("textures/awesomeface.png");

    //p_shader = new Shader( "shaders/msdf_text.vert", "shaders/msdf_text.frag" );
    p_shader = new Shader( "shaders/msdf_text.vert", "shaders/4.2.texture.fs" );

    mvp_location = glGetUniformLocation(p_shader->ID, "MVP");
    vpos_location = glGetAttribLocation(p_shader->ID, "vPos");
    vtex_location = glGetAttribLocation(p_shader->ID, "texCoord");

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
}

// render line of text
// -------------------
/*void RenderText(std::string text, float x, float y, float scale )
{
    // activate corresponding render state
    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE0, textureID );
    glBindVertexArray(VAO);

    std::vector<float> vertices;
    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        const GlyphData& glyph = glyphs[*c];

        float tx0 = glyph.x / 1024;
        float ty0 = glyph.y / 1024;
        float tx1 = (glyph.x + glyph.width) /1024;
        float ty1 = (glyph.y + glyph.height)/1024;

        float  x0 = x / width;
        float  x1 = (x + glyph.width) / width;
        float  y0 = y / height;
        float  y1 = (y + glyph.height) / height;

        // Add vertex data for the glyph quad
        vertices.insert(vertices.end(), {
            // positions   // texture coords
            -0.5f,  0.5f,  0.0f, 1.0f,  // top left
             0.5f,  0.5f,  1.0f, 1.0f, // top right
            -0.5f, -0.5f,  0.0f, 0.0f, // bottom left
             0.5f, -0.5f,  1.0f, 0.0f, // bottom right
            });

        vertices.insert(vertices.end(), {
            //Position     //TexCoords
            tx1, ty1,      tx1, ty1,
            tx1, ty0,      tx1, ty0,
            tx0, ty1,      tx0, ty1,

            tx1, ty0,      tx1, ty0,
            tx0, ty0,      tx0, ty0,
            tx0, ty1,      tx0, ty1
            });

        x += glyph.advance * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 4, (void*)0);
    glEnableVertexAttribArray(vtex_location);
    glVertexAttribPointer(vtex_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 4, (void*)(sizeof(float) * 2));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}*/


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
/*void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}*/

/*int main()
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1024, 1024, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    startup();

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &width, &height);

        glViewport( 0, 0, width, height );
        glClear( GL_COLOR_BUFFER_BIT );

        RenderText("H", 20.0f, 20.0f, 1.0f );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}*/

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
