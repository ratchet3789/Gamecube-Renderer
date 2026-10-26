// Created by Coby Jennings on 2026/10/08.

#ifndef GAMECUBEENGINE_OBSERVER_H
#define GAMECUBEENGINE_OBSERVER_H

class IObserver
{
public:
	virtual ~IObserver() = default;
	template<typename T>
	void Update(const T& data);
	int uid=0;
};

class ISubject
{
public:
	virtual ~ISubject() = default;
	virtual void Attach(IObserver* Observer)=0;
	virtual void Detach(IObserver* Observer)=0;
	virtual void Notify() = 0;
};

#define DEFINE_OBSERVER(Name, Int_MaxObservers, T) \
class Name : public ISubject \
{ \
	Name() { m_MaxObservers = Int_MaxObservers; } \
	virtual void Attach(IObserver* Observer) override \
	{ \
		if(Observer == nullptr || m_ActiveObservers >= m_MaxObservers) \
		{ \
			return; \
		} \
		m_ActiveObservers++; \
		m_pObserverList[m_ActiveObservers] = Observer; \
		m_pObserverList[m_ActiveObservers]->uid = m_ActiveObservers; \
	} \
 \
	virtual void Detach(IObserver* Observer) override \
	{ \
		if(m_ActiveObservers == 0 || Observer->uid > m_ActiveObservers) \
		{ \
			return; \
		} \
		m_pObserverList[Observer->uid] = nullptr; \
	} \
 \
	virtual void Notify() override \
	{ \
		for(int i = 0; i < m_ActiveObservers; i++) \
		{ \
			if(m_pObserverList[i] == nullptr) \
				continue; \
			 \
			m_pObserverList[i]->Update<T>(m_Callback); \
		} \
	} \
 \
private: \
	IObserver* m_pObserverList[Int_MaxObservers]; \
	T m_Callback; \
	uint8 m_ActiveObservers = 0; \
	uint8 m_MaxObservers=0; \
}; \

#endif // GAMECUBEENGINE_OBSERVER_H