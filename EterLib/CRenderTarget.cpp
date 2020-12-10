#include "StdAfx.h"
#include "CRenderTarget.h"
#include "../EterLib/Camera.h"
#include "../EterLib/CRenderTargetManager.h"
#include "../EterPythonLib/PythonGraphic.h"


#include "../EterBase/CRC32.h"
#include "../GameLib/GameType.h"
#include "../GameLib/MapType.h"
#include "../GameLib/ItemData.h"
#include "../GameLib/ActorInstance.h"
#include "../UserInterface/InstanceBase.h"

#ifdef RENDER_TARGED_CHARACTER_WINDOW
#include "../UserInterface/GameType.h"
#include "../UserInterface/AbstractPlayer.h"
#endif


#include "ResourceManager.h"
#include "../UserInterface/Locale_inc.h"
#ifdef RENDER_TARGED
CRenderTarget::CRenderTarget(const DWORD width, const DWORD height) : m_pModel(nullptr),
                                 m_background(nullptr),
                                 m_modelRotation(0),
                                 m_visible(false) 
{
	auto pTex = new CGraphicRenderTargetTexture;
	if (!pTex->Create(width, height, D3DFMT_X8R8G8B8, D3DFMT_D16)) {
		delete pTex;
		TraceError("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture %dx%d", width, height);
		throw std::runtime_error("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture");
	}

	m_renderTargetTexture = std::unique_ptr<CGraphicRenderTargetTexture>(pTex);

#ifdef RENDER_TARGED_CHARACTER_WINDOW
	m_cameraPosition = D3DXVECTOR3(0.0f, -1500.0f, 500.0f);
	m_targetPosition = D3DXVECTOR3(0.0f, 0.0f, 120.0f);
	m_direction = m_targetPosition - m_cameraPosition;
	D3DXVec3Normalize(&m_direction, &m_direction);
#endif
}

CRenderTarget::~CRenderTarget()
{

}

void CRenderTarget::SetVisibility(bool isShow)
{
	m_visible = isShow;
}
void CRenderTarget::SetArmor(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeArmor(vnum);
}

void CRenderTarget::SetWeapon(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeWeapon(vnum);
}

void CRenderTarget::ChangeHair(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeHair(vnum);
	//Unnötig//
	/*m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
	m_modelRotation = 0.0f;
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);
	auto& camera_manager = CCameraManager::instance();
	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(110.0);
	camera_manager.ResetToPreviousCamera();*/
}
void CRenderTarget::RenderTexture() const
{
	m_renderTargetTexture->Render();
}

void CRenderTarget::SetRenderingRect(RECT* rect) const
{
	m_renderTargetTexture->SetRenderingRect(rect);
}

void CRenderTarget::CreateTextures() const
{
	m_renderTargetTexture->CreateTextures();
}

void CRenderTarget::ReleaseTextures() const
{
	m_renderTargetTexture->ReleaseTextures();
}

#ifdef RENDER_TARGED_CHARACTER_WINDOW
void CRenderTarget::SetCharacterArmor(DWORD armorID)
{
	if (m_pModel == NULL)
		return;

	m_pModel->ChangeArmor(armorID);
	m_pModel->Refresh(CRaceMotionData::TYPE_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::TYPE_WAIT);
}

void CRenderTarget::SetCharacterWeapon(DWORD weaponID)
{
	if (m_pModel == NULL)
		return;

	m_pModel->ChangeWeapon(weaponID);
	m_pModel->Refresh(CRaceMotionData::TYPE_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::TYPE_WAIT);
}

void CRenderTarget::SetCharacterHair(DWORD hairID)
{
	if (m_pModel == NULL)
		return;

	m_pModel->ChangeHair(hairID);
	m_pModel->Refresh(CRaceMotionData::TYPE_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::TYPE_WAIT);
}


