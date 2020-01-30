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

	Render(camera, *screen, scene);
}

void Game::KeyDown(int key)
{
	float3 movement = float3();
	if (key == GLFW_KEY_Q) movement.y -= 1;
	if (key == GLFW_KEY_E) movement.y += 1;

	if (key == GLFW_KEY_W) movement.z += 1;
	if (key == GLFW_KEY_A) movement.x -= 1;
	if (key == GLFW_KEY_S) movement.z -= 1;
	if (key == GLFW_KEY_D) movement.x += 1;

	float rot = 0;
	if (key == GLFW_KEY_LEFT) rot -= speed * dt;
	if (key == GLFW_KEY_RIGHT) rot += speed *dt;
	

	camera = camera * camera.RotateY(rot) * mat4::Translate(movement * speed * clamp(dt,0.f,1.f));
}