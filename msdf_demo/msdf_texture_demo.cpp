#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <shader_m.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <shader_m.h>

using json = nlohmann::json;

struct GlyphData {
    char character;
    float x, y, width, height;
    float advance;
    float x_offset;
    float y_offset;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

std::map<char, GlyphData> glyphs;

void loadGlyphData(const std::string& jsonPath) {

    std::ifstream inputFile(jsonPath);
    if (inputFile.is_open()) {
        json metadata;
        inputFile >> metadata;

        for (auto& glyph : metadata["glyphs"]) {
            char character = glyph["unicode"].get<char>();
            if (glyph.contains("atlasBounds") && glyph.contains("planeBounds") ){
                GlyphData glyphData = {
                character,
                glyph["atlasBounds"]["left"],
                glyph["atlasBounds"]["bottom"],
                glyph["atlasBounds"]["right"].get<float>() - glyph["atlasBounds"]["left"].get<float>(),
                glyph["atlasBounds"]["top"].get<float>() - glyph["atlasBounds"]["bottom"].get<float>(),
                glyph["advance"],
                glyph["planeBounds"]["left"],
                glyph["planeBounds"]["bottom"],
                };
                glyphs[character] = glyphData;
            }
            else {
                GlyphData data;
                data.advance = glyph["advance"];
                glyphs[character] = data;
            }

        }
    }
}


// render line of text
// -------------------
std::vector<float> generateVertexData(std::string text, float x, float y, float scale ) {

    std::vector<float> vertices;
    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        const GlyphData& glyph = glyphs[*c];

        if (( *c ) == 32) {
            x += glyph.advance * 100 * scale;
            continue;
        }

        float tx0 = glyph.x / 1024;
        float ty0 = glyph.y / 1024;
        float tx1 = (glyph.x + glyph.width) /1024;
        float ty1 = (glyph.y + glyph.height)/1024;

        float w = glyph.width * scale;
        float h = glyph.height * scale;

        float x_off = glyph.x_offset * scale * glyph.width;
        float y_off = glyph.y_offset * scale * glyph.height;

        float  x0 = x + x_off;
        float  x1 = x0 + w;
        float  y0 = y + y_off;
        float  y1 = y0 + h;

        vertices.insert(vertices.end(), {
            //Position                         //TexCoords
            x1, y1, 0.0f,  1.0f, 0.0f, 0.0f,   tx1, ty1,
            x1, y0, 0.0f,  0.0f, 1.0f, 0.0f,   tx1, ty0,
            x0, y1, 0.0f,  1.0f, 1.0f, 0.0f,   tx0, ty1,

            x1, y0, 0.0f,  0.0f, 1.0f, 0.0f,   tx1, ty0,
            x0, y0, 0.0f,  0.0f, 0.0f, 1.0f,   tx0, ty0,
            x0, y1, 0.0f,  1.0f, 1.0f, 0.0f,   tx0, ty1
            });

        x += (glyph.width + glyph.advance) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    return vertices;
}


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if ( !gladLoadGL(glfwGetProcAddress) )
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("shaders/4.2.texture.vs", "shaders/msdf_text.frag");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    ourShader.use();
    glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    loadGlyphData( "textures/msdf_test2.json"  );
    std::vector<float> vertices = generateVertexData("great people!", 120.0f, 120.0f, 0.5f);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    /*float vertices[] = {
        // positions          // colors           // texture coords
         1024.0f,  1024.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         1024.0f,  0.0f,    0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
         0.0f,     1024.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f, // top left

         1024.0f,  0.0f,    0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
         0.0f,     0.0f,    0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
         0.0f,     1024.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };*/

    /*float vertices[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f, // top left

         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };*/

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // load and create a texture
    // -------------------------
    unsigned int texture1, texture2;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load("textures/msdf_test2.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 3) ? GL_RGB : GL_RED;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:
    glUniform1i(glGetUniformLocation(ourShader.ID, "msdf"), 0);
    // or set it via the texture class
    //ourShader.setInt("texture2", 1);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);

        // render container
        ourShader.use();
        glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDrawArrays( GL_TRIANGLES, 0, vertices.size() / 8 );
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}