#include "../Common/AOSMesh.h"
#include "../Common/GraphicsOp.h"
#include "UserOutput/UserOutput.h"
#include <string>
#include <sstream>
#include <cassert>

namespace EAE_Engine
{
    namespace Graphics
    {
        AOSMesh::AOSMesh() : _vertexArrayId(0), 
            _vertexBufferId(0), _indexBufferId(0), 
            _vertexCount(0), _indexCount(0), 
            _bufferInfo({ 0, GL_TRIANGLES, 0})
        {}

        AOSMesh::~AOSMesh()
        {
        }
    
        bool AOSMesh::CreateBuffers(MeshGLVertexElements elements,
            void* pVertices, uint32_t vertexCount,
            uint32_t* pIndexData, uint32_t indexCount,
            Mesh::sSubMesh* pSubMeshes, uint32_t subMeshCount)
        {
            bool wereThereErrors = false;
            _vertexBufferId = 0;
            _indexBufferId = 0;
            _bufferInfo = elements._bufferInfo;
            assert(_bufferInfo._stride > 0);
            // Create a vertex array object and make it active
            if (!CreateVertexArrayObj(_vertexArrayId)) 
            {
                wereThereErrors = true;
                goto OnExit;
            }
            if (!CreateVertexBufferElements(elements, pVertices, vertexCount))
            {
                wereThereErrors = true;
                goto OnExit;
            }
            if (!CreateIndexBuffer(pIndexData, indexCount, pSubMeshes, subMeshCount))
            {
                wereThereErrors = true;
                goto OnExit;
            }
        OnExit:
            // Delete the buffer object
            // (the vertex array will hold a reference to it)
            if (_vertexArrayId != 0)
            {
                // Unbind the vertex array
                // (this must be done before deleting the vertex buffer)
                glBindVertexArray(0);
                const GLenum errorCode = glGetError();
                if (errorCode == GL_NO_ERROR)
                {
                    if (_vertexBufferId != 0)
                    {
                        // NOTE: If you delete the vertex buffer object here, as I recommend,
                        // then gDEBugger won't know about it and you won't be able to examine the data.
                        // If you find yourself in a situation where you want to see the buffer object data in gDEBugger
                        // comment out this block of code temporarily
                        // (doing this will cause a memory leak so make sure to restore the deletion code after you're done debugging).
                        //wereThereErrors = !DeleteVertexBufferObj(_vertexBufferId);
                    }
                    if (_indexBufferId != 0)
                    {
                        // NOTE: See the same comment above about deleting the vertex buffer object here and gDEBugger
                        // holds true for the index buffer
                        //wereThereErrors = !DeleteIndexBufferObj(_indexBufferId);
                    }
                }
                else
                {
                    wereThereErrors = true;
                    std::stringstream errorMessage;
                    errorMessage << "OpenGL failed to unbind the vertex array: " <<
                        reinterpret_cast<const char*>(gluErrorString(errorCode));
                    EAE_Engine::UserOutput::Print(errorMessage.str().c_str());
                }
            }
            return !wereThereErrors;
        }

        bool AOSMesh::CreateVertexBufferElements(MeshGLVertexElements elements, void* pVertices, uint32_t vertexCount)
        {
            bool result = true;
            //Create a buffer for vertex, remember that we need to use GL_ARRAY_BUFFER for vertex elements
            if (!CreateBindBufferObj(_vertexBufferId, GL_ARRAY_BUFFER))
            {
                result = false;
            }
            if (!InitVertexFormat(elements, _bufferInfo._stride))
            {
                result = false;
            }
            // Assign the data to the vertex buffer
             _vertexCount = vertexCount;//record how many vertex in this AOSMesh
            const uint32_t bufferSize = vertexCount * _bufferInfo._stride;
            result = FillGLBuffer(GL_ARRAY_BUFFER, pVertices, bufferSize, _bufferInfo._usage);//suage: GL_STATIC_DRAW
            return result;
        }

        bool AOSMesh::CreateIndexBuffer(uint32_t* pIndexData, uint32_t indexCount, Mesh::sSubMesh* pSubMeshes, uint32_t subMeshCount)
        {
            bool result = true;
            if (!CreateBindBufferObj(_indexBufferId, GL_ELEMENT_ARRAY_BUFFER))
            {
                result = false;
            }
            // Allocate space and copy the triangle data into the index buffer
            _indexCount = indexCount;
            const GLsizeiptr bufferSize = indexCount * sizeof(uint32_t);
            result = FillGLBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexData, bufferSize, _bufferInfo._usage);//suage: GL_STATIC_DRAW
            // Set the SubMeshes Information
            for (uint32_t subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex)
            {
                Mesh::sSubMesh pMesh(pSubMeshes[subMeshIndex]._firstIndex, pSubMeshes[subMeshIndex]._lastIndex);
                _subMeshes.push_back(pMesh);
            }
            return result;
        }

        void AOSMesh::Release()
        {
            if (_vertexArrayId != 0) 
            {
                DeleteVertexArrayObj(_vertexArrayId);
            }
            // Just make sure that we have clean the Vertex and Index buffers.
            if (_vertexBufferId != 0)
            {
                DeleteBufferObj(_vertexBufferId, 1);
            }
            if (_indexBufferId != 0)
            {
                DeleteBufferObj(_indexBufferId, 1);
            }
            //clean all of the submeshes.
            _subMeshes.clear();
        }

