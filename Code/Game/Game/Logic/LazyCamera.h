#ifndef LAZY_CAMERA_H
#define LAZY_CAMERA_H

#include "Engine/Controller/Controller.h"
#include "Engine/Math/Vector.h"
#include "Engine/Math/EulerAngle.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/ColMatrix.h"
#include "Engine/Math/MathTool.h"
#include "Engine/Time/Time.h"
#include "Engine/UserInput/UserInput.h"
#include "Engine/Common/Interfaces.h"
#include "Engine/Engine/Engine.h"
#include "Engine/Graphics/Common/Camera.h"
#include "Engine/DebugShape/DebugShape.h"
#include <cassert>

class LazyCamera : public EAE_Engine::Controller::Controller
{
public:
  LazyCamera(EAE_Engine::Common::ICamera* pCamera) :
    EAE_Engine::Controller::Controller(pCamera->GetTransform()),
    _r(10.0f), _phi(20.0f), _theta(EAE_Engine::Math::Pi)
  {
    _inner = 2.0f;
    _outter = 10.0f;
  }

  void ResetCamera(EAE_Engine::Common::ITransform* pTarget)
  {
    _pTarget = pTarget;
    Reset();
  }

  void Update()
  {
    UpdateOrientation();
    return;
  }

private:
  void UpdateOrientation() 
  {
    if (!_pTarget)
      return;
    EAE_Engine::Math::Vector3 relativePos = _pTarget->GetPos() - _pTransform->GetPos();
    relativePos._y = 0.0f;
    EAE_Engine::Math::Vector3 relativeForward = relativePos.GetNormalize();
    EAE_Engine::Math::Vector3 targetForward = relativeForward;// _pTarget->GetForward();
    EAE_Engine::Math::Vector3 camForward = _pTransform->GetForward();
    float angle = EAE_Engine::Math::Degree(camForward, targetForward);
    EAE_Engine::Math::Vector3 normal = EAE_Engine::Math::Vector3::Cross(camForward, targetForward).Normalize();
    // get the direction of the angle.
    float direct = EAE_Engine::Math::Vector3::Dot(normal, EAE_Engine::Math::Vector3::Up);
    if (direct < 0)
      angle = -angle;
    // only when the camera is too far away from the Camera's Forward, we rotate the camera.
    if (angle > _phi || angle < -_phi)
    {
      // first, set the camera to look at the target
      _pTransform->SetForward(targetForward);
      // second, rotate back so that the target will keep staying at the edge
      EAE_Engine::Math::Quaternion rotation(_phi * EAE_Engine::Math::DegreeToRadian, normal);
      _pTransform->Rotate(rotation);
    }

  }

  void Reset() 
  {
    if (!_pTarget)
      return;
    _pTransform->SetLocalPos(_pTarget->GetPos() + _pTarget->GetForward() * -10.0f + EAE_Engine::Math::Vector3::Up * 1.0f);
  }

private:
  EAE_Engine::Common::ITransform* _pTarget;
  float _r;
  float _phi;
  float _theta;

  float _inner;
  float _outter;
};

#endif// LAZY_CAMERA_H