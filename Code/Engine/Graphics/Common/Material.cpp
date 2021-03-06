#include "Material.h"
#include "General/MemoryOp.h"
#include "UniformVariable.h"
#include "Texture.h"

#include "Effect.h"
#if defined( EAEENGINE_PLATFORM_D3D9 )
#include <d3dx9shader.h>
#elif defined( EAEENGINE_PLATFORM_GL )
#include <gl/GL.h>
#include <gl/GLU.h>
#include "../../../External/OpenGlExtensions/OpenGlExtensions.h"
#endif

namespace EAE_Engine 
{
	namespace Graphics 
	{
		void MaterialDesc::SetUniformForEffect()
		{
			uint8_t* pUniformVariableValueBuffer = nullptr;
			pUniformVariableValueBuffer = (uint8_t*)this + _offsetOfUniformVariableValueBuffer;
			UniformBlockDesc* pUBD = GetUniformBlockDesc();
			if (!pUBD) return;
			UniformDesc* pUniformDescBuffer = GetUniformDesc();
			if (!pUniformDescBuffer) return;
			uint8_t* pUniformBlockNameBuffer = (uint8_t*)this + _offsetOfUniformBlockNameBuffer;
			for (uint32_t index = 0; index < _uniformBlockCount; ++index)
			{
				size_t offsetInNameBuffer = pUBD[index]._offsetInUniformBlockNameBuffer;
				const char* pUBName = (char*)(pUniformBlockNameBuffer + offsetInNameBuffer);
				uint32_t startIndex = pUBD[index]._startUniformDescIndex;
				uint32_t endIndex = pUBD[index]._endUniformDescIndex;
				if (strcmp(pUBName, "Default") == 0)
				{
					// Set each UniformDesc of this MaterialDesc
					for (size_t index = startIndex; index <= endIndex; ++index)
					{
						UniformDesc* pUD = &pUniformDescBuffer[index];
						uint8_t* pValue = pUniformVariableValueBuffer + pUD->_offsetInValueBuffer;
						uint32_t bufferSize = pUD->_valueBufferSize;
						// We can use the _pUniformVariable to set it,
						// But I think this way is slower because it updates the UniformVariable.
						// pUD->_pUniformVariable->SetValue(pValue, bufferSize);
						// pUD->_pUniformVariable->NotifyOwner(_pEffect);
						// Or We can also call the pEffect to change the uniform directly.
						// I think this will be faster because it set the Uniform via Effect directly.
#if defined( EAEENGINE_PLATFORM_D3D9 )
						_pEffect->SetUniform(pUD->_handle, pValue, bufferSize, pUD->_shaderType);
#elif defined( EAEENGINE_PLATFORM_GL )
						_pEffect->SetUniform(pUD->_pUniformVariable->GetUniformType(), pUD->_handle, pValue, bufferSize);
#endif
					}
				}
				else
				{
					//UniformBlock* pUB = UniformBlockManager::GetInstance()->GetUniformBlock(pUBName);
					//.......
				}
			}
		}

		void MaterialDesc::SetTexturesForEffect()
		{
			TextureDesc* pTexDescBuffer = GetTextureDesc();
			if (pTexDescBuffer == nullptr)
				return;
			// Set each UniformDesc of this MaterialDesc
			for (uint32_t index = 0; index < _textureCount; ++index)
			{
#if defined( EAEENGINE_PLATFORM_D3D9 )
				(pTexDescBuffer + index)->SetTexture();
#elif defined( EAEENGINE_PLATFORM_GL )
				// In OpenGL, rather than associating the texture directly with the sampler ID,
				// there is an indirect step involving what OpenGL calls a "texture unit". 
				// We have to associate the texture with an arbitrary texture unit, 
				// and then assign that texture unit to the sampler.
				// For now,  I just use the index of the texture in the material, 
				// and this guarantees that each texture in material uses a different texture unit. 
				uint32_t textureUnit = index;
				(pTexDescBuffer + index)->SetTexture(textureUnit);
#endif
			}
		}

		UniformBlockDesc* MaterialDesc::GetUniformBlockDesc()
		{
			if (_uniformBlockCount == 0) return nullptr;
			return (UniformBlockDesc*)((uint8_t*)this + sizeof(MaterialDesc));
		}
		UniformDesc* MaterialDesc::GetUniformDesc()
		{
			if (_uniformCount == 0) return nullptr;
			return (UniformDesc*)((uint8_t*)this + sizeof(MaterialDesc) + sizeof(UniformBlockDesc) * _uniformBlockCount);
		}

