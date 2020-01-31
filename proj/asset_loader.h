#pragma once

/**
 * Free functions to load models
 */

Model LoadGLTF(const char* path, const mat4& t = mat4::Identity());