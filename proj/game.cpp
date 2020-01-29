#include "precomp.h" // include (only) this in every .cpp file

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	scene.Add({ { -1,-1,1 }, { 1, -1, 1 }, { 0,1,1 } });
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
	// clear the graphics window
	screen->Clear(deltaTime);

	Render(mat4::Identity(), *screen, scene);
}

void Tmpl8::Game::MoveCamera(float deltaTime)
{
	//float3 movement = float3();
	//if (glfwGetKey(GLFW_KEY_Q) == GLFW_TRUE) movement.y += 1;
	//if (glfwGetKey(GLFW_KEY_E) == GLFW_TRYE) movement.y += -1;
	//if (glfwGetKey(GLFW_KEY_W) == GLFW_TRYE) movement.z += 1;
	//if (glfwGetKey(GLFW_KEY_A) == GLFW_TRYE) movement.x -= 1;
	//if (glfwGetKey(GLFW_KEY_S) == GLFW_TRYE) movement.z -= 1;
	//if (glfwGetKey(GLFW_KEY_D) == GLFW_TRYE) movement.x += 1;
	//
	//m_camera.Translate(movement * m_speed * deltaTime);
}