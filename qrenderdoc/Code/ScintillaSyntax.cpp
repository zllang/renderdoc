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

#include "ScintillaSyntax.h"
#include "Code/QRDUtils.h"
#include "scintilla/include/SciLexer.h"
#include "scintilla/include/qt/ScintillaEdit.h"

static const char *python_keywords =
    "False None True and as assert break class continue def del elif else except finally for from "
    "global if import in is lambda nonlocal not or pass raise return try while with yield";

static const char *hlsl_keywords[2] = {
    // keyword set 0:
    // Secondary keywords and identifiers
    R"EOKEYWORDS(
defined

register packoffset static const

break continue discard do for if else switch while case default return true false

abort abs acos all AllMemoryBarrier AllMemoryBarrierWithGroupSync any asdouble asfloat asin asint
asuint atan atan2 ceil clamp clip cos cosh countbits cross D3DCOLORtoUBYTE4 ddx ddx_coarse ddx_fine
ddy ddy_coarse ddy_fine degrees determinant DeviceMemoryBarrier DeviceMemoryBarrierWithGroupSync
distance dot dst errorf EvaluateAttributeAtCentroid EvaluateAttributeAtSample
EvaluateAttributeSnapped exp exp2 f16tof32 f32tof16 faceforward firstbithigh firstbitlow floor fma
fmod frac frexp fwidth GetRenderTargetSampleCount GetRenderTargetSamplePosition GroupMemoryBarrier
GroupMemoryBarrierWithGroupSync InterlockedAdd InterlockedAnd InterlockedCompareExchange
InterlockedCompareStore InterlockedExchange InterlockedMax InterlockedMin InterlockedOr
InterlockedXor isfinite isinf isnan ldexp length lerp lit log log10 log2 mad max min modf msad4 mul
noise normalize pow printf Process2DQuadTessFactorsAvg Process2DQuadTessFactorsMax
Process2DQuadTessFactorsMin ProcessIsolineTessFactors ProcessQuadTessFactorsAvg
ProcessQuadTessFactorsMax ProcessQuadTessFactorsMin ProcessTriTessFactorsAvg
ProcessTriTessFactorsMax ProcessTriTessFactorsMin radians rcp reflect refract reversebits round
rsqrt saturate sign sin sincos sinh smoothstep sqrt step tan tanh tex1D tex1Dbias tex1Dgrad tex1Dlod
tex1Dproj tex2D tex2Dbias tex2Dgrad tex2Dlod tex2Dproj tex3D tex3Dbias tex3Dgrad tex3Dlod tex3Dproj
texCUBE texCUBEbias texCUBEgrad texCUBElod texCUBEproj transpose trunc

BINORMAL BINORMAL0 BINORMAL1 BINORMAL2 BINORMAL3 BINORMAL4 BINORMAL5 BINORMAL6 BINORMAL7
BLENDINDICES BLENDINDICES0 BLENDINDICES1 BLENDINDICES2 BLENDINDICES3 BLENDINDICES4 BLENDINDICES5
BLENDINDICES6 BLENDINDICES7 BLENDWEIGHT BLENDWEIGHT0 BLENDWEIGHT1 BLENDWEIGHT2 BLENDWEIGHT3
BLENDWEIGHT4 BLENDWEIGHT5 BLENDWEIGHT6 BLENDWEIGHT7 COLOR COLOR0 COLOR1 COLOR2 COLOR3 COLOR4 COLOR5
COLOR6 COLOR7 NORMAL NORMAL0 NORMAL1 NORMAL2 NORMAL3 NORMAL4 NORMAL5 NORMAL6 NORMAL7 POSITION
POSITION0 POSITION1 POSITION2 POSITION3 POSITION4 POSITION5 POSITION6 POSITION7 POSITIONT PSIZE
PSIZE0 PSIZE1 PSIZE2 PSIZE3 PSIZE4 PSIZE5 PSIZE6 PSIZE7 TANGENT TANGENT0 TANGENT1 TANGENT2 TANGENT3
TANGENT4 TANGENT5 TANGENT6 TANGENT7 TEXCOORD TEXCOORD0 TEXCOORD1 TEXCOORD2 TEXCOORD3 TEXCOORD4
TEXCOORD5 TEXCOORD6 TEXCOORD7 TEXCOORD8 TEXCOORD9 TEXCOORD0 TEXCOORD1 TEXCOORD2 TEXCOORD3 TEXCOORD4
TEXCOORD5 TEXCOORD6 TEXCOORD7 TEXCOORD8 TEXCOORD9

SV_Coverage SV_Depth SV_DispatchThreadID SV_DomainLocation SV_GroupID SV_GroupIndex SV_GroupThreadID
SV_GSInstanceID SV_InsideTessFactor SV_IsFrontFace SV_OutputControlPointID SV_POSITION SV_Position
SV_RenderTargetArrayIndex SV_SampleIndex SV_TessFactor SV_ViewportArrayIndex SV_InstanceID
SV_PrimitiveID SV_VertexID SV_TargetID SV_TARGET SV_Target SV_Target0 SV_Target1 SV_Target2
SV_Target3 SV_Target4 SV_Target5 SV_Target6 SV_Target7 SV_ClipDistance0 SV_ClipDistance1
SV_ClipDistance2 SV_ClipDistance3 SV_ClipDistance4 SV_ClipDistance5 SV_ClipDistance6
SV_ClipDistance7 SV_CullDistance0 SV_CullDistance1 SV_CullDistance2 SV_CullDistance3
SV_CullDistance4 SV_CullDistance5 SV_CullDistance6 SV_CullDistance7
)EOKEYWORDS",

    // keyword set 1:
    // Secondary keywords and identifiers
    R"EOKEYWORDS(
