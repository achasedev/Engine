#include "Engine/Networking/NetObject.hpp"
#include "Engine/Networking/NetObjectView.hpp"
#include "Engine/Networking/NetObjectConnectionView.hpp"

NetObjectConnectionView::~NetObjectConnectionView()
{
	int viewCount = (int)m_objectViews.size();

	for (int viewIndex = 0; viewIndex < viewCount; ++viewIndex)
	{
		delete m_objectViews[viewIndex];
	}

	m_objectViews.clear();
}

void NetObjectConnectionView::AddNetObjectView(NetObjectView* objectView)
{
	m_objectViews.push_back(objectView);
}

void NetObjectConnectionView::AddNetObjectView(NetObject* netObject)
{
	m_objectViews.push_back(new NetObjectView(netObject));
}

void NetObjectConnectionView::RemoveNetObjectView(NetObject* netObject)
{
	int viewCount = (int)m_objectViews.size();

	for (int viewIndex = 0; viewIndex < viewCount; ++viewIndex)
	{
		if (m_objectViews[viewIndex]->GetNetObject() == netObject)
		{
			delete m_objectViews[viewIndex];
			m_objectViews.erase(m_objectViews.begin() + viewIndex);
			break;
		}
	}
}

int NetObjectConnectionView::GetViewCount() const
{
	return (int)m_objectViews.size();
}

NetObjectView* NetObjectConnectionView::GetNextObjectViewToSendUpdateFor() const
{
	int objectViewCount = (int)m_objectViews.size();
	NetObjectView* nextViewToUpdate = nullptr;

	for (int i = 0; i < objectViewCount; ++i)
	{
		NetObjectView* currView = m_objectViews[i];

		// Only Update others with objects that we own
		// Redundant check! If we don't own it, we won't (shouldn't) have a view for
		// it to begin with
		if (!currView->GetNetObject()->DoIOwn())
		{
			continue;
		}

		if (currView->GetTimeSinceLastSend() > 0.f && (nextViewToUpdate == nullptr || nextViewToUpdate->GetTimeSinceLastSend() < currView->GetTimeSinceLastSend()))
		{
			nextViewToUpdate = currView;
		}
	}

	return nextViewToUpdate;
}

