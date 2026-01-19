#pragma once
#include <GL/glew.h>
#include "GPUResource.h"

class GLResource : public GPUResource {
public:
    virtual ~GLResource() = default;
};