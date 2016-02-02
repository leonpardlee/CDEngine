#ifndef EAE_ENGINE_CORE_INTERFACE
#define EAE_ENGINE_CORE_INTERFACE

#include "Engine/General/RTTI.h"

namespace EAE_Engine
{
	namespace Math
	{
		template<typename T>
		class TVector3;
		typedef TVector3<float> Vector3;
		class ColMatrix44;
		class Quaternion;
	}

	namespace Common
	{
		class IMessage
		{
		public:
			//virtual void BroadcastMessage(const char* message) = 0;
		};

		class ICompo;
		class IGetCompo
		{
		public:
			virtual ICompo* GetComponent(typeid_t type) = 0;
			//virtual IComponent* GetComponents(typeid_t type) = 0;
			//virtual IComponent* GetComponentInChildren() = 0;
			//virtual IComponent* GetComponentInParent() = 0;
		};

		class ITransform;
		class IGameObj : public IGetCompo
		{
		public:
			virtual ~IGameObj() {}
			virtual ITransform* GetTransform() = 0;
			virtual const char* GetName() = 0;
		//	virtual void AddComponent(IComponent*) = 0;
		};

		class ICompo : public IGetCompo
		{
		public:
			virtual ~ICompo() {}
			virtual IGameObj* GetGameObj() = 0;
		};

		class ITransform : public ICompo
		{
		public:
			virtual ~ITransform() {}
			// global transform
			virtual Math::Vector3 GetPos() = 0;
			virtual void SetPos(const Math::Vector3&) = 0;
			virtual void SetRotation(const Math::Quaternion&) = 0;
			virtual Math::Quaternion GetRotation() = 0;
			virtual void SetScale(const Math::Vector3&) = 0;
			virtual Math::Vector3 GetScale() = 0;
			// local transform
			virtual Math::Vector3& GetLocalPos() = 0;
			virtual void SetLocalPos(const Math::Vector3&) = 0;
			virtual void Rotate(const Math::Quaternion&) = 0;
			virtual Math::Quaternion& GetLocalRotation() = 0;
			virtual void SetLocalRotation(const Math::Quaternion&) = 0;
			virtual void SetLocalScale(const Math::Vector3&) = 0;
			virtual Math::Vector3 LocalScale() = 0;
			// Velocity
			virtual Math::Vector3& GetVelocity() = 0;
			virtual void SetVelocity(Math::Vector3& velocity) = 0;
			// Matrix
			virtual Math::ColMatrix44 GetRotateTransformMatrix() = 0;
			virtual Math::ColMatrix44 GetLocalToWorldMatrix() = 0;
			virtual Math::Vector3 GetForward() = 0;
			// Children
			virtual uint32_t GetChildCount() = 0;
			virtual ITransform* pGetChildren(uint32_t index = 0) = 0;
			// Parent
			virtual void SetParent(ITransform* pParent) = 0;
			virtual ITransform* GetParent() = 0;
		};

		class ICamera // : public ICompo
		{
		public:
			virtual ~ICamera() {}
			virtual Math::ColMatrix44 GetWroldToViewMatrix() = 0;
			virtual Math::ColMatrix44 GetProjClipMatrix() = 0;
			virtual ITransform* GetTransform() = 0;
			virtual Math::Vector3 ConvertWorldToViewport(const Math::Vector3& i_worldPos) = 0;
			virtual Math::Vector3 ConvertViewportToWorld(Math::Vector3& i_portPos) = 0;
		};

		class IController// : public ICompo
		{
		public:
			virtual ~IController() {}
			virtual void Update() = 0;
			virtual ITransform* GetTransform() = 0;
		};

	}
}
#endif//EAE_ENGINE_CORE_INTERFACE
