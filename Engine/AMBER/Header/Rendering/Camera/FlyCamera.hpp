#ifndef __ENGINE_FLY_CAMERA__
#define __ENGINE_FLY_CAMERA__

#include "Camera.hpp"
#include "I_InputManager.hpp"

namespace Ge
{
	class FlyCamera : Camera
	{
	public:
		FlyCamera(VulkanMisc * vM, I_InputManager * input);
		void updateCamera();
	private:	
		I_InputManager * m_im; 
		float m_angleX;
		float m_angleY;
		float m_maxSpeed = 10.0f;
		float m_fastSpeedMult = 1.0f;
		float m_moveSpeed = 0.0f;
		float m_maxSpeedRotate = 0.2f;
		float m_addMoveSpeed = 0.0f;
	};
}

#endif //__ENGINE_FLY_CAMERA__