        void AOSMesh::ChangeWholeBuffers(void* pVertices, uint32_t vertexCount, uint32_t* pIndexData, uint32_t indexCount, Mesh::sSubMesh* pSubMeshes, uint32_t subMeshCount)
        {
            //clean all of the submeshes.
            _subMeshes.clear();
            // We can use glBufferSubData() or glMapBuffer to change the buffer.
            // However, because we want to change the Whole chunk of the memory, 
            // So it is good to use glBufferData(), 
            // which means we call the FillVertexBuffer and FillIndexBuffer again.
            glBindVertexArray(_vertexArrayId);
            //Update VOA
            glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
            _vertexCount = vertexCount;
            const uint32_t bufferSize = vertexCount * _bufferInfo._stride;
            bool result = FillGLBuffer(GL_ARRAY_BUFFER, pVertices, bufferSize, GL_STATIC_DRAW);
            //Update IOA
            if (pIndexData != nullptr) 
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
                _indexCount = indexCount;
                result = FillGLBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexData, indexCount * sizeof(uint32_t), GL_STATIC_DRAW);
                // Set the SubMeshes Information
                for (uint32_t subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex)
                {
                    Mesh::sSubMesh pMesh(pSubMeshes[subMeshIndex]._firstIndex, pSubMeshes[subMeshIndex]._lastIndex);
                    _subMeshes.push_back(pMesh);
                }
            }
            else 
            {
                if (_indexBufferId != 0)
                {
                    DeleteBufferObj(_indexBufferId, 1);
                }
                _indexCount = 0;
            }
            glBindVertexArray(0);
            // Check the errors so far.
            GLenum errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                std::stringstream errorMessage;
                errorMessage << "OpenGL failed to update the mesh buffers " <<
                    reinterpret_cast<const char*>(gluErrorString(errorCode));
                UserOutput::Print(errorMessage.str().c_str());
            }
        }

        Mesh::sSubMesh* AOSMesh::GetSubMesh(uint32_t subMeshIndex)
        {
            if (subMeshIndex > _subMeshes.size())
            {
                return nullptr;
            }
            return &_subMeshes[subMeshIndex];
        };

        void AOSMesh::SetSubMeshes(std::vector<Mesh::sSubMesh>& subMeshes)
        {
            _subMeshes.clear();
            _subMeshes = subMeshes;
        }

/////////////////////////////////////////////////////////InteralFunctions///////////////////////////////////////////////////////
        bool RenderElements(AOSMesh* pAOSMesh, uint32_t subMeshIndex);
        bool RenderArrays(AOSMesh* pAOSMesh);

        AOSMesh* CreateAOSMeshInternal(MeshGLVertexElements elements,
            void* pVertices, uint32_t vertexCount,
            uint32_t* pIndices, uint32_t indexCount,
            Mesh::sSubMesh* pSubMeshes, uint32_t subMeshCount)
        {
            AOSMesh* pMesh = new AOSMesh();
            pMesh->CreateBuffers(elements,
                pVertices, vertexCount,
                pIndices, indexCount,
                pSubMeshes, subMeshCount);
            return pMesh;
        }

        bool RenderAOSMeshInternal(AOSMesh* pAOSMesh, uint32_t subMeshIndex)
        {
            bool result = true;
            if (pAOSMesh->GetIndexCount() > 0)
            {
                // The mesh uses indices
                result = RenderElements(pAOSMesh, subMeshIndex);
            }
            else
            {
                // The mesh doesn't uses indices
                result = RenderArrays(pAOSMesh);
            }
            return result;
        }

        bool RenderElements(AOSMesh* pAOSMesh, uint32_t subMeshIndex)
        {
            // Render objects from the current streams
            {
                // We are using triangles as the "primitive" type,
                // and we have defined the vertex buffer as a triangle list
                // (meaning that every triangle is defined by three vertices)
                const GLenum mode = pAOSMesh->GetPrimitiveMode();
                // We'll use 32-bit indices in this class to keep things simple
                // (i.e. every index will be a 32 bit unsigned integer)
                const GLenum indexType = GL_UNSIGNED_INT;
                const uint32_t sizeOfIndex = sizeof(uint32_t);
                // It is possible to start rendering in the middle of an index buffer
                {
                    // Remember the void ptr should point to a byte, 
                    // so we need to calculate the offset based on the size of each index
                    const GLvoid* const offset = (GLvoid*)(pAOSMesh->GetSubMesh(subMeshIndex)->_firstIndex * sizeOfIndex);
                    // Draw the AOSMesh
                    const GLsizei vertexCountToRender = pAOSMesh->GetSubMesh(subMeshIndex)->GetIndicesCount();//count of vertices we want to render.
                    glDrawElements(mode, vertexCountToRender, indexType, offset);
                    assert(glGetError() == GL_NO_ERROR);
                }
            }
            return glGetError() == GL_NO_ERROR;
        }

        bool RenderArrays(AOSMesh* pAOSMesh)
        {
          // Render objects from the current streams
          {
            const GLenum mode = pAOSMesh->GetPrimitiveMode();
            const GLsizei vertexCountToRender = pAOSMesh->GetVertexCount();
            const GLint firstofTheVertices = 0;
            {
              GLsizei primitiveCountToRender = vertexCountToRender;
              switch (mode)
              {
              case GL_TRIANGLE_STRIP:
                primitiveCountToRender = vertexCountToRender;// -2;
                break;
              case GL_LINES:
                primitiveCountToRender = vertexCountToRender;// -2;
                break;
              case GL_LINE_STRIP:
                primitiveCountToRender = vertexCountToRender;// -2;
                break;
              default:
                primitiveCountToRender = vertexCountToRender;
              }
              // Draw the AOSMesh
              glDrawArrays(mode, firstofTheVertices, primitiveCountToRender);
              assert(glGetError() == GL_NO_ERROR);
            }
          }
          return glGetError() == GL_NO_ERROR;
        }

    }
}