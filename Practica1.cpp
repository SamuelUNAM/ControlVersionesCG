#include <stdio.h>
#include <string.h>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>

#include <glew.h>
#include <glfw3.h>

const int WIDTH = 800, HEIGHT = 800;

GLuint VAO, VBO, shader;

// =====================================================
// SHADERS
// =====================================================

static const char* vShader = "                         \n\
#version 330                                             \n\
layout (location = 0) in vec3 pos;                      \n\
void main()                                              \n\
{                                                        \n\
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f);       \n\
}";


// Todas las letras serán blancas
static const char* fShader = "                         \n\
#version 330                                             \n\
out vec4 color;                                          \n\
void main()                                              \n\
{                                                        \n\
    color = vec4(1.0f, 1.0f, 1.0f, 1.0f);               \n\
}";


// =====================================================
// FUNCIONES PARA CONSTRUIR LAS LETRAS CON TRIÁNGULOS
// =====================================================

// Agrega un vértice al arreglo
void AgregarVertice(
    std::vector<GLfloat>& vertices,
    float x,
    float y)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(0.0f);
}


// Crea una línea gruesa utilizando 2 triángulos
void AgregarLineaGruesa(
    std::vector<GLfloat>& vertices,
    float x1,
    float y1,
    float x2,
    float y2,
    float grosor)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    float longitud = sqrtf(dx * dx + dy * dy);

    // Vector perpendicular
    float nx = (-dy / longitud) * (grosor / 2.0f);
    float ny = (dx / longitud) * (grosor / 2.0f);

    // Cuatro esquinas de la línea
    float ax = x1 + nx;
    float ay = y1 + ny;

    float bx = x2 + nx;
    float by = y2 + ny;

    float cx = x2 - nx;
    float cy = y2 - ny;

    float dx2 = x1 - nx;
    float dy2 = y1 - ny;


    // TRIÁNGULO 1
    AgregarVertice(vertices, ax, ay);
    AgregarVertice(vertices, bx, by);
    AgregarVertice(vertices, cx, cy);

    // TRIÁNGULO 2
    AgregarVertice(vertices, ax, ay);
    AgregarVertice(vertices, cx, cy);
    AgregarVertice(vertices, dx2, dy2);
}


// =====================================================
// CREAR LAS INICIALES S M C
// =====================================================

int CrearIniciales()
{
    std::vector<GLfloat> vertices;

    float grosor = 0.09f;


    // =================================================
    // LETRA S
    // =================================================

    // Parte superior
    AgregarLineaGruesa(
        vertices,
        -0.90f, 0.40f,
        -0.45f, 0.40f,
        grosor
    );

    // Lado superior izquierdo
    AgregarLineaGruesa(
        vertices,
        -0.90f, 0.40f,
        -0.90f, 0.05f,
        grosor
    );

    // Parte central
    AgregarLineaGruesa(
        vertices,
        -0.90f, 0.05f,
        -0.45f, 0.05f,
        grosor
    );

    // Lado inferior derecho
    AgregarLineaGruesa(
        vertices,
        -0.45f, 0.05f,
        -0.45f, -0.35f,
        grosor
    );

    // Parte inferior
    AgregarLineaGruesa(
        vertices,
        -0.45f, -0.35f,
        -0.90f, -0.35f,
        grosor
    );


    // =================================================
    // LETRA M
    // =================================================

    // Línea izquierda
    AgregarLineaGruesa(
        vertices,
        -0.25f, -0.35f,
        -0.25f, 0.40f,
        grosor
    );

    // Diagonal izquierda
    AgregarLineaGruesa(
        vertices,
        -0.25f, 0.40f,
        0.00f, 0.05f,
        grosor
    );

    // Diagonal derecha
    AgregarLineaGruesa(
        vertices,
        0.00f, 0.05f,
        0.25f, 0.40f,
        grosor
    );

    // Línea derecha
    AgregarLineaGruesa(
        vertices,
        0.25f, 0.40f,
        0.25f, -0.35f,
        grosor
    );


    // =================================================
    // LETRA C
    // =================================================

    // Parte superior
    AgregarLineaGruesa(
        vertices,
        0.45f, 0.40f,
        0.90f, 0.40f,
        grosor
    );

    // Lado izquierdo
    AgregarLineaGruesa(
        vertices,
        0.45f, 0.40f,
        0.45f, -0.35f,
        grosor
    );

    // Parte inferior
    AgregarLineaGruesa(
        vertices,
        0.45f, -0.35f,
        0.90f, -0.35f,
        grosor
    );


    // =================================================
    // ENVIAR LOS VÉRTICES A OPENGL
    // =================================================

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(GLfloat),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(GLfloat),
        (GLvoid*)0
    );

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // Número total de vértices
    return (int)(vertices.size() / 3);
}


// =====================================================
// AGREGAR SHADER
// =====================================================

void AddShader(
    GLuint theProgram,
    const char* shaderCode,
    GLenum shaderType)
{
    GLuint theShader = glCreateShader(shaderType);

    const GLchar* theCode[1];
    theCode[0] = shaderCode;

    GLint codeLength[1];
    codeLength[0] = (GLint)strlen(shaderCode);

    glShaderSource(
        theShader,
        1,
        theCode,
        codeLength
    );

    glCompileShader(theShader);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glGetShaderiv(
        theShader,
        GL_COMPILE_STATUS,
        &result
    );

    if (!result)
    {
        glGetShaderInfoLog(
            theShader,
            sizeof(eLog),
            NULL,
            eLog
        );

        printf(
            "Error al compilar shader %d: %s\n",
            shaderType,
            eLog
        );

        return;
    }

    glAttachShader(theProgram, theShader);
    glDeleteShader(theShader);
}


