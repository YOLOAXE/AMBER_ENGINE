#include "BehaviourManager.hpp"

namespace Ge
{
	void BehaviourManager::addBehaviour(Behaviour * b)
	{
		m_behaviours.push_back(b);
		m_startBehaviours.push_back(b);
	}

	void BehaviourManager::removeBehaviour(Behaviour * b)
	{		
		m_stopBehaviours.push_back(b);
		m_behaviours.erase(std::remove(m_behaviours.begin(), m_behaviours.end(), b), m_behaviours.end());
	}

	void BehaviourManager::update()
	{
		for(int i = 0; i< m_startBehaviours.size();i++)
		{
			m_startBehaviours[i]->start();
		}
		m_startBehaviours.clear();
		for (int i = 0; i < m_behaviours.size(); i++)
		{
			m_behaviours[i]->update();
		}
		for (int i = 0; i < m_stopBehaviours.size(); i++)
		{
			m_stopBehaviours[i]->stop();
		}
		m_stopBehaviours.clear();
	}

	void BehaviourManager::fixedUpdate()
	{
		for (int i = 0; i < m_behaviours.size(); i++)
		{
			m_behaviours[i]->fixedUpdate();
		}
	}
}