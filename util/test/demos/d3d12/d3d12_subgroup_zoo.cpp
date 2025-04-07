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

#include "3rdparty/fmt/core.h"
#include "d3d12_test.h"

RD_TEST(D3D12_Subgroup_Zoo, D3D12GraphicsTest)
{
  static constexpr const char *Description =
      "Test of behaviour around subgroup operations in shaders.";

  const std::string common = R"EOSHADER(

cbuffer rootconsts : register(b0)
{
  uint root_test;
}

#define IsTest(x) (root_test == x)

)EOSHADER";

  const std::string vertex = common + R"EOSHADER(

struct OUT
{
  float4 pos : SV_Position;
  float4 data : DATA;
};

OUT main(uint vert : SV_VertexID)
{
  OUT ret = (OUT)0;

  float2 positions[] = {
    float2(-1.0f,  1.0f),
    float2( 1.0f,  1.0f),
    float2(-1.0f, -1.0f),
    float2( 1.0f, -1.0f),
  };

  float scale = 1.0f;
  if(IsTest(2))
    scale = 0.2f;

  ret.pos = float4(positions[vert]*float2(scale,scale), 0, 1);

  ret.data = 0.0f.xxxx;

  uint wave = WaveGetLaneIndex();

  if(IsTest(0))
    ret.data = float4(wave, 0, 0, 1);
  else if(IsTest(3))
    ret.data = float4(WaveActiveSum(wave), 0, 0, 0);

  return ret;
}

)EOSHADER";

  const std::string pixel = common + R"EOSHADER(

struct IN
{
  float4 pos : SV_Position;
  float4 data : DATA;
};

float4 main(IN input) : SV_Target0
{
  uint subgroupId = WaveGetLaneIndex();

  float4 pixdata = 0.0f.xxxx;

  if(IsTest(1) || IsTest(2))
  {
    pixdata = float4(subgroupId, 0, 0, 1);
  }
  else if(IsTest(4))
  {
    pixdata = float4(WaveActiveSum(subgroupId), 0, 0, 0);
  }
  else if(IsTest(5))
  {
    // QuadReadLaneAt : unit tests
    pixdata.x = float(QuadReadLaneAt(subgroupId, 0));
    pixdata.y = float(QuadReadLaneAt(subgroupId, 1));
    pixdata.z = float(QuadReadLaneAt(subgroupId, 2));
    pixdata.w = float(QuadReadLaneAt(subgroupId, 3));
  }
  else if(IsTest(6))
  {
    // QuadReadAcrossDiagonal, QuadReadAcrossX, QuadReadAcrossY: unit tests
    pixdata.x = float(QuadReadAcrossDiagonal(subgroupId));
    pixdata.y = float(QuadReadAcrossX(subgroupId));
    pixdata.z = float(QuadReadAcrossY(subgroupId));
    pixdata.w = QuadReadLaneAt(pixdata.x, 2);
  }

  return input.data + pixdata;
}

)EOSHADER";

  const std::string comp = common + R"EOSHADER(

RWStructuredBuffer<float4> outbuf : register(u0);

static uint3 tid;

float4 funcD(uint id)
{
  return WaveActiveSum(id/2).xxxx;
}

float4 nestedFunc(uint id)
{
  float4 ret = funcD(id/3);
  ret.w = WaveActiveSum(id);
  return ret;
}

float4 funcA(uint id)
{
   return nestedFunc(id*2);
}

float4 funcB(uint id)
{
   return nestedFunc(id*4);
}

float4 funcTest(uint id)
{
  if ((id % 2) == 0)
  {
    return 0.xxxx;
  }
  else
  {
    float value = WaveActiveSum(id);
    if (id < 10)
    {
      return value.xxxx;
    }
    value += WaveActiveSum(id/2);
    return value.xxxx;
  }
}

void SetOuput(float4 data)
{
  outbuf[root_test * 1024 + tid.y * GROUP_SIZE_X + tid.x] = data;
}

