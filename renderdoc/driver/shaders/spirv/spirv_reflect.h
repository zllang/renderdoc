/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2025 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#include "api/replay/rdcarray.h"
#include "spirv_common.h"
#include "spirv_processor.h"

enum class GraphicsAPI : uint32_t;
enum class ShaderStage : uint8_t;
enum class ShaderBuiltin : uint32_t;
struct ShaderReflection;

struct SPIRVInterfaceAccess
{
  // ID of the base variable
  rdcspv::Id ID;

  // ID of the struct parent of this variable
  rdcspv::Id structID;

  // member in the parent struct of this variable (for MemberDecorate)
  uint32_t structMemberIndex = 0;

  // the access chain of indices
  rdcarray<uint32_t> accessChain;

  // this is an element of an array that's been exploded after [0].
  // i.e. this is false for non-arrays, and false for element [0] in an array, then true for
  // elements [1], [2], [3], etc..
  bool isArraySubsequentElement = false;
};

// extra information that goes along with a ShaderReflection that has extra information for SPIR-V
// patching
struct SPIRVPatchData
{
  // matches the input/output signature array, with details of where to fetch the output from in the
  // SPIR-V.
  rdcarray<SPIRVInterfaceAccess> inputs;
  rdcarray<SPIRVInterfaceAccess> outputs;

  // store the interface in reflection for lookup when generating binding indices
  rdcarray<rdcspv::Id> cblockInterface;
  rdcarray<rdcspv::Id> roInterface;
  rdcarray<rdcspv::Id> rwInterface;
  rdcarray<rdcspv::Id> samplerInterface;

  // set of used IDs
  rdcarray<rdcspv::Id> usedIds;

  // the spec IDs in order - these are the order of constants encountered while parsing, and are
  // used for byte offsets into the resulting data blob (each constant takes 64-bits).
  // The shader constants reported already have the write offset, but this allows looking up the
  // offset of a spec ID.
  rdcarray<uint32_t> specIDs;

  rdcspv::ThreadScope threadScope;

  // for mesh shaders, the maximum number of vertices/primitives generated by each meshlet
  uint32_t maxVertices = 0, maxPrimitives = 0;

  // if an invalid task payload is detected (non-struct, due to dxc bug generating many scalars
  // instead of one struct)
  bool invalidTaskPayload = false;

  bool usesPrintf = false;
};

namespace rdcspv
{
struct SourceFile
{
  rdcstr name;
  rdcstr contents;
};

class Reflector : public Processor
{
public:
  Reflector();
  virtual void Parse(const rdcarray<uint32_t> &spirvWords);

  rdcstr Disassemble(const rdcstr &entryPoint, std::map<size_t, uint32_t> &instructionLines) const;

  rdcarray<ShaderEntryPoint> EntryPoints() const;

  void MakeReflection(const GraphicsAPI sourceAPI, const ShaderStage stage,
                      const rdcstr &entryPoint, const rdcarray<SpecConstant> &specInfo,
                      ShaderReflection &reflection, SPIRVPatchData &patchData) const;

private:
  virtual void PreParse(uint32_t maxId);
  virtual void PostParse();

  virtual void RegisterOp(Iter iter);
  virtual void UnregisterOp(Iter iter);

  void CalculateArrayTypeName(DataType &type);

  rdcstr StringiseConstant(rdcspv::Id id) const;
  void CheckDebuggable(bool &debuggable, rdcstr &debugStatus) const;

  void ApplyMatrixByteStride(const DataType &type, uint8_t matrixByteStride,
                             rdcarray<ShaderConstant> &members) const;
  void MakeConstantBlockVariables(rdcspv::StorageClass storage, const DataType &structType,
                                  uint32_t arraySize, uint32_t arrayByteStride,
                                  rdcarray<ShaderConstant> &cblock,
                                  SparseIdMap<uint16_t> &pointerTypes,
                                  const rdcarray<SpecConstant> &specInfo) const;
  void MakeConstantBlockVariable(ShaderConstant &outConst, SparseIdMap<uint16_t> &pointerTypes,
                                 rdcspv::StorageClass storage, const DataType &type,
                                 const rdcstr &name, const Decorations &varDecorations,
                                 const rdcarray<SpecConstant> &specInfo) const;
  void AddSignatureParameter(const bool isInput, const ShaderStage stage, const Id id,
                             const Id structID, uint32_t &regIndex,
                             const SPIRVInterfaceAccess &parentPatch, const rdcstr &varName,
                             const DataType &type, const Decorations &decorations,
                             rdcarray<SigParameter> &sigarray, SPIRVPatchData &patchData,
                             const rdcarray<SpecConstant> &specInfo) const;

  rdcstr cmdline;
  DenseIdMap<rdcstr> strings;
  SourceLanguage sourceLanguage = SourceLanguage::Unknown;
  rdcarray<SourceFile> sources;
  SparseIdMap<size_t> debugSources;
  SparseIdMap<size_t> compUnitToFileIndex;
  SparseIdMap<size_t> debugFuncToBaseFile;
  SparseIdMap<rdcstr> debugFuncToCmdLine;
  SparseIdMap<LineColumnInfo> debugFuncToLocation;
  SparseIdMap<rdcstr> debugFuncName;
  SparseIdMap<Id> funcToDebugFunc;

  Id curBlock;
  std::set<Id> loopBlocks;

  struct MemberName
  {
    Id id;
    uint32_t member;
    rdcstr name;
  };

  rdcarray<MemberName> memberNames;
};

};    // namespace rdcspv

void StripCommonGLPrefixes(rdcstr &name);

void FillSpecConstantVariables(ResourceId shader, const SPIRVPatchData &patchData,
                               const rdcarray<ShaderConstant> &invars,
                               rdcarray<ShaderVariable> &outvars,
                               const rdcarray<SpecConstant> &specInfo);

// common function used by any API that utilises SPIR-V
void AddXFBAnnotations(const ShaderReflection &refl, const SPIRVPatchData &patchData,
                       uint32_t rastStream, const char *entryName, rdcarray<uint32_t> &modSpirv,
                       uint32_t &xfbStride);
