#include "precomp.h" // include (only) this in every .cpp file

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	//scene.Add({ { -1,-1,1 }, { 1, -1, 1 }, { 0,1,1 } });
	//scene.Add({ { -1,-1,0.5f }, { 0, -1, 0.5f }, { -0.5f,1,1.5f }, 0xFF0000 });
	scene.Add(LoadGLTF("assets/Box/glTF/Box.gltf"));
	//scene.Add(LoadGLTF("assets/Duck/glTF/Duck.gltf"));
	std::cout << "-----\nDone loading" << '\n';

	constexpr int x = 1280;
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
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick(float deltaTime)
{
	dt = deltaTime; // For camera. TODO make input manager

	// clear the graphics window
	screen->Clear(deltaTime);

	MoveCamera();
	Render(camera, *screen, scene);

	static bool show = true;
	ImGui::ShowDemoWindow(&show);

	if(!ImGui::Begin("Debug"))
	{
		ImGui::End();
		return;
	}
	int x, y;
	glfwGetWindowSize(window, &x, &y);

	ImGui::Text("Delta time: %f", dt);
	ImGui::Text("FPS: %f", 1.f / dt);
	ImGui::Text("Window: %i %i", x,y);
	ImGui::Text("Render: %i %i", screen->GetWidth(), screen->GetHeight());
	ImGui::Text("Camera speed: "); ImGui::SameLine(); ImGui::InputFloat("", &speed);
	ImGui::End();

}

void Game::MoveCamera()
{
	float3 movement = float3();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) movement.y -= 1;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) movement.y += 1;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movement.z += 1;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movement.z -= 1;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movement.x += 1;

	double dX = 0.f;
	double dY = 0.f;
	glfwGetCursorPos(window, &dX, &dY);


	float rotX = 	ImGui::GetIO().MouseDelta.x* 0.1f * dt;
	float rotY = 	ImGui::GetIO().MouseDelta.y* 0.1f * dt;


	camera = camera * mat4::RotateX(rotY) * mat4::RotateY(rotX) * mat4::Translate(movement * speed * clamp(dt,0.f,1.f));
}