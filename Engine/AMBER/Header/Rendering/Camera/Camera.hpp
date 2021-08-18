#ifndef __ENGINE_CAMERA__
#define __ENGINE_CAMERA__

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <glm/common.hpp>
#include "Vector3.hpp"
#include "I_Camera.hpp"
#include "UniformBufferCamera.hpp"
#include "VulkanMisc.hpp"
#include "GObject.hpp"
#include "BufferManager.hpp"

namespace Ge
{
	class Camera : public I_Camera, public virtual GObject
	{
	private:
		friend class CameraManager;
		void updatePerspective();
	public:
		Camera(VulkanMisc * vM);
		~Camera();
		void setFieldOfView(float fov);
		void setNear(float near);
		void setFar(float far);
		void setPriority(int p);
		void setOrtho(bool state);
		float getFieldOfView();
		float getNear();
		float getFar();
		int getPriority();
		VmaBuffer getUniformBuffer();
		glm::mat4 getView();
		glm::mat4 getProjection();
	private:
		VulkanMisc * vulkanM;
		UniformBufferCamera m_uniformBufferCamera;
		VmaBuffer m_vmaUniformBuffer;
		float m_fov = 70.0f;
		float m_near = 0.1f;
		float m_far = 300.0f;
		int m_priority = 0;
		bool m_ortho = false;
	};
}

#endif//__ENGINE_CAMERA__