bool bool1 bool2 bool3 bool4 bool1x1 bool1x2 bool1x3 bool1x4 bool2x1 bool2x2 bool2x3 bool2x4 bool3x1
bool3x2 bool3x3 bool3x4 bool4x1 bool4x2 bool4x3 bool4x4

int int1 int2 int3 int4 int1x1 int1x2 int1x3 int1x4 int2x1 int2x2 int2x3 int2x4 int3x1 int3x2 int3x3
int3x4 int4x1 int4x2 int4x3 int4x4

uint uint1 uint2 uint3 uint4 uint1x1 uint1x2 uint1x3 uint1x4 uint2x1 uint2x2 uint2x3 uint2x4 uint3x1
uint3x2 uint3x3 uint3x4 uint4x1 uint4x2 uint4x3 uint4x4

UINT UINT2 UINT3 UINT4

dword dword1 dword2 dword3 dword4 dword1x1 dword1x2 dword1x3 dword1x4 dword2x1 dword2x2 dword2x3
dword2x4 dword3x1 dword3x2 dword3x3 dword3x4 dword4x1 dword4x2 dword4x3 dword4x4

half half1 half2 half3 half4 half1x1 half1x2 half1x3 half1x4 half2x1 half2x2 half2x3 half2x4 half3x1
half3x2 half3x3 half3x4 half4x1 half4x2 half4x3 half4x4

float float1 float2 float3 float4 float1x1 float1x2 float1x3 float1x4 float2x1 float2x2 float2x3
float2x4 float3x1 float3x2 float3x3 float3x4 float4x1 float4x2 float4x3 float4x4

double double1 double2 double3 double4 double1x1 double1x2 double1x3 double1x4 double2x1 double2x2
double2x3 double2x4 double3x1 double3x2 double3x3 double3x4 double4x1 double4x2 double4x3 double4x4

snorm unorm string void cbuffer struct

Buffer AppendStructuredBfufer ByteAddressBuffer ConsumeStructuredBuffer StructuredBuffer RWBuffer
RWByteAddressBuffer RWStructuredBuffer RWTexture1D RWTexture1DArray RWTexture2D RWTexture2DArray
RWTexture3D

InputPatch OutputPatch

linear centroid nointerpolation noperspective sample

sampler sampler1D sampler2D sampler3D samplerCUBE SamplerComparisonState SamplerState sampler_state
AddressU AddressV AddressW BorderColor Filter MaxAnisotropy MaxLOD MinLOD MipLODBias ComparisonFunc
ComparisonFilter

