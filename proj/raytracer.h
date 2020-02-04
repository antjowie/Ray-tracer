#pragma once
/**
 * Contains the render systems that is responsible for rendering a scene
 */


class Renderer
{
public:
    void Render(
        const mat4& t, 
        Surface& screen, 
        const Scene& scene);

    bool showBVH;
    unsigned threadCount;
    unsigned squareX;
    unsigned squareY;
};
