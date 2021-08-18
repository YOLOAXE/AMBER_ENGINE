#ifndef __ENGINE_MODEL_MANAGER___
#define __ENGINE_MODEL_MANAGER___

#include "VulkanMisc.hpp"
#include "Debug.hpp"
#include <map>
#include "ModelBuffer.hpp"
#include "Model.hpp"
#include <unordered_map>
#include "Descriptor.hpp"
#include "UniformBufferObject.hpp"

namespace Ge
{
    class ModelManager
    {
    public:
        bool initiliaze(VulkanMisc *vM);
        void release();
        ShapeBuffer *allocateBuffer(const char *path);
        Model *createModel(ShapeBuffer *buffer, std::string nom = "Model");
        void destroyModel(Model *model);
        void destroyBuffer(ShapeBuffer *buffer);

    private:
        std::map<ShapeBuffer *, ModelBuffer *> m_modelBuffers;
        std::map<Shape *, Model *> m_models;
        VulkanMisc *vulkanM;

    public:
        static Descriptor *m_descriptor;
    };
}

#endif //__ENGINE_MODEL_MANAGER___