texture Texture1D Texture1DArray Texture2D Texture2DArray Texture2DMS Texture2DMSArray Texture3D
TextureCube
)EOKEYWORDS"};

static const char *glsl_keywords[2] = {
    // keyword set 0:
    // Secondary keywords and identifiers
    R"EOKEYWORDS(
defined

in out inout static const

break continue do for while switch case default if else true false discard return

radians degrees sin cos tan asin acos atan sinh cosh tanh asinh acosh atanh pow exp log exp2 log2
sqrt inversesqrt abs sign floor trunc round roundEven ceil fract mod modf min max clamp mix step
smoothstep isnan isinf floatBitsToInt floatBitsToUint intBitsToFloat uintBitsToFloat fma frexp ldexp

packUnorm2x16 packSnorm2x16 packUnorm4x8 packSnorm4x8 unpackUnorm2x16 unpackSnorm2x16 unpackUnorm4x8
unpackSnorm4x8 packDouble2x32 unpackDouble2x32 packHalf2x16 unpackHalf2x16 length distance dot cross
normalize faceforward reflect refract matrixCompMult outerProduct transpose determinant inverse
lessThan lessThanEqual greaterThan greaterThanEqual equal notEqual any all not uaddCarry usubBorrow
umulExtended imulExtended bitfieldExtract bitfieldInsert bitfieldReverse bitCount findLSB findMSB

textureSize textureQueryLod textureQueryLevels textureSamples texture textureProj textureLod
textureOffset texelFetch texelFetchOffset textureProjOffset textureLodOffset textureProjLod
textureProjLodOffset textureGrad textureGradOffset textureProjGrad textureProjGradOffset
textureGather textureGatherOffset textureGatherOffsets

atomicCounterIncrement atomicCounterDecrement atomicCounter atomicAdd atomicMin atomicMax atomicAnd
atomicOr atomicXor atomicExchange atomicCompSwap

imageSize imageSamples imageLoad imageStore imageAtomicAdd imageAtomicMin imageAtomicMax
imageAtomicAnd imageAtomicOr imageAtomicXor imageAtomicExchange imageAtomicCompSwap

dFdx dFdy dFdxFine dFdyFine dFdxCoarse dFdyCoarse fwidth fwidthFine fwidthCoarse
interpolateAtCentroid interpolateAtSample interpolateAtOffset EmitStreamVertex EndStreamPrimitive
EmitVertex EndPrimitive barrier memoryBarrier memoryBarrierAtomicCounter memoryBarrierBuffer
memoryBarrierShared memoryBarrierImage groupMemoryBarrier

debugPrintfEXT

gl_CullDistance gl_FragCoord gl_FragDepth gl_FrontFacing gl_GlobalInvocationID gl_HelperInvocation
gl_in gl_InstanceID gl_InvocationID gl_Layer gl_LocalInvocationID gl_LocalInvocationIndex
gl_MaxPatchVertices gl_NumWorkGroups gl_out gl_PatchVerticesIn gl_PerVertex gl_PointCoord
gl_PointSize gl_Position gl_PrimitiveID gl_PrimitiveIDIn gl_SampleID gl_SampleMask gl_SampleMaskIn
gl_SamplePosition gl_TessCoord gl_TessLevelInner gl_TessLevelOuter gl_VertexID gl_ViewIndex gl_ViewportIndex
gl_WorkGroupID gl_WorkGroupSize

gl_MaxComputeWorkGroupCount gl_MaxComputeWorkGroupSize gl_MaxComputeUniformComponents
gl_MaxComputeTextureImageUnits gl_MaxComputeImageUniforms gl_MaxComputeAtomicCounters
gl_MaxComputeAtomicCounterBuffers gl_MaxVertexAttribs gl_MaxVertexUniformComponents
gl_MaxVaryingComponents gl_MaxVertexOutputComponents gl_MaxGeometryInputComponents
gl_MaxGeometryOutputComponents gl_MaxFragmentInputComponents gl_MaxVertexTextureImageUnits
gl_MaxCombinedTextureImageUnits gl_MaxTextureImageUnits gl_MaxImageUnits
gl_MaxCombinedImageUnitsAndFragmentOutputs gl_MaxCombinedShaderOutputResources gl_MaxImageSamples
gl_MaxVertexImageUniforms gl_MaxTessControlImageUniforms gl_MaxTessEvaluationImageUniforms
gl_MaxGeometryImageUniforms gl_MaxFragmentImageUniforms gl_MaxCombinedImageUniforms
gl_MaxFragmentUniformComponents gl_MaxDrawBuffers gl_MaxClipDistances
gl_MaxGeometryTextureImageUnits gl_MaxGeometryOutputVertices gl_MaxGeometryTotalOutputComponents
gl_MaxGeometryUniformComponents gl_MaxGeometryVaryingComponents gl_MaxTessControlInputComponents
gl_MaxTessControlOutputComponents gl_MaxTessControlTextureImageUnits
gl_MaxTessControlUniformComponents gl_MaxTessControlTotalOutputComponents
gl_MaxTessEvaluationInputComponents gl_MaxTessEvaluationOutputComponents
gl_MaxTessEvaluationTextureImageUnits gl_MaxTessEvaluationUniformComponents
gl_MaxTessPatchComponents gl_MaxPatchVertices gl_MaxTessGenLevel gl_MaxViewports
gl_MaxVertexUniformVectors gl_MaxFragmentUniformVectors gl_MaxVaryingVectors
gl_MaxVertexAtomicCounters gl_MaxTessControlAtomicCounters gl_MaxTessEvaluationAtomicCounters
gl_MaxGeometryAtomicCounters gl_MaxFragmentAtomicCounters gl_MaxCombinedAtomicCounters
gl_MaxAtomicCounterBindings gl_MaxVertexAtomicCounterBuffers gl_MaxTessControlAtomicCounterBuffers
gl_MaxTessEvaluationAtomicCounterBuffers gl_MaxGeometryAtomicCounterBuffers
gl_MaxFragmentAtomicCounterBuffers gl_MaxCombinedAtomicCounterBuffers gl_MaxAtomicCounterBufferSize
gl_MinProgramTexelOffset gl_MaxProgramTexelOffset gl_MaxTransformFeedbackBuffers
gl_MaxTransformFeedbackInterleavedComponents gl_MaxCullDistances gl_MaxCombinedClipAndCullDistances
gl_MaxSamples gl_MaxVertexImageUniforms gl_MaxFragmentImageUniforms gl_MaxComputeImageUniforms
gl_MaxCombinedImageUniforms gl_MaxCombinedShaderOutputResources gl_DepthRangeParameters
gl_DepthRange gl_NumSamples
)EOKEYWORDS",

    // keyword set 1:
    // Secondary keywords and identifiers
    R"EOKEYWORDS(
