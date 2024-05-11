#include "Precompiled.h"
#include "PostProcessingEffect.h"
#include "RenderObject.h"
#include "GraphicsSystem.h"
#include "VertexTypes.h"

using namespace KTEngine;
using namespace KTEngine::Graphics;

namespace
{
	const char* gModeNames[] =
	{
		"None",
		"Monochrome",
		"Invert",
		"Mirror",
		"Blur",
		"Combine2",
		"MotionBlur",
		"ChromaticAberration"
	};
}

void PostProcessingEffect::Initialize(const std::filesystem::path& filePath)
{
	mPostProcessingBuffer.Initialize();
	mVertexShader.Initialize<VertexPX>(filePath);
	mPixelShader.Initialize(filePath);

	mSampler.Initialize(Sampler::Filter::Point, Sampler::AddressMode::Wrap);
}
void PostProcessingEffect::Terminate()
{
	mSampler.Terminate();
	mPixelShader.Terminate();
	mVertexShader.Terminate();
	mPostProcessingBuffer.Terminate();
}

void PostProcessingEffect::Begin()
{
	mVertexShader.Bind();
	mPixelShader.Bind();
	mPostProcessingBuffer.BindPS(0);
	mSampler.BindPS(0);
	for (uint32_t i = 0; i < mTextures.size(); ++i)
	{
		if (mTextures[i] != nullptr)
		{
			mTextures[i]->BindPS(i);
		}
	}
	PostProcessData data;
	data.mode = static_cast<int>(mMode);
	switch (mMode)
	{
	case Mode::None: break;
	case Mode::Monochrome: break;
	case Mode::Invert: break;
	case Mode::Mirror: 
	{
		data.params0 = mMirrorX;
		data.params1 = mMirrorY;
	}
	break;
	case Mode::Blur:
	{
		GraphicsSystem* gs = GraphicsSystem::Get();
		const float screenWidth = gs->GetBackBufferWidth();
		const float screenHeight = gs->GetBackBufferHeight();
		data.params0 = mBlurStrength / screenWidth;
		data.params1 = mBlurStrength / screenHeight;
	}
	break;
	case Mode::Combine2: break;
	case Mode::ChromaticAberration:
	{
		data.params0 = mAberrationValue;
		data.params1 = mAberrationValue;
	}
	break;
	default:
		break;
	}
	mPostProcessingBuffer.Update(data);
	mPostProcessingBuffer.BindPS(0);
}
void PostProcessingEffect::End()
{
	for (uint32_t i = 0; i < mTextures.size(); ++i)
	{
		Texture::UnbindPS(i);
	}
}

void PostProcessingEffect::Render(const RenderObject& renderObject)
{
	renderObject.meshBuffer.Render();
}

void PostProcessingEffect::DebugUI()
{
	if (ImGui::CollapsingHeader("PostProcessingEffect", ImGuiTreeNodeFlags_DefaultOpen))
	{
		int currentMode = static_cast<int>(mMode);
		if (ImGui::Combo("Mode", &currentMode, gModeNames, std::size(gModeNames)))
		{
			mMode = static_cast<Mode>(currentMode);
		}
		ImGui::DragFloat("MirrorY", &mMirrorY, 0.1f, -1.0f, 1.0f);
		ImGui::DragFloat("MirrorX", &mMirrorX, 0.1f, -1.0f, 1.0f);
		ImGui::DragFloat("BlurStrength", &mBlurStrength, 1.0f, 0.0f, 100.0f);
		ImGui::DragFloat("AberrationValue", &mAberrationValue, 0.001f, 0.0f, 1.0f);
	}
}

void PostProcessingEffect::SetTexture(const Texture* texture, uint32_t slot)
{
	ASSERT(slot < mTextures.size(), "PostProcessingEffect: invalid slot index");
	mTextures[slot] = texture;
}