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
    float pl, pb, pr, pt;
};

struct AtlasMetric {
    float fontSize;
    float width;
    float height;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

std::map<char, GlyphData> glyphs;
AtlasMetric atlas_metric;
float kerning_table[256][256];

void loadGlyphData(const std::string& jsonPath) {

    std::ifstream inputFile(jsonPath);
    if (inputFile.is_open()) {
        json metadata;
        inputFile >> metadata;

        atlas_metric.fontSize = metadata["atlas"]["size"].get<float>();
        atlas_metric.height = metadata["atlas"]["height"].get<float>();
        atlas_metric.width = metadata["atlas"]["width"].get<float>();

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
                glyph["planeBounds"]["right"],
                glyph["planeBounds"]["top"],
                };
                glyphs[character] = glyphData;
            }
            else {
                GlyphData data;
                data.advance = glyph["advance"];
                glyphs[character] = data;
            }

        }

        memset( kerning_table, 0.0f, sizeof( kerning_table ) );
        for (auto& kerning : metadata["kerning"]) {
            int unicode1 = kerning["unicode1"].get<int>();
            int unicode2 = kerning["unicode2"].get<int>();

            float kerning_val = kerning["advance"].get<float>();
            kerning_table[unicode1][unicode2] = kerning_val;
        }
    }
}


// render line of text
// -------------------
std::vector<float> generateVertexData(std::string text, float x, float y, float scale ) {

    std::vector<float> vertices;
    // iterate through all characters
    std::string::const_iterator c;
    float font_size = atlas_metric.fontSize;
    float atlas_w = atlas_metric.width;
    float atlas_h = atlas_metric.height;
    char prev_ch = 0;

    for (c = text.begin(); c != text.end(); c++) {
        const GlyphData& glyph = glyphs[*c];

        if ((*c) != 32) { //if it is not the space character
            float tx0 = glyph.x / atlas_w;
            float ty0 = glyph.y / atlas_h;
            float tx1 = (glyph.x + glyph.width) / atlas_w;
            float ty1 = (glyph.y + glyph.height) / atlas_h;

            float w = glyph.width * scale;
            float h = glyph.height * scale;

            float  x0 = x + font_size * glyph.pl * scale;
            float  x1 = x + font_size * glyph.pr * scale;
            float  y0 = y + font_size * glyph.pb * scale;
            float  y1 = y + font_size * glyph.pt * scale;

            vertices.insert(vertices.end(), {
                //Position                         //TexCoords
                x1, y1, 0.0f,  1.0f, 0.0f, 0.0f,   tx1, ty1,
                x1, y0, 0.0f,  0.0f, 1.0f, 0.0f,   tx1, ty0,
                x0, y1, 0.0f,  1.0f, 1.0f, 0.0f,   tx0, ty1,

                x1, y0, 0.0f,  0.0f, 1.0f, 0.0f,   tx1, ty0,
                x0, y0, 0.0f,  0.0f, 0.0f, 1.0f,   tx0, ty0,
                x0, y1, 0.0f,  1.0f, 1.0f, 0.0f,   tx0, ty1
            });
        }
        float kerning = kerning_table[prev_ch][*c];
        x += (font_size * (glyph.advance + kerning)) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        prev_ch = *c;
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
    Shader ourShader("shaders/4.2.texture.vs", "shaders/msdf_text_halo2.frag");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    ourShader.use();
    glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    std::string text = "lynx tuft frogs, dolphins abduct by proxy the ever awkward klutz, dud, dummkopf,"
        "jinx snubnose filmgoer, orphan sgt. ";

    loadGlyphData( "textures/msdf_test2.json"  );
    std::vector<float> vertices = generateVertexData(text, 10.0f, SCR_HEIGHT - 120, 0.8f);

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_STATIC_DRAW);

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