		TextureDesc* MaterialDesc::GetTextureDesc()
		{
			if (_textureCount == 0)
				return nullptr;
			return (TextureDesc*)((uint8_t*)this + sizeof(MaterialDesc) + sizeof(UniformBlockDesc) * _uniformBlockCount + sizeof(UniformDesc) * _uniformCount);
		}

		void MaterialDesc::ChangeTexture(uint32_t index, TextureInfo* pTextureInfo)
		{
			if (index >= _textureCount)
				return;
			TextureDesc* pTexDescBuffer = GetTextureDesc();
			if (pTexDescBuffer == nullptr)
				return;
			(pTexDescBuffer + index)->_pTextureInfo = pTextureInfo;
		}

    void MaterialDesc::ChangeUniformVariable(const char* i_pName, void* i_pValue)
    {
      if (_uniformCount == 0)
        return;
      char* pNameBuffer = (char*)((uint8_t*)this + _offsetOfUniformVariableNameBuffer);
      char** pNames = new char*[_uniformCount];
      for (uint32_t i = 0; i < _uniformCount; ++i)
      {
        UniformDesc* pUniform = GetUniformDesc() + i;
        uint32_t offsetInNameBuffer = pUniform->_offsetInNameBuffer;
        pNames[i] = (char*)this + _offsetOfUniformVariableNameBuffer + offsetInNameBuffer;
      }
      for (uint32_t i = 0; i < _uniformCount; ++i)
      {
        if (strcmp(i_pName, pNames[i]) == 0)
        {
          UniformDesc* pUniform = GetUniformDesc() + i;
          uint32_t offsetInValueBuffer = pUniform->_offsetInValueBuffer;
          uint8_t* pValueOfUniform = (uint8_t*)this + _offsetOfUniformVariableValueBuffer + offsetInValueBuffer;
          CopyMem((uint8_t*)i_pValue, pValueOfUniform, pUniform->_valueBufferSize);
        }
      }
      SAFE_DELETE_ARRAY(pNames);
    }

////////////////////////MaterialManager static members//////////////////
		MaterialManager* MaterialManager::s_pMaterialManager = nullptr;

		MaterialManager* MaterialManager::GetInstance()
		{
			if (!s_pMaterialManager)
				s_pMaterialManager = new MaterialManager();
			return s_pMaterialManager;
		}
		void MaterialManager::CleanInstance()
		{
			if (!s_pMaterialManager) 
				return;
			s_pMaterialManager->Clean();
			SAFE_DELETE(s_pMaterialManager);
		}

////////////////////////////////////member function////////////////////////////
		bool MaterialManager::AddMaterialDesc(const char* key, uint8_t* pValue)
		{
			for (std::vector<MaterialKeyValuePair>::const_iterator iter = _materialsDesc.begin(); iter != _materialsDesc.end(); ++iter)
			{
				MaterialKeyValuePair pair = *iter;
				char* pKey = const_cast<char*>(pair._pKey);
				uint8_t* pValue = pair._pMaterialDesc;
				if (strcmp(pKey, key) == 0) 
					return false;
			}
			MaterialKeyValuePair keyValuePair = { _strdup(key), pValue };
			_materialsDesc.push_back(keyValuePair);
			return true;
		}
		void MaterialManager::Clean()
		{
			for (std::vector<MaterialKeyValuePair>::const_iterator iter = _materialsDesc.begin(); iter != _materialsDesc.end();)
			{
				MaterialKeyValuePair pair = *iter++;
				char* pKey = const_cast<char*>(pair._pKey);
				uint8_t* pValue = pair._pMaterialDesc;
				SAFE_DELETE(pKey);
				SAFE_DELETE_ARRAY(pValue);
			}
			_materialsDesc.clear();
		}
		MaterialDesc* MaterialManager::GetMaterialDesc(const char* key)
		{
			for (std::vector<MaterialKeyValuePair>::const_iterator iter = _materialsDesc.begin(); iter != _materialsDesc.end(); ++iter)
			{
				MaterialKeyValuePair pair = *iter;
				char* pKey = const_cast<char*>(pair._pKey);
				uint8_t* pValue = pair._pMaterialDesc;
				if (strcmp(iter->_pKey, key) == 0)
				{
					return (MaterialDesc*)pValue;
				}
			}
			return nullptr;
		}

	}
}