void CRenderTarget::RenderPlayer()
{
	IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();

	if (&rPlayer == nullptr)
		return;

	CInstanceBase* player = rPlayer.NEW_GetMainActorPtr();

	if (player == nullptr)
		return;

	CInstanceBase::SCreateData kCreateData{};

	kCreateData.m_bType = CActorInstance::TYPE_PC; // Dynamic Type
	kCreateData.m_dwRace = player->GetRace();

	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
		{
			m_pModel.reset();
		}
		return;
	}

	m_pModel = std::move(model);
	m_pModel->NEW_SetPixelPosition(TPixelPosition(0.0, 0.0, 0.0));

	m_pModel->NEW_SetPixelPosition(TPixelPosition(0.0, 0.0, 0.0));
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
	m_modelRotation = 0.0f;
	if (!UpdatePlayerModel())
	{
		Tracef("CRenderTarget.RenderPlayer: Error in updating player model\n");
		return;
	}
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);

	auto& camera_manager = CCameraManager::instance();
	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(110.0);
	camera_manager.ResetToPreviousCamera();
}

bool CRenderTarget::UpdatePlayerModel()
{
	IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();

	if (&rPlayer == nullptr)
		return false;

	CInstanceBase* player = rPlayer.NEW_GetMainActorPtr();

	if (player == nullptr)
	{
		Tracef("CRenderTarget.UpdatePlayerModel: player is null, skipping updating\n");
		return false;
	}

	DWORD armorID = player->GetPart(CRaceData::EParts::PART_ARMOR);
	DWORD weaponID = player->GetPart(CRaceData::EParts::PART_WEAPON);
	DWORD hairID = player->GetPart(CRaceData::EParts::PART_HAIR);

	if (m_pModel == nullptr)
	{
		Tracef("CRenderTarget.UpdatePlayerModel: m_pModel is null, should call RenderPlayer first, skipping updating\n");
		return false;
	}

	m_pModel->SetArmor(armorID);
	m_pModel->SetWeapon(weaponID);
	m_pModel->SetHair(hairID);
	m_pModel->Refresh(CRaceMotionData::TYPE_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::TYPE_WAIT);
	return true;
}

void CRenderTarget::ZoomCamera(int direction)
{
	float distance = D3DXVec3Length(&(m_targetPosition - m_cameraPosition));

	if (((distance > 2300.0f) && (direction < 0)) || ((distance < 800.0f) && (direction > 0)))
		return;

	direction = direction > 0 ? 1 : -1;

	D3DXVECTOR3 newPosition = m_cameraPosition + (m_direction * direction * 125.0f);

	D3DXVec3Lerp(&m_cameraPosition, &m_cameraPosition, &newPosition, 0.5f);
}

void CRenderTarget::SetModelRotation(float value)
{
	m_modelRotation += value;
}

void CRenderTarget::SetAutoRotate(bool value)
{
	m_autoRotate = value;
}
#endif

void CRenderTarget::SelectModel(const DWORD index)
{
	CInstanceBase::SCreateData kCreateData{};


	kCreateData.m_bType = CActorInstance::TYPE_PC; // Dynamic Type
	kCreateData.m_dwRace = index;

	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
		{
			m_pModel.reset();
		}
		return;
	}

	m_pModel = std::move(model);
	m_pModel->NEW_SetPixelPosition(TPixelPosition(0,0,0));
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
	m_modelRotation = 0.0f;
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);

	auto& camera_manager = CCameraManager::instance();
	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(110.0);
	camera_manager.ResetToPreviousCamera();
}

//void CInstanceBase::SetEffect() {
//	GetGraphicThingInstanceRef().RenderAllAttachingEffect();
//	Refresh(CRaceMotionData::NAME_WAIT, true);
//}	

bool CRenderTarget::CreateBackground(const char* imgPath, const DWORD width, const DWORD height)
{
	if (m_background)
		return false;

	m_background = std::make_unique<CGraphicImageInstance>();
	
	const auto graphic_image = dynamic_cast<CGraphicImage*>(CResourceManager::instance().GetResourcePointer(imgPath));
	if (!graphic_image)
	{
		m_background.reset();
		return false;
	}

	m_background->SetImagePointer(graphic_image);
	m_background->SetScale(static_cast<float>(width) / graphic_image->GetWidth(), static_cast<float>(height) / graphic_image->GetHeight());
	return true;
}