// =====================================================
// COMPILAR SHADERS
// =====================================================

void CompileShaders()
{
    shader = glCreateProgram();

    if (!shader)
    {
        printf("Error creando programa de shader");
        return;
    }

    AddShader(
        shader,
        vShader,
        GL_VERTEX_SHADER
    );

    AddShader(
        shader,
        fShader,
        GL_FRAGMENT_SHADER
    );

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glLinkProgram(shader);

    glGetProgramiv(
        shader,
        GL_LINK_STATUS,
        &result
    );

    if (!result)
    {
        glGetProgramInfoLog(
            shader,
            sizeof(eLog),
            NULL,
            eLog
        );

        printf(
            "Error al linkear: %s\n",
            eLog
        );

        return;
    }

    glValidateProgram(shader);

    glGetProgramiv(
        shader,
        GL_VALIDATE_STATUS,
        &result
    );

    if (!result)
    {
        glGetProgramInfoLog(
            shader,
            sizeof(eLog),
            NULL,
            eLog
        );

        printf(
            "Error al validar: %s\n",
            eLog
        );

        return;
    }
}


// =====================================================
// MAIN
// =====================================================

int main()
{
    // Inicializar GLFW
    if (!glfwInit())
    {
        printf("Fallo al inicializar GLFW");
        glfwTerminate();
        return 1;
    }


    // Configuración de OpenGL
    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        4
    );

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        3
    );

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    glfwWindowHint(
        GLFW_OPENGL_FORWARD_COMPAT,
        GL_TRUE
    );


    // Crear ventana
    GLFWwindow* mainWindow = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "Iniciales SMC",
        NULL,
        NULL
    );

    if (!mainWindow)
    {
        printf("Fallo al crear la ventana");

        glfwTerminate();
        return 1;
    }


    // Obtener tamaño del framebuffer
    int BufferWidth, BufferHeight;

    glfwGetFramebufferSize(
        mainWindow,
        &BufferWidth,
        &BufferHeight
    );


    glfwMakeContextCurrent(mainWindow);

    glewExperimental = GL_TRUE;


    if (glewInit() != GLEW_OK)
    {
        printf("Fallo al inicializar GLEW");

        glfwDestroyWindow(mainWindow);
        glfwTerminate();

        return 1;
    }


    glViewport(
        0,
        0,
        BufferWidth,
        BufferHeight
    );


    // =================================================
    // CREAR INICIALES Y SHADERS
    // =================================================

    int numeroVertices = CrearIniciales();

    CompileShaders();


    // =================================================
    // GENERADOR DE COLORES ALEATORIOS
    // =================================================

    /*
        Se utiliza una semilla basada en:

        1. random_device
        2. reloj de alta precisión

        Esto ayuda a que cada ejecución tenga
        una secuencia diferente de colores.
    */

    unsigned int semillaTiempo =
        (unsigned int)
        std::chrono::high_resolution_clock::
        now().time_since_epoch().count();


    std::seed_seq semilla{
        std::random_device{}(),
        semillaTiempo
    };


    std::mt19937 generador(semilla);


    // Genera valores entre 0.0 y 1.0
    std::uniform_real_distribution<float>
        colorAleatorio(0.0f, 1.0f);


    // Primer color aleatorio
    float rojo = colorAleatorio(generador);
    float verde = colorAleatorio(generador);
    float azul = colorAleatorio(generador);


    // Guardar el tiempo del último cambio
    double ultimoCambio = glfwGetTime();


    // =================================================
    // BUCLE PRINCIPAL
    // =================================================

    while (!glfwWindowShouldClose(mainWindow))
    {
        glfwPollEvents();


        // Tiempo actual
        double tiempoActual = glfwGetTime();


        // =============================================
        // CAMBIAR COLOR CADA 2 SEGUNDOS
        // =============================================

        if (tiempoActual - ultimoCambio >= 2.0)
        {
            rojo =
                colorAleatorio(generador);

            verde =
                colorAleatorio(generador);

            azul =
                colorAleatorio(generador);


            ultimoCambio = tiempoActual;
        }


        // =============================================
        // COLOR DE FONDO ALEATORIO RGB
        // =============================================

        glClearColor(
            rojo,
            verde,
            azul,
            1.0f
        );


        glClear(GL_COLOR_BUFFER_BIT);


        // =============================================
        // DIBUJAR S, M y C SIMULTÁNEAMENTE
        // =============================================

        glUseProgram(shader);

        glBindVertexArray(VAO);


        glDrawArrays(
            GL_TRIANGLES,
            0,
            numeroVertices
        );


        glBindVertexArray(0);

        glUseProgram(0);


        glfwSwapBuffers(mainWindow);
    }


    // =================================================
    // LIBERAR RECURSOS
    // =================================================

    glDeleteBuffers(
        1,
        &VBO
    );

    glDeleteVertexArrays(
        1,
        &VAO
    );

    glDeleteProgram(shader);

    glfwDestroyWindow(mainWindow);

    glfwTerminate();

    return 0;
}