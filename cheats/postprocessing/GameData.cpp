#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <list>
#include <memory>
#include <unordered_map>

#include "../../ImGui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../ImGui/imgui_internal.h"

#ifdef _WIN32
#include "../../ImGui/imgui_impl_dx9.h"
#else
#include "../../ImGui/imgui_impl_opengl3.h"
#endif

#define STBI_ONLY_PNG
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "GameData.h"

