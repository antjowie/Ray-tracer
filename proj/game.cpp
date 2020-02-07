#include "precomp.h" // include (only) this in every .cpp file

tf::ExecutorObserver* obs = nullptr;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
    //obs = executor.make_observer<tf::ExecutorObserver>();
    renderer.Init(*screen, scene, screen->GetWidth() * screen->GetHeight(),512);

    auto box = LoadGLTF("assets/Box/glTF/Box.gltf", mat4::Translate(2,0,5));
    
    scene.Add(std::move(box));
    
    Model roof;

    // --- Make a floor
    {
        Model fmodel; Mesh fmesh;
        fmesh.mat.color = 0x404040;
        
        fmesh.faces.emplace_back();
        fmesh.faces.back()[0] = { -100.f, -1.5f, 100.f };
        fmesh.faces.back()[1] = { -100.f,  -1.5f, -100.f };
        fmesh.faces.back()[2] = { 100.f,  -1.5f, -100.f };
        fmesh.faces.emplace_back();
        fmesh.faces.back()[0] = { 100.f, -1.5f, 100.f };
        fmesh.faces.back()[1] = { -100.f,  -1.5f, 100.f };
        fmesh.faces.back()[2] = { 100.f,   -1.5f, -100.f };

        for (int i = 0; i < fmesh.faces.size(); i++)
        {
            fmesh.normals.push_back({ 0.f,1.f,0.f });
        }
        
        fmodel.meshes.push_back(fmesh);
        roof = fmodel;
        fmodel.meshes.front().mat.color = 0x10F010;
        scene.Add(std::move(fmodel));
    }
    // ---

    // Add a light
    const auto transform = mat4::Translate(0, 5, 0);
    for (auto& mesh : roof.meshes)
        for (auto& face : mesh.faces)
            for(int i =0; i < 3; i++)
                face[i] = transform.TransformPoint(face[i]);
    roof.meshes.front().mat.emissive = true;
    roof.meshes.front().mat.color = 0xDDDDDD;

    scene.Add(std::move(roof));

    //scene.Add(LoadGLTF("assets/Duck/glTF/Duck.gltf"));
    std::cout << "-----\nDone loading" << '\n';

    constexpr int x = 720;
    constexpr int y = 720;
    glfwSetWindowSize(window, x, y);

    dt = 0;
    speed = 10.f;
    camera = camera.Identity();
}

// -----------------------------------------------------------
// Close down application * 16.m_speed * deltaTime
// -----------------------------------------------------------
void Game::Shutdown()
{
    if(obs)
    obs->dump(std::ofstream("dump.json",std::ios::beg));
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick(float deltaTime)
{
    dt = deltaTime; // For camera. TODO make input manager

    // clear the graphics window
    renderer.Render(camera, *screen, scene);

    MoveCamera();

    // ImGui stuff
    {
        static bool show = false;
        ImGui::ShowDemoWindow(&show);

        if(!ImGui::Begin("Debug"))
        {
            ImGui::End();
            return;
        }
        int x, y;
        glfwGetWindowSize(window, &x, &y);

        if (ImGui::CollapsingHeader("Info",ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Delta time: %f", dt);
            ImGui::Text("FPS: %f", 1.f / dt);
            ImGui::Text("Window: %i %i", x,y);
            ImGui::Text("Render: %i %i", screen->GetWidth(), screen->GetHeight());
            ImGui::Text("Samples: %i/%i", renderer.SampleCount(), renderer.MaxSampleCount());
            ImGui::Text("Camera speed: "); ImGui::SameLine(); ImGui::DragFloat("##camera", &speed,0.2f,0.f);
        }

        if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Square X: "); ImGui::SameLine(); ImGui::DragScalar("##squareX", ImGuiDataType_U32, &renderer.squareX, 0.2f, 0);
            ImGui::Text("Square Y: "); ImGui::SameLine(); ImGui::DragScalar("##squareY", ImGuiDataType_U32, &renderer.squareY, 0.2f, 0);

        }
        ImGui::End();
    }
}

void Game::MoveCamera()
{
    float3 movement = float3();
    
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) movement.y -= 1;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) movement.y += 1;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movement.z += 1;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movement.z -= 1;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movement.x += 1;

    static double px = -1;
    static double py = -1;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (px == -1)
    {
        px = x; 
        py = y;
    }

    double dX = x - px;
    double dY = y - py;
    px = x;
    py = y;

    float rotX = 0.f;
    float rotY = 0.f;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        
        rotX = dX * 0.3f * 0.025f;
        rotY = dY * 0.2f * 0.025f;
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (rotX != 0 || rotY != 0 || dot(movement, movement) != 0.f)
    {
        renderer.OnMove();
    }
    
    camera = camera * mat4::RotateX(rotY) * mat4::RotateY(rotX) * mat4::Translate(movement * speed * clamp(dt,0.f,1.f));
}