float double int void bool

mat2 mat3 mat4 dmat2 dmat3 dmat4 mat2x2 mat2x3 mat2x4 dmat2x2 dmat2x3 dmat2x4 mat3x2 mat3x3 mat3x4
dmat3x2 dmat3x3 dmat3x4 mat4x2 mat4x3 mat4x4 dmat4x2 dmat4x3 dmat4x4 vec2 vec3 vec4 ivec2 ivec3
ivec4 bvec2 bvec3 bvec4 dvec2 dvec3 dvec4 uint uvec2 uvec3 uvec4

atomic_uint patch sample buffer subroutine struct

invariant precise layout

lowp mediump highp precision attribute uniform varying shared coherent volatile restrict readonly
writeonly centroid flat smooth noperspective

sampler1D sampler2D sampler3D samplerCube sampler1DShadow sampler2DShadow samplerCubeShadow
sampler1DArray sampler2DArray sampler1DArrayShadow sampler2DArrayShadow isampler1D isampler2D
isampler3D isamplerCube isampler1DArray isampler2DArray usampler1D usampler2D usampler3D
usamplerCube usampler1DArray usampler2DArray sampler2DRect sampler2DRectShadow isampler2DRect
usampler2DRect samplerBuffer isamplerBuffer usamplerBuffer sampler2DMS isampler2DMS usampler2DMS
sampler2DMSArray isampler2DMSArray usampler2DMSArray samplerCubeArray samplerCubeArrayShadow
isamplerCubeArray usamplerCubeArray