void CRenderTarget::RenderBackground() const
{
	if (!m_visible)
		return;

	if (!m_background)
		return;

	auto& rectRender = *m_renderTargetTexture->GetRenderingRect();
	m_renderTargetTexture->SetRenderTarget();

	CGraphicRenderTargetTexture::Clear();
	CPythonGraphic::Instance().SetInterfaceRenderState();
	// BUG-FIX//
	/*const auto width = static_cast<float>(rectRender.right - rectRender.left);
	const auto height = static_cast<float>(rectRender.bottom - rectRender.top);

	CPythonGraphic::Instance().SetViewport(0.0f, 0.0f, width, height);*/

	m_background->Render();
	m_renderTargetTexture->ResetRenderTarget();
	//CPythonGraphic::Instance().RestoreViewport();
}

void CRenderTarget::UpdateModel()
{
	if (!m_visible || !m_pModel)
		return;

#ifdef RENDER_TARGED_CHARACTER_WINDOW
	if (m_autoRotate)
	{
		if (m_modelRotation < 360.0f)
			m_modelRotation += 1.0f;
		else
			m_modelRotation = 0.0f;
	}

#else
	if (m_modelRotation < 360.0f)
		m_modelRotation += 1.0f;
	else
		m_modelRotation = 0.0f;
#endif

	m_pModel->SetRotation(m_modelRotation);
	m_pModel->Transform();
	m_pModel->GetGraphicThingInstanceRef().RotationProcess();
}

void CRenderTarget::DeformModel() const
{
	if (!m_visible)
		return;

	if (m_pModel)
		m_pModel->Deform();
}

void CRenderTarget::RenderModel() const
{
	if (!m_visible)
	{
		return;
	}

	auto& python_graphic = CPythonGraphic::Instance();
	auto& camera_manager = CCameraManager::instance();
	auto& state_manager = CStateManager::Instance();

	auto& rectRender = *m_renderTargetTexture->GetRenderingRect();

	if (!m_pModel)
	{
		return;
	}

	m_renderTargetTexture->SetRenderTarget();

	if (!m_background)
	{
		m_renderTargetTexture->Clear();
	}


	python_graphic.ClearDepthBuffer();

	const auto fov = python_graphic.GetFOV();
	const auto aspect = python_graphic.GetAspect();
	const auto near_y = python_graphic.GetNear();
	const auto far_y = python_graphic.GetFar();


	const auto width = static_cast<float>(rectRender.right - rectRender.left);
	const auto height = static_cast<float>(rectRender.bottom - rectRender.top);


	state_manager.SetRenderState(D3DRS_FOGENABLE, FALSE);

	python_graphic.SetViewport(0.0f, 0.0f, width, height);

	python_graphic.PushState();

	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
#ifdef RENDER_TARGED_CHARACTER_WINDOW 
	camera_manager.GetCurrentCamera()->SetViewParams(
		m_cameraPosition,
		m_targetPosition,
		D3DXVECTOR3{ 0.0f, 0.0f, 1.0f }
	);

#else
	camera_manager.GetCurrentCamera()->SetViewParams(
		D3DXVECTOR3{ 0.0f, -1500.0f, 600.0f },
		D3DXVECTOR3{ 0.0f, 0.0f, 95.0f },
		D3DXVECTOR3{0.0f, 0.0f, 1.0f}
	);
#endif

	python_graphic.UpdateViewMatrix();

	python_graphic.SetPerspective(10.0f, width / height, 100.0f, 3000.0f);
	m_pModel->Render();
#ifdef RENDER_TARGED_SHINING
	m_pModel->GetGraphicThingInstanceRef().RenderAllAttachingEffect();
#endif
	camera_manager.ResetToPreviousCamera();
	python_graphic.RestoreViewport();
	python_graphic.PopState();
	python_graphic.SetPerspective(fov, aspect, near_y, far_y);
	m_renderTargetTexture->ResetRenderTarget();
	state_manager.SetRenderState(D3DRS_FOGENABLE, TRUE);
}
#endif