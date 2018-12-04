#include "Engine/Networking/NetObjectView.hpp"

NetObjectView::NetObjectView(NetObject* netObject)
	: m_netObject(netObject)
{
	m_lastSentTimer.Reset();
}

float NetObjectView::GetTimeSinceLastSend() const
{
	return m_lastSentTimer.GetElapsedTime();
}

NetObject* NetObjectView::GetNetObject() const
{
	return m_netObject;
}

void NetObjectView::ResetTimeSinceLastSend()
{
	m_lastSentTimer.Reset();
}