[numthreads(GROUP_SIZE_X, GROUP_SIZE_Y, 1)]
void main(uint3 inTid : SV_DispatchThreadID)
{
  float4 data = 0.0f.xxxx;
  tid = inTid;

  uint id = WaveGetLaneIndex();

  SetOuput(id);

  if(IsTest(0))
  {
    data.x = id;
  }
  else if(IsTest(1))
  {
    data.x = WaveActiveSum(id);
  }
  else if(IsTest(2))
  {
    // Diverged threads which reconverge 
    if (id < 10)
    {
        // active threads 0-9
        data.x = WaveActiveSum(id);

        if ((id % 2) == 0)
          data.y = WaveActiveSum(id);
        else
          data.y = WaveActiveSum(id);

        data.x += WaveActiveSum(id);
    }
    else
    {
        // active threads 10...
        data.x = WaveActiveSum(id);
    }
    data.y = WaveActiveSum(id);
  }
  else if(IsTest(3))
  {
    // Converged threads calling a function 
    data = funcTest(id);
    data.y = WaveActiveSum(id);
  }
  else if(IsTest(4))
  {
    // Converged threads calling a function which has a nested function call in it
    data = nestedFunc(id);
    data.y = WaveActiveSum(id);
  }
  else if(IsTest(5))
  {
    // Diverged threads calling the same function
    if (id < 10)
    {
      data = funcD(id);
    }
    else
    {
      data = funcD(id);
    }
    data.y = WaveActiveSum(id);
  }
  else if(IsTest(6))
  {
    // Diverged threads calling the same function which has a nested function call in it
    if (id < 10)
    {
      data = funcA(id);
    }
    else
    {
      data = funcB(id);
    }
    data.y = WaveActiveSum(id);
  }
  else if(IsTest(7))
  {
    // Diverged threads which early exit
    if (id < 10)
    {
      data.x = WaveActiveSum(id+10);
      SetOuput(data);
      return;
    }
    data.x = WaveActiveSum(id);
  }
  else if(IsTest(8))
  {
     // Loops with different number of iterations per thread
    for (uint i = 0; i < id; i++)
    {
      data.x += WaveActiveSum(id);
    }
  }
  else if(IsTest(9))
  {
    // Query functions : unit tests
    data.x = float(WaveGetLaneCount());
    data.y = float(WaveGetLaneIndex());
    data.z = float(WaveIsFirstLane());
  }
  else if(IsTest(10))
  {
    // Vote functions : unit tests
    data.x = float(WaveActiveAnyTrue(id*2 > id+10));
    data.y = float(WaveActiveAllTrue(id < WaveGetLaneCount()));
    if (id > 10)
    {
      data.z = float(WaveActiveAllTrue(id > 10));
      uint4 ballot = WaveActiveBallot(id > 20);
      data.w = countbits(ballot.x) + countbits(ballot.y) + countbits(ballot.z) + countbits(ballot.w);
    }
    else
    {
      data.z = float(WaveActiveAllTrue(id > 3));
      uint4 ballot = WaveActiveBallot(id > 4);
      data.w = countbits(ballot.x) + countbits(ballot.y) + countbits(ballot.z) + countbits(ballot.w);
    }
  }
  else if(IsTest(11))
  {
    // Broadcast functions : unit tests
    if (id >= 2 && id <= 20)
    {
      data.x = WaveReadLaneFirst(id);
      data.y = WaveReadLaneAt(id, 5);
      data.z = WaveReadLaneAt(id, id);
      data.w = WaveReadLaneAt(data.x, 2+id%3);
    }
  }
  else if(IsTest(12))
  {
    // Scan and Prefix functions : unit tests
    if (id >= 2 && id <= 20)
    {
      data.x = WavePrefixCountBits(id > 4);
      data.y = WavePrefixCountBits(id > 10);
      data.z = WavePrefixSum(data.x);
      data.w = WavePrefixProduct(1 + data.y);
    }
  }
  else if(IsTest(13))
  {
    // Reduction functions : unit tests
    if (id >= 2 && id <= 20)
    {
      data.x = float(WaveActiveMax(id));
      data.y = float(WaveActiveMin(id));
      data.z = float(WaveActiveProduct(id));
      data.w = float(WaveActiveSum(id));
    }
  }
  else if(IsTest(14))
  {
    // Reduction functions : unit tests
    if (id >= 2 && id <= 20)
    {
      data.x = float(WaveActiveCountBits(id > 23));
      data.y = float(WaveActiveBitAnd(id));
      data.z = float(WaveActiveBitOr(id));
      data.w = float(WaveActiveBitXor(id));
    }
  }
  else if(IsTest(15))
  {
    // Reduction functions : unit tests
    if (id > 13)
    {
      bool test1 = (id > 15).x;
      bool2 test2 = bool2(test1, (id < 23));
      bool3 test3 = bool3(test1, (id < 23), (id >= 25));
      bool4 test4 = bool4(test1, (id < 23), (id >= 25), (id >= 28));

      data.x = float(WaveActiveAllEqual(test1).x);
      data.y = float(WaveActiveAllEqual(test2).y);
      data.z = float(WaveActiveAllEqual(test3).z);
      data.w = float(WaveActiveAllEqual(test4).w);
    }
  }
  SetOuput(data);
}

)EOSHADER";

  void Prepare(int argc, char **argv)
  {
    D3D12GraphicsTest::Prepare(argc, argv);

    if(opts1.WaveLaneCountMax < 16)
      Avail = "Subgroup size is less than 16";
  }

  int main()
  {
    // initialise, create window, create device, etc
    if(!Init())
      return 3;

    ID3D12RootSignaturePtr sig = MakeSig({constParam(D3D12_SHADER_VISIBILITY_ALL, 0, 0, 1),
                                          uavParam(D3D12_SHADER_VISIBILITY_ALL, 0, 0)});

    const uint32_t imgDim = 128;

    ID3D12ResourcePtr fltTex = MakeTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, imgDim, imgDim)
                                   .RTV()
                                   .InitialState(D3D12_RESOURCE_STATE_RENDER_TARGET);
    fltTex->SetName(L"fltTex");
    D3D12_CPU_DESCRIPTOR_HANDLE fltRTV = MakeRTV(fltTex).CreateCPU(0);
    D3D12_GPU_DESCRIPTOR_HANDLE fltSRV = MakeSRV(fltTex).CreateGPU(8);

    int vertTests = 0, pixTests = 0;
    int numCompTests = 0;

    {
      size_t pos = 0;
      while(pos != std::string::npos)
      {
        pos = pixel.find("IsTest(", pos);
        if(pos == std::string::npos)
          break;
        pos += sizeof("IsTest(") - 1;
        pixTests = std::max(pixTests, atoi(pixel.c_str() + pos) + 1);
      }

      pos = 0;
      while(pos != std::string::npos)
      {
        pos = vertex.find("IsTest(", pos);
        if(pos == std::string::npos)
          break;
        pos += sizeof("IsTest(") - 1;
        vertTests = std::max(vertTests, atoi(vertex.c_str() + pos) + 1);
      }

      pos = 0;
      while(pos != std::string::npos)
      {
        pos = comp.find("IsTest(", pos);
        if(pos == std::string::npos)
          break;
        pos += sizeof("IsTest(") - 1;
        numCompTests = std::max(numCompTests, atoi(comp.c_str() + pos) + 1);
      }
    }

    const uint32_t numGraphicsTests = std::max(vertTests, pixTests);

    struct
    {
      int x, y;
    } compsize[] = {
        {256, 1},
        {128, 2},
        {8, 128},
        {150, 1},
    };
    std::string comppipe_name[ARRAY_COUNT(compsize)];
    ID3D12PipelineStatePtr comppipe[ARRAY_COUNT(compsize)];

    std::string defines;
    defines += fmt::format("#define COMP_TESTS {}\n", numCompTests);
    defines += "\n";

    ID3D12PipelineStatePtr graphics = MakePSO()
                                          .RootSig(sig)
                                          .VS(Compile(defines + vertex, "main", "vs_6_0"))
                                          .PS(Compile(defines + pixel, "main", "ps_6_0"))
                                          .RTVs({DXGI_FORMAT_R32G32B32A32_FLOAT});

    for(int i = 0; i < ARRAY_COUNT(comppipe); i++)
    {
      std::string sizedefine;
      sizedefine = fmt::format("#define GROUP_SIZE_X {}\n#define GROUP_SIZE_Y {}\n", compsize[i].x,
                               compsize[i].y);
      comppipe_name[i] = fmt::format("{}x{}", compsize[i].x, compsize[i].y);

      comppipe[i] =
          MakePSO().RootSig(sig).CS(Compile(defines + sizedefine + comp, "main", "cs_6_0"));
      comppipe[i]->SetName(UTF82Wide(comppipe_name[i]).c_str());
    }

    ID3D12ResourcePtr bufOut = MakeBuffer().Size(sizeof(Vec4f) * 1024 * numCompTests).UAV();
    D3D12ViewCreator uavView =
        MakeUAV(bufOut).Format(DXGI_FORMAT_R32_UINT).NumElements(4 * 1024 * numCompTests);
    D3D12_CPU_DESCRIPTOR_HANDLE uavcpu = uavView.CreateClearCPU(10);
    D3D12_GPU_DESCRIPTOR_HANDLE uavgpu = uavView.CreateGPU(10);

    bufOut->SetName(L"bufOut");

    while(Running())
    {
      ID3D12GraphicsCommandListPtr cmd = GetCommandBuffer();

      Reset(cmd);

      cmd->SetDescriptorHeaps(1, &m_CBVUAVSRV.GetInterfacePtr());

      ID3D12ResourcePtr bb = StartUsingBackbuffer(cmd, D3D12_RESOURCE_STATE_RENDER_TARGET);

      ClearRenderTargetView(cmd, BBRTV, {0.2f, 0.2f, 0.2f, 1.0f});

      cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

      cmd->SetPipelineState(graphics);
      cmd->SetGraphicsRootSignature(sig);

      RSSetViewport(cmd, {0.0f, 0.0f, (float)imgDim, (float)imgDim, 0.0f, 1.0f});
      RSSetScissorRect(cmd, {0, 0, imgDim, imgDim});

      pushMarker(cmd, "Graphics Tests");

      for(uint32_t i = 0; i < numGraphicsTests; i++)
      {
        ResourceBarrier(cmd);

        OMSetRenderTargets(cmd, {fltRTV}, {});
        ClearRenderTargetView(cmd, fltRTV, {123456.0f, 789.0f, 101112.0f, 0.0f});

        cmd->SetGraphicsRoot32BitConstant(0, i, 0);
        cmd->DrawInstanced(4, 1, 0, 0);
      }

      popMarker(cmd);

      pushMarker(cmd, "Compute Tests");

      for(size_t p = 0; p < ARRAY_COUNT(comppipe); p++)
      {
        ResourceBarrier(cmd);

        UINT zero[4] = {};
        cmd->ClearUnorderedAccessViewUint(uavgpu, uavcpu, bufOut, zero, 0, NULL);

        ResourceBarrier(cmd);
        pushMarker(cmd, comppipe_name[p]);

        cmd->SetPipelineState(comppipe[p]);
        cmd->SetComputeRootSignature(sig);
        cmd->SetComputeRootUnorderedAccessView(1, bufOut->GetGPUVirtualAddress());

        for(int i = 0; i < numCompTests; i++)
        {
          cmd->SetComputeRoot32BitConstant(0, i, 0);
          cmd->Dispatch(1, 1, 1);
        }

        popMarker(cmd);
      }

      popMarker(cmd);

      FinishUsingBackbuffer(cmd, D3D12_RESOURCE_STATE_RENDER_TARGET);

      cmd->Close();

      SubmitAndPresent({cmd});
    }

    return 0;
  }
};

REGISTER_TEST();
