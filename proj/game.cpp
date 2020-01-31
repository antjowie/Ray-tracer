#include "precomp.h" // include (only) this in every .cpp file

tf::ExecutorObserver* obs = nullptr;
// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	//obs = executor.make_observer<tf::ExecutorObserver>();
	renderer.threadCount = std::thread::hardware_concurrency();
	renderer.squareX = 16;
	renderer.squareY = 16;

	//scene.Add({ { -1,-1,1 }, { 1, -1, 1 }, { 0,1,1 } });
	//scene.Add({ { -1,-1,0.5f }, { 0, -1, 0.5f }, { -0.5f,1,1.5f }, 0xFF0000 });
	scene.Add(LoadGLTF("assets/Box/glTF/Box.gltf",mat4::Translate(0,0,4)));
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
	screen->Clear(deltaTime);

	MoveCamera();
	renderer.Render(camera, *screen, scene);



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
			ImGui::Text("Camera speed: "); ImGui::SameLine(); ImGui::DragFloat("##camera", &speed,0.2f,0.f);
		}

		if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Thread count: "); ImGui::SameLine(); ImGui::DragScalar("##threadCount", ImGuiDataType_U32,&renderer.threadCount, 0.2f, 0);
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

	camera = camera * mat4::RotateX(rotY) * mat4::RotateY(rotX) * mat4::Translate(movement * speed * clamp(dt,0.f,1.f));
}