image1D iimage1D uimage1D image2D iimage2D uimage2D image3D iimage3D uimage3D image2DRect
iimage2DRect uimage2DRect imageCube iimageCube uimageCube imageBuffer iimageBuffer uimageBuffer
image1DArray iimage1DArray uimage1DArray image2DArray iimage2DArray uimage2DArray imageCubeArray
iimageCubeArray uimageCubeArray image2DMS iimage2DMS uimage2DMS image2DMSArray iimage2DMSArray
uimage2DMSArray
)EOKEYWORDS"};

static const char *buffer_keywords[2] = {"struct enum",

                                         // keyword set 1:
                                         // Secondary keywords and identifiers
                                         R"EOKEYWORDS(
bool
bool bool1 bool2 bool3 bool4 bool1x1 bool1x2 bool1x3 bool1x4 bool2x1 bool2x2 bool2x3 bool2x4 bool3x1
bool3x2 bool3x3 bool3x4 bool4x1 bool4x2 bool4x3 bool4x4

char
char char1 char2 char3 char4 char1x1 char1x2 char1x3 char1x4 char2x1 char2x2 char2x3 char2x4 char3x1
char3x2 char3x3 char3x4 char4x1 char4x2 char4x3 char4x4

byte
byte byte1 byte2 byte3 byte4 byte1x1 byte1x2 byte1x3 byte1x4 byte2x1 byte2x2 byte2x3 byte2x4 byte3x1
byte3x2 byte3x3 byte3x4 byte4x1 byte4x2 byte4x3 byte4x4

ubyte
ubyte ubyte1 ubyte2 ubyte3 ubyte4 ubyte1x1 ubyte1x2 ubyte1x3 ubyte1x4 ubyte2x1 ubyte2x2 ubyte2x3 ubyte2x4 ubyte3x1
ubyte3x2 ubyte3x3 ubyte3x4 ubyte4x1 ubyte4x2 ubyte4x3 ubyte4x4

xbyte
xbyte xbyte1 xbyte2 xbyte3 xbyte4 xbyte1x1 xbyte1x2 xbyte1x3 xbyte1x4 xbyte2x1 xbyte2x2 xbyte2x3 xbyte2x4 xbyte3x1
xbyte3x2 xbyte3x3 xbyte3x4 xbyte4x1 xbyte4x2 xbyte4x3 xbyte4x4

short
short short1 short2 short3 short4 short1x1 short1x2 short1x3 short1x4 short2x1 short2x2 short2x3 short2x4 short3x1
short3x2 short3x3 short3x4 short4x1 short4x2 short4x3 short4x4

ushort
ushort ushort1 ushort2 ushort3 ushort4 ushort1x1 ushort1x2 ushort1x3 ushort1x4 ushort2x1 ushort2x2 ushort2x3 ushort2x4 ushort3x1
ushort3x2 ushort3x3 ushort3x4 ushort4x1 ushort4x2 ushort4x3 ushort4x4

xshort
xshort xshort1 xshort2 xshort3 xshort4 xshort1x1 xshort1x2 xshort1x3 xshort1x4 xshort2x1 xshort2x2 xshort2x3 xshort2x4 xshort3x1
xshort3x2 xshort3x3 xshort3x4 xshort4x1 xshort4x2 xshort4x3 xshort4x4

long
long long1 long2 long3 long4 long1x1 long1x2 long1x3 long1x4 long2x1 long2x2 long2x3 long2x4 long3x1
long3x2 long3x3 long3x4 long4x1 long4x2 long4x3 long4x4

ulong
ulong ulong1 ulong2 ulong3 ulong4 ulong1x1 ulong1x2 ulong1x3 ulong1x4 ulong2x1 ulong2x2 ulong2x3 ulong2x4 ulong3x1
ulong3x2 ulong3x3 ulong3x4 ulong4x1 ulong4x2 ulong4x3 ulong4x4

xlong
xlong xlong1 xlong2 xlong3 xlong4 xlong1x1 xlong1x2 xlong1x3 xlong1x4 xlong2x1 xlong2x2 xlong2x3 xlong2x4 xlong3x1
xlong3x2 xlong3x3 xlong3x4 xlong4x1 xlong4x2 xlong4x3 xlong4x4

int
int int1 int2 int3 int4 int1x1 int1x2 int1x3 int1x4 int2x1 int2x2 int2x3 int2x4 int3x1
int3x2 int3x3 int3x4 int4x1 int4x2 int4x3 int4x4

uint
uint uint1 uint2 uint3 uint4 uint1x1 uint1x2 uint1x3 uint1x4 uint2x1 uint2x2 uint2x3 uint2x4 uint3x1
uint3x2 uint3x3 uint3x4 uint4x1 uint4x2 uint4x3 uint4x4

xint
xint xint1 xint2 xint3 xint4 xint1x1 xint1x2 xint1x3 xint1x4 xint2x1 xint2x2 xint2x3 xint2x4 xint3x1
xint3x2 xint3x3 xint3x4 xint4x1 xint4x2 xint4x3 xint4x4

float
float float1 float2 float3 float4 float1x1 float1x2 float1x3 float1x4 float2x1 float2x2 float2x3 float2x4 float3x1
float3x2 float3x3 float3x4 float4x1 float4x2 float4x3 float4x4

half
half half1 half2 half3 half4 half1x1 half1x2 half1x3 half1x4 half2x1 half2x2 half2x3 half2x4 half3x1
half3x2 half3x3 half3x4 half4x1 half4x2 half4x3 half4x4

double double1 double2 double3 double4 double1x1 double1x2 double1x3 double1x4 double2x1 double2x2
double2x3 double2x4 double3x1 double3x2 double3x3 double3x4 double4x1 double4x2 double4x3 double4x4

mat2 mat3 mat4
mat2x2 mat2x3 mat2x4
mat3x2 mat3x3 mat3x4
mat4x2 mat4x3 mat4x4
dmat2 dmat3 dmat4
dmat2x2 dmat2x3 dmat2x4
dmat3x2 dmat3x3 dmat3x4
dmat4x2 dmat4x3 dmat4x4
imat2 imat3 imat4
imat2x2 imat2x3 imat2x4
imat3x2 imat3x3 imat3x4
imat4x2 imat4x3 imat4x4
umat2 umat3 umat4
umat2x2 umat2x3 umat2x4
umat3x2 umat3x3 umat3x4
umat4x2 umat4x3 umat4x4
vec2 vec3 vec4
dvec2 dvec3 dvec4
ivec2 ivec3 ivec4
uvec2 uvec3 uvec4

unormh unormh1 unormh2 unormh3 unormh4
unormb unormb1 unormb2 unormb3 unormb4
snormh snormh1 snormh2 snormh3 snormh4
snormb snormb1 snormb2 snormb3 snormb4
uintten uintten1 uintten2 uintten3 uintten4
unormten unormten1 unormten2 unormten3 unormten4
floateleven floateleven1 floateleven2 floateleven3 floateleven4

row_major column_major unsigned signed rgb
)EOKEYWORDS"};

