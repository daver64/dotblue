// Example usage of GLM in DotBlue
#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"

void ExampleGLMUsage() {
    using namespace DotBlue;
    
    // Basic vector operations
    vec3 position(1.0f, 2.0f, 3.0f);
    vec3 velocity(0.1f, 0.0f, 0.0f);
    position += velocity; // Move object
    
    // Color with GLM
    RGBA red(1.0f, 0.0f, 0.0f, 1.0f);
    vec4 colorVec = red.toVec4();
    
    // 3D transformations
    mat4 model = glm::mat4(1.0f); // Identity matrix
    model = Math::translate(model, vec3(0.0f, 0.0f, -3.0f));
    model = Math::rotate(model, Math::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
    model = Math::scale(model, vec3(2.0f, 2.0f, 2.0f));
    
    // Camera matrices
    mat4 view = Math::lookAt(
        vec3(0.0f, 0.0f, 3.0f),  // Camera position
        vec3(0.0f, 0.0f, 0.0f),  // Look at origin
        vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );
    
    mat4 projection = Math::perspective(
        Math::radians(45.0f),    // FOV
        800.0f / 600.0f,         // Aspect ratio
        0.1f,                    // Near plane
        100.0f                   // Far plane
    );
    
    // Using with shaders (example)
    /*
    shader->bind();
    shader->setMat4("u_model", model);
    shader->setMat4("u_view", view);
    shader->setMat4("u_projection", projection);
    shader->setVec3("u_color", vec3(1.0f, 0.5f, 0.2f));
    // ... draw calls
    shader->unbind();
    */
}
