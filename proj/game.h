#pragma once

namespace Tmpl8 {

class Game
{
public:
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init();
	void Shutdown();
	void Tick( float deltaTime );

	void MouseUp(int button){};
	void MouseDown( int button ){};
	void MouseMove( int x, int y ){};
	void KeyUp( int key ){};
	void KeyDown(int key) {};
	
	Surface* screen;

private:
	void MoveCamera();

	Scene scene;

	float dt;
	float speed;
	mat4 camera;
};

}; // namespace Tmpl8