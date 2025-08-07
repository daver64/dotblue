// Example usage of GLM in DotBlue
#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"

void ExampleGLMUsage() {
    using namespace DotBlue;
    
    // Basic vector operations
    Vec3 position(1.0f, 2.0f, 3.0f);
    Vec3 velocity(0.1f, 0.0f, 0.0f);
    position += velocity; // Move object
    
    // Color with GLM
    RGBA red(1.0f, 0.0f, 0.0f, 1.0f);
    Vec4 colorVec = red.toVec4();
    
    // 3D transformations
    Mat4 model = glm::mat4(1.0f); // Identity matrix
    model = Math::translate(model, Vec3(0.0f, 0.0f, -3.0f));
    model = Math::rotate(model, Math::radians(45.0f), Vec3(0.0f, 1.0f, 0.0f));
    model = Math::scale(model, Vec3(2.0f, 2.0f, 2.0f));
    
    // Camera matrices
    Mat4 view = Math::lookAt(
        Vec3(0.0f, 0.0f, 3.0f),  // Camera position
        Vec3(0.0f, 0.0f, 0.0f),  // Look at origin
        Vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );
    
    Mat4 projection = Math::perspective(
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
    shader->setVec3("u_color", Vec3(1.0f, 0.5f, 0.2f));
    // ... draw calls
    shader->unbind();
    */
}
