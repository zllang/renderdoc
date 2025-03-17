/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2025 Baldur Karlsson
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

#include "dx_debug.h"
#include "common/formatting.h"
#include "driver/shaders/dxil/dxil_debug.h"
#include "dxbc_bytecode.h"
#include "dxbc_common.h"
#include "dxbc_container.h"
#include "dxbc_debug.h"

namespace DXDebug
{
void GatherPSInputDataForInitialValues(const DXBC::DXBCContainer *dxbc,
                                       const DXBC::DXBCContainer *prevdxbc, PSInputFetcher &fetcher,
                                       rdcarray<rdcstr> &floatInputs, rdcarray<rdcstr> &inputVarNames)
{
  rdcarray<DXBC::InterpolationMode> interpModes;

  const rdcarray<SigParameter> &stageInputSig = dxbc->GetReflection()->InputSig;
  const rdcarray<SigParameter> &prevStageOutputSig = prevdxbc->GetReflection()->OutputSig;

  if(dxbc->GetDXBCByteCode())
    DXBCDebug::GetInterpolationModeForInputParams(stageInputSig, dxbc->GetDXBCByteCode(),
                                                  interpModes);
  else
    DXILDebug::GetInterpolationModeForInputParams(stageInputSig, dxbc->GetDXILByteCode(),
                                                  interpModes);

  // When debugging a pixel shader, we need to get the initial values of each pixel shader
  // input for the pixel that we are debugging, from whichever the previous shader stage was
  // configured in the pipeline. This function returns the input element definitions, other
  // associated data, the HLSL definition to use when gathering pixel shader initial values,
  // and the stride of that HLSL structure.

  // This function does not provide any HLSL definitions for additional metadata that may be
  // needed for gathering initial values, such as primitive ID, and also does not provide the
  // shader function body.

  fetcher.inputs.clear();
  floatInputs.clear();
  inputVarNames.clear();
  fetcher.hlsl += "struct PSInput\n{\n";
  rdcstr defines;
  fetcher.stride = 0;

  if(stageInputSig.empty())
  {
    fetcher.hlsl += "float4 input_dummy : SV_Position;\n";
    fetcher.hlsl += "#define POSITION_VAR input_dummy\n";

    fetcher.inputs.push_back(PSInputElement(-1, 0, 4, ShaderBuiltin::Undefined, true));

    fetcher.stride += 4;
  }

  // name, pair<start semantic index, end semantic index>
  rdcarray<rdcpair<rdcstr, rdcpair<uint32_t, uint32_t>>> arrays;

  uint32_t nextreg = 0;

  size_t numInputs = stageInputSig.size();
  inputVarNames.resize(numInputs);

  for(size_t i = 0; i < numInputs; i++)
  {
    const SigParameter &sig = stageInputSig[i];

    fetcher.hlsl += "  ";

    bool included = true;

    // handled specially to account for SV_ ordering
    if(sig.systemValue == ShaderBuiltin::MSAACoverage ||
       sig.systemValue == ShaderBuiltin::IsFrontFace ||
       sig.systemValue == ShaderBuiltin::MSAASampleIndex)
    {
      fetcher.hlsl += "//";
      included = false;
    }

    // it seems sometimes primitive ID can be included within inputs and isn't subject to the SV_
    // ordering restrictions - possibly to allow for geometry shaders to output the primitive ID as
    // an interpolant. Only comment it out if it's the last input.
    if(i + 1 == numInputs && sig.systemValue == ShaderBuiltin::PrimitiveIndex)
    {
      fetcher.hlsl += "//";
      included = false;
    }

    int arrayIndex = -1;

    for(size_t a = 0; a < arrays.size(); a++)
    {
      if(sig.semanticName == arrays[a].first && arrays[a].second.first <= sig.semanticIndex &&
         arrays[a].second.second >= sig.semanticIndex)
      {
        fetcher.hlsl += "//";
        included = false;
        arrayIndex = sig.semanticIndex - arrays[a].second.first;
      }
    }

    int missingreg = int(sig.regIndex) - int(nextreg);

    // fill in holes from output sig of previous shader if possible, to try and
    // ensure the same register order
    for(int dummy = 0; dummy < missingreg; dummy++)
    {
      bool filled = false;

      size_t numPrevOutputs = prevStageOutputSig.size();
      for(size_t os = 0; os < numPrevOutputs; os++)
      {
        if(prevStageOutputSig[os].regIndex == nextreg + dummy)
        {
          filled = true;
          VarType varType = prevStageOutputSig[os].varType;
          uint32_t bytesPerColumn = (varType != VarType::Half) ? 4 : 2;

          if(varType == VarType::Float)
            fetcher.hlsl += "float";
          else if(varType == VarType::Half)
            fetcher.hlsl += "half";
          else if(varType == VarType::SInt)
            fetcher.hlsl += "int";
          else if(varType == VarType::UInt)
            fetcher.hlsl += "uint";
          else
            RDCERR("Unexpected input signature type: %s",
                   ToStr(prevStageOutputSig[os].varType).c_str());

          int numCols = (prevStageOutputSig[os].regChannelMask & 0x1 ? 1 : 0) +
                        (prevStageOutputSig[os].regChannelMask & 0x2 ? 1 : 0) +
                        (prevStageOutputSig[os].regChannelMask & 0x4 ? 1 : 0) +
                        (prevStageOutputSig[os].regChannelMask & 0x8 ? 1 : 0);

          rdcstr name = prevStageOutputSig[os].semanticIdxName;
          fetcher.hlsl += ToStr((uint32_t)numCols) + " input_" + name + " : " + name + ";\n";

          uint32_t byteSize = AlignUp4(numCols * bytesPerColumn);
          fetcher.stride += byteSize;

          fetcher.inputs.push_back(
              PSInputElement(-1, 0, byteSize / 4, ShaderBuiltin::Undefined, true));
        }
      }

      if(!filled)
      {
        rdcstr dummy_reg = "dummy_register";
        dummy_reg += ToStr((uint32_t)nextreg + dummy);
        fetcher.hlsl += "float4 var_" + dummy_reg + " : semantic_" + dummy_reg + ";\n";

        fetcher.inputs.push_back(PSInputElement(-1, 0, 4, ShaderBuiltin::Undefined, true));

        fetcher.stride += 4 * sizeof(float);
      }
    }

    nextreg = sig.regIndex + 1;

    DXBC::InterpolationMode interpolation = interpModes[i];
    if(interpolation != DXBC::InterpolationMode::INTERPOLATION_UNDEFINED)
      fetcher.hlsl += ToStr(interpolation) + " ";
    fetcher.hlsl += ToStr(sig.varType);

    int numCols = (sig.regChannelMask & 0x1 ? 1 : 0) + (sig.regChannelMask & 0x2 ? 1 : 0) +
                  (sig.regChannelMask & 0x4 ? 1 : 0) + (sig.regChannelMask & 0x8 ? 1 : 0);

    rdcstr name = sig.semanticIdxName;

    // arrays of interpolators are handled really weirdly. They use cbuffer
    // packing rules where each new value is in a new register (rather than
    // e.g. 2 x float2 in a single register), but that's pointless because
    // you can't dynamically index into input registers.
    // If we declare those elements as a non-array, the float2s or floats
    // will be packed into registers and won't match up to the previous
    // shader.
    // HOWEVER to add an extra bit of fun, fxc will happily pack other
    // parameters not in the array into spare parts of the registers.
    //
    // So I think the upshot is that we can detect arrays reliably by
    // whenever we encounter a float or float2 at the start of a register,
    // search forward to see if the next register has an element that is the
    // same semantic name and one higher semantic index. If so, there's an
    // array, so keep searching to enumerate its length.
    // I think this should be safe if the packing just happens to place those
    // registers together.

    int arrayLength = 0;

    if(included && numCols <= 2 && (sig.regChannelMask & 0x1))
    {
      uint32_t nextIdx = sig.semanticIndex + 1;

      for(size_t j = i + 1; j < numInputs; j++)
      {
        const SigParameter &jSig = stageInputSig[j];

        // if we've found the 'next' semantic
        if(sig.semanticName == jSig.semanticName && nextIdx == jSig.semanticIndex)
        {
          int jNumCols = (jSig.regChannelMask & 0x1 ? 1 : 0) + (jSig.regChannelMask & 0x2 ? 1 : 0) +
                         (jSig.regChannelMask & 0x4 ? 1 : 0) + (jSig.regChannelMask & 0x8 ? 1 : 0);

          DXBC::InterpolationMode jInterp = interpModes[j];

          // if it's the same size, type, and interpolation mode, then it could potentially be
          // packed into an array. Check if it's using the first channel component to tell whether
          // it's tightly packed with another semantic.
          if(jNumCols == numCols && interpolation == jInterp && sig.varType == jSig.varType &&
             jSig.regChannelMask & 0x1)
          {
            if(arrayLength == 0)
              arrayLength = 2;
            else
              arrayLength++;

            // continue searching now
            nextIdx++;
            j = i + 1;
            continue;
          }
        }
      }

      if(arrayLength > 0)
        arrays.push_back(
            make_rdcpair(sig.semanticName, make_rdcpair((uint32_t)sig.semanticIndex, nextIdx - 1)));
    }

    if(included)
    {
      // in UAV structs, arrays are packed tightly, so just multiply by arrayLength
      fetcher.stride += 4 * numCols * RDCMAX(1, arrayLength);
    }

    // as another side effect of the above, an element declared as a 1-length array won't be
    // detected but it WILL be put in its own register (not packed together), so detect this
    // case too.
    // Note we have to search *backwards* because we need to know if this register should have
    // been packed into the previous register, but wasn't. float/float2/float3 can be packed after
    // an array just fine, so long as the sum of their components doesn't exceed a register width
    if(included && i > 0 && arrayLength == 0)
    {
      const SigParameter &prev = stageInputSig[i - 1];

      if(prev.regIndex != sig.regIndex && prev.compCount + sig.compCount <= 4)
        arrayLength = 1;
    }

    // The compiler is also really annoying and will go to great lengths to rearrange elements
    // and screw up our declaration, to pack things together. E.g.:
    // float2 a : TEXCOORD1;
    // float4 b : TEXCOORD2;
    // float4 c : TEXCOORD3;
    // float2 d : TEXCOORD4;
    // the compiler will move d up and pack it into the last two components of a.
    // To prevent this, we look forward and backward to check that we aren't expecting to pack
    // with anything, and if not then we just make it a 1-length array to ensure no packing.
    // Note the regChannelMask & 0x1 means it is using .x, so it's not the tail-end of a pack
    if(included && arrayLength == 0 && numCols <= 2 && (sig.regChannelMask & 0x1))
    {
      if(i == numInputs - 1)
      {
        // the last element is never packed
        arrayLength = 1;
      }
      else
      {
        // if the next reg is using .x, it wasn't packed with us
        if(stageInputSig[i + 1].regChannelMask & 0x1)
          arrayLength = 1;
      }
    }

    rdcstr inputName = "input_" + name;
    fetcher.hlsl += ToStr((uint32_t)numCols) + " " + inputName;
    if(arrayLength > 0)
      fetcher.hlsl += "[" + ToStr(arrayLength) + "]";
    fetcher.hlsl += " : " + name;
    // DXIL does not allow redeclaring SV_ variables, any that we might need which could already be
    // in PSInput must be obtained from there and not redeclared in our entry point
    if(sig.systemValue == ShaderBuiltin::Position)
      defines += "#define POSITION_VAR " + inputName + "\n";
    else if(sig.systemValue == ShaderBuiltin::PrimitiveIndex)
      defines += "#define PRIM_VAR " + inputName + "\n";

    inputVarNames[i] = inputName;
    if(arrayLength > 0)
      inputVarNames[i] += StringFormat::Fmt("[%d]", RDCMAX(0, arrayIndex));

    if(included && sig.varType == VarType::Float)
    {
      if(arrayLength == 0)
      {
        floatInputs.push_back("input_" + name);
      }
      else
      {
        for(int a = 0; a < arrayLength; a++)
          floatInputs.push_back("input_" + name + "[" + ToStr(a) + "]");
      }
    }

    fetcher.hlsl += ";\n";

    int firstElem = sig.regChannelMask & 0x1   ? 0
                    : sig.regChannelMask & 0x2 ? 1
                    : sig.regChannelMask & 0x4 ? 2
                    : sig.regChannelMask & 0x8 ? 3
                                               : -1;
    uint32_t bytesPerColumn = (sig.varType != VarType::Half) ? 4 : 2;
    uint32_t byteSize = AlignUp4(numCols * bytesPerColumn);

    // arrays get added all at once (because in the struct data, they are contiguous even if
    // in the input signature they're not).
    if(arrayIndex < 0)
    {
      if(arrayLength == 0)
      {
        fetcher.inputs.push_back(
            PSInputElement(sig.regIndex, firstElem, byteSize / 4, sig.systemValue, included));
      }
      else
      {
        for(int a = 0; a < arrayLength; a++)
        {
          fetcher.inputs.push_back(
              PSInputElement(sig.regIndex + a, firstElem, byteSize / 4, sig.systemValue, included));
        }
      }
    }
  }

  fetcher.hlsl += "};\n\n" + defines;
}

void CreatePSInputFetcher(const DXBC::DXBCContainer *dxbc, const DXBC::DXBCContainer *prevdxbc,
                          const PSInputFetcherConfig &cfg, PSInputFetcher &fetcher)
{
  // If the pipe contains a geometry/mesh shader, then SV_PrimitiveID cannot be used in the pixel
  // shader without being emitted from the geometry shader. For now, check if this semantic
  // will succeed in a new pixel shader with the rest of the pipe unchanged
  bool usePrimitiveID = ((prevdxbc->m_Type != DXBC::ShaderType::Geometry) &&
                         (prevdxbc->m_Type != DXBC::ShaderType::Mesh));

  rdcarray<rdcstr> floatInputs;
  rdcarray<rdcstr> inputVarNames;
  DXDebug::GatherPSInputDataForInitialValues(dxbc, prevdxbc, fetcher, floatInputs, inputVarNames);

  for(const PSInputElement &e : fetcher.inputs)
  {
    if(e.sysattribute == ShaderBuiltin::PrimitiveIndex)
    {
      usePrimitiveID = true;
      break;
    }
  }

  fetcher.hlsl += StringFormat::Fmt(
      "#define DESTX %u.5\n"
      "#define DESTY %u.5\n"
      "#define USEPRIM %u\n"
      "#define MAXHIT %u\n",
      cfg.x, cfg.y, usePrimitiveID ? 1 : 0, DXDebug::maxPixelHits);

  if(cfg.uavspace == 0)
    fetcher.hlsl += StringFormat::Fmt(
        "#define HITBUFFER u%u\n"
        "#define EVALCACHEBUFFER u%u\n",
        cfg.uavslot, cfg.uavslot + 1);
  else
    fetcher.hlsl += StringFormat::Fmt(
        "#define HITBUFFER u%u, space%u\n"
        "#define EVALCACHEBUFFER u%u, space%u\n",
        cfg.uavslot, cfg.uavspace, cfg.uavslot + 1, cfg.uavspace);

  fetcher.hlsl += "\n";

  fetcher.hlsl += GetEmbeddedResource(quadswizzle_hlsl);

  fetcher.hlsl += R"(
struct LaneData
{
  float4 pixelPos;

  uint isHelper;
  uint quadId;
  uint quadLane;
  uint coverage;

  PSInput IN;
};

struct PixelDebugHit
{
  // only used in the first instance
  uint numHits;
  float3 pos_depth; // xy position and depth

  float derivValid;
  uint primitive;
  uint isFrontFace;
  uint sample;

  uint quadLaneIndex;
  uint3 pad;

  // input values
  LaneData quad[4];
};

RWStructuredBuffer<PixelDebugHit> HitBuffer : register(HITBUFFER);

// float4 is wasteful in some cases but it's easier than using ByteAddressBuffer and manual
// packing
RWBuffer<float4> EvalCacheBuffer : register(EVALCACHEBUFFER);

void ExtractInputsPS(PSInput IN,
#ifndef POSITION_VAR
                     float4 debug_pixelPos : SV_Position,
#endif
#if USEPRIM && !defined(PRIM_VAR)
                     uint primitive : SV_PrimitiveID,
#endif
                     // sample, coverage and isFrontFace are deliberately omittted from the
                     // IN struct for SV_ ordering reasons
                     uint sample : SV_SampleIndex,
                     uint coverage : SV_Coverage,
                     bool isFrontFace : SV_IsFrontFace)
{
#ifdef POSITION_VAR
  float4 debug_pixelPos = IN.POSITION_VAR;
#endif

#if USEPRIM && defined(PRIM_VAR)
  uint primitive = IN.PRIM_VAR;
#elif !USEPRIM
  uint primitive = 0;
#endif

  const uint quadLaneIndex = (2u * (uint(debug_pixelPos.y) & 1u)) + (uint(debug_pixelPos.x) & 1u);

  // grab our output slot
  uint idx = MAXHIT;
  if(abs(debug_pixelPos.x - DESTX) < 0.5f && abs(debug_pixelPos.y - DESTY) < 0.5f)
    InterlockedAdd(HitBuffer[0].numHits, 1, idx);
  idx = min(idx, MAXHIT);

  HitBuffer[idx].pos_depth = debug_pixelPos.xyz;

  HitBuffer[idx].derivValid = ddx(debug_pixelPos.x);

  HitBuffer[idx].primitive = primitive;

  HitBuffer[idx].isFrontFace = isFrontFace;
  HitBuffer[idx].sample = sample;

  HitBuffer[idx].quadLaneIndex = quadLaneIndex;

  // quad pixelPos will be set with other derivatives for float inputs

  // for the simple quad case, only the desired thread is considered non-helper
  HitBuffer[idx].quad[0].isHelper = 1u;
  HitBuffer[idx].quad[1].isHelper = 1u;
  HitBuffer[idx].quad[2].isHelper = 1u;
  HitBuffer[idx].quad[3].isHelper = 1u;
  HitBuffer[idx].quad[quadLaneIndex].isHelper = 0u;

  // quadId is a single value that's unique for this quad and uniform across the quad. Degenerate
  // for the simple quad case
  uint quadId = 1000+quadSwizzleHelper(quadLaneIndex, quadLaneIndex, 0u);
  HitBuffer[idx].quad[0].quadId = quadId;
  HitBuffer[idx].quad[1].quadId = quadId;
  HitBuffer[idx].quad[2].quadId = quadId;
  HitBuffer[idx].quad[3].quadId = quadId;

  // per-quad lane identifier, degenerate for the simple quad case
  HitBuffer[idx].quad[0].quadLane = 0;
  HitBuffer[idx].quad[1].quadLane = 1;
  HitBuffer[idx].quad[2].quadLane = 2;
  HitBuffer[idx].quad[3].quadLane = 3;

  // coverage is handled with pixelPos as it can vary per-thread

  // start off with just copying all the inputs to all the quad. For float inputs or uints that may
  // vary across the quad we will quadSwizzle them
  HitBuffer[idx].quad[0].IN = IN;
  HitBuffer[idx].quad[1].IN = IN;
  HitBuffer[idx].quad[2].IN = IN;
  HitBuffer[idx].quad[3].IN = IN;
)";

  for(int q = 0; q < 4; q++)
  {
    fetcher.hlsl += StringFormat::Fmt(
        "  HitBuffer[idx].quad[%i].pixelPos = "
        "quadSwizzleHelper(debug_pixelPos, quadLaneIndex, %i);\n",
        q, q);
    fetcher.hlsl += StringFormat::Fmt(
        "  HitBuffer[idx].quad[%i].coverage = "
        "quadSwizzleHelper(coverage, quadLaneIndex, %i);\n",
        q, q);
  }

  for(size_t i = 0; i < floatInputs.size(); i++)
  {
    const rdcstr &name = floatInputs[i];
    for(int q = 0; q < 4; q++)
    {
      fetcher.hlsl += StringFormat::Fmt(
          "  HitBuffer[idx].quad[%i].IN.%s = quadSwizzleHelper(IN.%s, quadLaneIndex, %i);\n", q,
          name.c_str(), name.c_str(), q);
    }
  }

  // if we're not rendering at MSAA, no need to fill the cache because evaluates will all return the
  // plain input anyway.
  if(cfg.outputSampleCount > 1)
  {
    if(dxbc->GetDXBCByteCode())
    {
      dxbc->GetDXBCByteCode()->CalculateEvalSampleCache(cfg, fetcher);
    }
    else
    {
      RDCWARN("TODO DXIL Pixel Shader Debugging support for MSAA Evaluate");
    }
  }

  if(!fetcher.evalSampleCacheData.empty())
  {
    fetcher.hlsl += StringFormat::Fmt("  uint stride = %zu;\n", fetcher.evalSampleCacheData.size());
    fetcher.hlsl += StringFormat::Fmt("  uint evalIdx = idx * stride * 4;\n");
    fetcher.hlsl += StringFormat::Fmt("  float4 evalCacheVal;\n");

    uint32_t evalIdx = 0;
    for(const SampleEvalCacheKey &key : fetcher.evalSampleCacheData)
    {
      uint32_t keyMask = 0;

      for(int32_t i = 0; i < key.numComponents; i++)
        keyMask |= (1 << (key.firstComponent + i));

      // find the name of the variable matching the operand, in the case of merged input variables.
      rdcstr name, swizzle = "xyzw";
      for(size_t i = 0; i < dxbc->GetReflection()->InputSig.size(); i++)
      {
        if(dxbc->GetReflection()->InputSig[i].regIndex == (uint32_t)key.inputRegisterIndex &&
           dxbc->GetReflection()->InputSig[i].systemValue == ShaderBuiltin::Undefined &&
           (dxbc->GetReflection()->InputSig[i].regChannelMask & keyMask) == keyMask)
        {
          name = inputVarNames[i];

          if(!name.empty())
            break;
        }
      }

      swizzle.resize(key.numComponents);

      if(name.empty())
      {
        RDCERR("Couldn't find matching input variable for v%d [%d:%d]", key.inputRegisterIndex,
               key.firstComponent, key.numComponents);
        fetcher.hlsl += StringFormat::Fmt("  EvalCacheBuffer[evalIdx+stride*0+%u] = 0;\n", evalIdx);
        fetcher.hlsl += StringFormat::Fmt("  EvalCacheBuffer[evalIdx+stride*1+%u] = 0;\n", evalIdx);
        fetcher.hlsl += StringFormat::Fmt("  EvalCacheBuffer[evalIdx+stride*2+%u] = 0;\n", evalIdx);
        fetcher.hlsl += StringFormat::Fmt("  EvalCacheBuffer[evalIdx+stride*3+%u] = 0;\n", evalIdx);
        evalIdx++;
        continue;
      }

      name = StringFormat::Fmt("IN.%s.%s", name.c_str(), swizzle.c_str());

      // we must write all components, so just swizzle the values - they'll be ignored later.
      rdcstr expandSwizzle = swizzle;
      while(expandSwizzle.size() < 4)
        expandSwizzle.push_back('x');

      if(key.sample >= 0)
      {
        fetcher.hlsl +=
            StringFormat::Fmt("  evalCacheVal = EvaluateAttributeAtSample(%s, %d).%s;\n",
                              name.c_str(), key.sample, expandSwizzle.c_str());
      }
      else
      {
        // we don't need to special-case EvaluateAttributeAtCentroid, since it's just a case with
        // 0,0
        fetcher.hlsl +=
            StringFormat::Fmt("  evalCacheVal = EvaluateAttributeSnapped(%s, int2(%d, %d)).%s;\n",
                              name.c_str(), key.offsetx, key.offsety, expandSwizzle.c_str());
      }

      fetcher.hlsl += StringFormat::Fmt(
          "  EvalCacheBuffer[evalIdx+stride*0+%u] = "
          "quadSwizzleHelper(evalCacheVal, quadLaneIndex, 0);\n",
          evalIdx);
      fetcher.hlsl += StringFormat::Fmt(
          "  EvalCacheBuffer[evalIdx+stride*1+%u] = "
          "quadSwizzleHelper(evalCacheVal, quadLaneIndex, 1);\n",
          evalIdx);
      fetcher.hlsl += StringFormat::Fmt(
          "  EvalCacheBuffer[evalIdx+stride*2+%u] = "
          "quadSwizzleHelper(evalCacheVal, quadLaneIndex, 2);\n",
          evalIdx);
      fetcher.hlsl += StringFormat::Fmt(
          "  EvalCacheBuffer[evalIdx+stride*3+%u] = "
          "quadSwizzleHelper(evalCacheVal, quadLaneIndex, 3);\n",
          evalIdx);

      evalIdx++;
    }
  }

  fetcher.hlsl += "\n}\n";
}

// "NaN has special handling. If one source operand is NaN, then the other source operand is
// returned. If both are NaN, any NaN representation is returned."

float dxbc_min(float a, float b)
{
  if(RDCISNAN(a))
    return b;

  if(RDCISNAN(b))
    return a;

  return a < b ? a : b;
}

double dxbc_min(double a, double b)
{
  if(RDCISNAN(a))
    return b;

  if(RDCISNAN(b))
    return a;

  return a < b ? a : b;
}

float dxbc_max(float a, float b)
{
  if(RDCISNAN(a))
    return b;

  if(RDCISNAN(b))
    return a;

  return a >= b ? a : b;
}

double dxbc_max(double a, double b)
{
  if(RDCISNAN(a))
    return b;

  if(RDCISNAN(b))
    return a;

  return a >= b ? a : b;
}

float round_ne(float x)
{
  if(!RDCISFINITE(x))
    return x;

  float rem = remainderf(x, 1.0f);

  return x - rem;
}

double round_ne(double x)
{
  if(!RDCISFINITE(x))
    return x;

  double rem = remainder(x, 1.0);

  return x - rem;
}

float flush_denorm(const float f)
{
  uint32_t x;
  memcpy(&x, &f, sizeof(f));

  // if any bit is set in the exponent, it's not denormal
  if(x & 0x7F800000)
    return f;

  // keep only the sign bit
  x &= 0x80000000;
  float ret;
  memcpy(&ret, &x, sizeof(ret));
  return ret;
}

uint32_t BitwiseReverseLSB16(uint32_t x)
{
  // Reverse the bits in x, then discard the lower half
  // https://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
  x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
  x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
  x = ((x >> 4) & 0x0F0F0F0F) | ((x & 0x0F0F0F0F) << 4);
  x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
  return x << 16;
}

uint32_t PopCount(uint32_t x)
{
  // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
  x = x - ((x >> 1) & 0x55555555);
  x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
  return (((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void get_sample_position(uint32_t sampleIndex, uint32_t sampleCount, float *position)
{
  // assume standard sample pattern - this might not hold in all cases
  // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476218(v=vs.85).aspx

  if(sampleIndex >= sampleCount)
  {
    // Per HLSL docs, if sampleIndex is out of bounds a zero vector is returned
    RDCWARN("sample index %u is out of bounds on resource bound to sample_pos (%u samples)",
            sampleIndex, sampleCount);
    position[0] = 0.0f;
    position[1] = 0.0f;
    position[2] = 0.0f;
    position[3] = 0.0f;
  }
  else
  {
    const float *sample_pattern = NULL;

// co-ordinates are given as (i,j) in 16ths of a pixel
#define _SMP(c) ((c) / 16.0f)

    if(sampleCount == 1)
    {
      sample_pattern = NULL;
    }
    else if(sampleCount == 2)
    {
      static const float pattern_2x[] = {
          _SMP(4.0f),
          _SMP(4.0f),
          _SMP(-4.0f),
          _SMP(-4.0f),
      };

      sample_pattern = &pattern_2x[0];
    }
    else if(sampleCount == 4)
    {
      static const float pattern_4x[] = {
          _SMP(-2.0f), _SMP(-6.0f), _SMP(6.0f), _SMP(-2.0f),
          _SMP(-6.0f), _SMP(2.0f),  _SMP(2.0f), _SMP(6.0f),
      };

      sample_pattern = &pattern_4x[0];
    }
    else if(sampleCount == 8)
    {
      static const float pattern_8x[] = {
          _SMP(1.0f),  _SMP(-3.0f), _SMP(-1.0f), _SMP(3.0f),  _SMP(5.0f),  _SMP(1.0f),
          _SMP(-3.0f), _SMP(-5.0f), _SMP(-5.0f), _SMP(5.0f),  _SMP(-7.0f), _SMP(-1.0f),
          _SMP(3.0f),  _SMP(7.0f),  _SMP(7.0f),  _SMP(-7.0f),
      };

      sample_pattern = &pattern_8x[0];
    }
    else if(sampleCount == 16)
    {
      static const float pattern_16x[] = {
          _SMP(1.0f),  _SMP(1.0f),  _SMP(-1.0f), _SMP(-3.0f), _SMP(-3.0f), _SMP(2.0f),  _SMP(4.0f),
          _SMP(-1.0f), _SMP(-5.0f), _SMP(-2.0f), _SMP(2.0f),  _SMP(5.0f),  _SMP(5.0f),  _SMP(3.0f),
          _SMP(3.0f),  _SMP(-5.0f), _SMP(-2.0f), _SMP(6.0f),  _SMP(0.0f),  _SMP(-7.0f), _SMP(-4.0f),
          _SMP(-6.0f), _SMP(-6.0f), _SMP(4.0f),  _SMP(-8.0f), _SMP(0.0f),  _SMP(7.0f),  _SMP(-4.0f),
          _SMP(6.0f),  _SMP(7.0f),  _SMP(-7.0f), _SMP(-8.0f),
      };

      sample_pattern = &pattern_16x[0];
    }
    else    // unsupported sample count
    {
      RDCERR("Unsupported sample count on resource for sample_pos: %u", sampleCount);
      sample_pattern = NULL;
    }

    if(sample_pattern == NULL)
    {
      position[0] = 0.0f;
      position[1] = 0.0f;
    }
    else
    {
      position[0] = sample_pattern[sampleIndex * 2 + 0];
      position[1] = sample_pattern[sampleIndex * 2 + 1];
    }
  }
#undef _SMP
}

};    // namespace DXDebug

#if ENABLED(ENABLE_UNIT_TESTS)

#include <limits>
#include "catch/catch.hpp"

using namespace DXDebug;

TEST_CASE("DXBC DXIL shader debugging helpers", "[program]")
{
  const float posinf = std::numeric_limits<float>::infinity();
  const float neginf = -std::numeric_limits<float>::infinity();
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float a = 1.0f;
  const float b = 2.0f;

  SECTION("dxbc_min")
  {
    CHECK(dxbc_min(neginf, neginf) == neginf);
    CHECK(dxbc_min(neginf, a) == neginf);
    CHECK(dxbc_min(neginf, posinf) == neginf);
    CHECK(dxbc_min(neginf, nan) == neginf);
    CHECK(dxbc_min(a, neginf) == neginf);
    CHECK(dxbc_min(a, b) == a);
    CHECK(dxbc_min(a, posinf) == a);
    CHECK(dxbc_min(a, nan) == a);
    CHECK(dxbc_min(posinf, neginf) == neginf);
    CHECK(dxbc_min(posinf, a) == a);
    CHECK(dxbc_min(posinf, posinf) == posinf);
    CHECK(dxbc_min(posinf, nan) == posinf);
    CHECK(dxbc_min(nan, neginf) == neginf);
    CHECK(dxbc_min(nan, a) == a);
    CHECK(dxbc_min(nan, posinf) == posinf);
    CHECK(RDCISNAN(dxbc_min(nan, nan)));
  };

  SECTION("dxbc_max")
  {
    CHECK(dxbc_max(neginf, neginf) == neginf);
    CHECK(dxbc_max(neginf, a) == a);
    CHECK(dxbc_max(neginf, posinf) == posinf);
    CHECK(dxbc_max(neginf, nan) == neginf);
    CHECK(dxbc_max(a, neginf) == a);
    CHECK(dxbc_max(a, b) == b);
    CHECK(dxbc_max(a, posinf) == posinf);
    CHECK(dxbc_max(a, nan) == a);
    CHECK(dxbc_max(posinf, neginf) == posinf);
    CHECK(dxbc_max(posinf, a) == posinf);
    CHECK(dxbc_max(posinf, posinf) == posinf);
    CHECK(dxbc_max(posinf, nan) == posinf);
    CHECK(dxbc_max(nan, neginf) == neginf);
    CHECK(dxbc_max(nan, a) == a);
    CHECK(dxbc_max(nan, posinf) == posinf);
    CHECK(RDCISNAN(dxbc_max(nan, nan)));
  };

  SECTION("test denorm flushing")
  {
    float foo = 3.141f;

    // check normal values
    CHECK(flush_denorm(0.0f) == 0.0f);
    CHECK(flush_denorm(foo) == foo);
    CHECK(flush_denorm(-foo) == -foo);

    // check NaN/inf values
    CHECK(RDCISNAN(flush_denorm(nan)));
    CHECK(flush_denorm(neginf) == neginf);
    CHECK(flush_denorm(posinf) == posinf);

    // check zero sign bit - bit more complex
    uint32_t negzero = 0x80000000U;
    float negzerof;
    memcpy(&negzerof, &negzero, sizeof(negzero));

    float flushed = flush_denorm(negzerof);
    CHECK(memcmp(&flushed, &negzerof, sizeof(negzerof)) == 0);

    // check that denormal values are flushed, preserving sign
    foo = 1.12104e-44f;
    CHECK(flush_denorm(foo) != foo);
    CHECK(flush_denorm(-foo) != -foo);
    CHECK(flush_denorm(foo) == 0.0f);
    flushed = flush_denorm(-foo);
    CHECK(memcmp(&flushed, &negzerof, sizeof(negzerof)) == 0);
  };
};

#endif    // ENABLED(ENABLE_UNIT_TESTS)