void ConfigureSyntax(ScintillaEdit *scintilla, int language)
{
  bool hlsl = false;
  bool glsl = false;
  int lexLang = language;

  if(lexLang == SCLEX_HLSL || lexLang == SCLEX_GLSL || lexLang == SCLEX_BUFFER)
    lexLang = SCLEX_CPP;

  scintilla->setLexer(lexLang);

#define SC_COL(qcol) SCINTILLA_COLOUR(qcol.red(), qcol.green(), qcol.blue())

  // set the default style to base/text
  QColor base = scintilla->palette().color(QPalette::Base);
  QColor text = scintilla->palette().color(QPalette::Text);
  scintilla->styleSetBack(STYLE_DEFAULT, SC_COL(base));
  scintilla->styleSetFore(STYLE_DEFAULT, SC_COL(text));

  scintilla->setCaretFore(SC_COL(text));

  // default all lexer styles up to STYLE_DEFAULT as the same, then override per-colour below
  for(sptr_t i = 0; i < STYLE_DEFAULT; i++)
  {
    scintilla->styleSetBack(i, SC_COL(base));
    scintilla->styleSetFore(i, SC_COL(text));
  }

  // set highlight text colour
  QColor highlight = scintilla->palette().color(QPalette::Highlight);
  QColor highlightedText = scintilla->palette().color(QPalette::HighlightedText);
  scintilla->setSelBack(true, SC_COL(highlight));
  scintilla->setSelFore(true, SC_COL(highlightedText));

  // set margin colours
  QColor window = scintilla->palette().color(QPalette::Window);
  QColor windowText = scintilla->palette().color(QPalette::WindowText);
  for(sptr_t i = 0; i < 5; i++)
    scintilla->setMarginBackN(i, SC_COL(window));
  scintilla->styleSetBack(STYLE_LINENUMBER, SC_COL(window));
  scintilla->styleSetFore(STYLE_LINENUMBER, SC_COL(windowText));

  sptr_t blue = IsDarkTheme() ? SCINTILLA_COLOUR(105, 105, 255) : SCINTILLA_COLOUR(0, 0, 150);
  sptr_t magenta = IsDarkTheme() ? SCINTILLA_COLOUR(255, 105, 255) : SCINTILLA_COLOUR(150, 0, 150);
  sptr_t rouge = IsDarkTheme() ? SCINTILLA_COLOUR(255, 150, 150) : SCINTILLA_COLOUR(175, 70, 70);

  // works for either dark or light
  sptr_t green = SCINTILLA_COLOUR(0, 150, 0);
  sptr_t teal = SCINTILLA_COLOUR(0, 150, 150);
  sptr_t olive = SCINTILLA_COLOUR(150, 150, 0);

  if(lexLang == SCLEX_CPP)
  {
    scintilla->setProperty("lexer.cpp.track.preprocessor", "0");
    scintilla->setProperty("styling.within.preprocessor", "1");

    scintilla->styleSetFore(SCE_C_COMMENT, green);
    scintilla->styleSetFore(SCE_C_COMMENTDOC, green);
    scintilla->styleSetFore(SCE_C_COMMENTLINE, green);
    scintilla->styleSetFore(SCE_C_WORD, blue);
    scintilla->styleSetFore(SCE_C_WORD2, blue);
    scintilla->styleSetFore(SCE_C_PREPROCESSOR, blue);
    scintilla->styleSetBold(SCE_C_PREPROCESSOR, true);

    if(language == SCLEX_HLSL)
    {
      scintilla->setKeyWords(0, hlsl_keywords[0]);
      scintilla->setKeyWords(1, hlsl_keywords[1]);
    }
    else if(language == SCLEX_GLSL)
    {
      scintilla->setKeyWords(0, glsl_keywords[0]);
      scintilla->setKeyWords(1, glsl_keywords[1]);
    }
    else if(language == SCLEX_BUFFER)
    {
      scintilla->setKeyWords(0, buffer_keywords[0]);
      scintilla->setKeyWords(1, buffer_keywords[1]);
    }
  }
  else if(language == SCLEX_PYTHON)
  {
    scintilla->setProperty("tab.timmy.whinge.level", "1");
    scintilla->setProperty("fold", "1");

    scintilla->setKeyWords(0, python_keywords);

    scintilla->styleSetFore(SCE_P_COMMENTLINE, green);
    scintilla->styleSetFore(SCE_P_COMMENTBLOCK, green);
    scintilla->styleSetFore(SCE_P_NUMBER, teal);
    scintilla->styleSetFore(SCE_P_STRING, magenta);
    scintilla->styleSetFore(SCE_P_TRIPLE, rouge);
    scintilla->styleSetFore(SCE_P_TRIPLEDOUBLE, rouge);
    scintilla->styleSetFore(SCE_P_CHARACTER, magenta);
    scintilla->styleSetFore(SCE_P_DEFNAME, olive);
    scintilla->styleSetFore(SCE_P_CLASSNAME, magenta);
    scintilla->styleSetFore(SCE_P_WORD, blue);
    scintilla->styleSetFore(SCE_P_WORD2, blue);
    scintilla->styleSetBold(SCE_P_WORD, true);
    scintilla->styleSetBold(SCE_P_WORD2, true);
  }
}
