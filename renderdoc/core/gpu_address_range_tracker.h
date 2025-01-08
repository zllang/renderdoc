/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Baldur Karlsson
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

#include <functional>

#include "api/replay/resourceid.h"
#include "common/threading.h"

struct GPUAddressRange
{
  using Address = uint64_t;

  Address start, realEnd, oobEnd;
  ResourceId id;

  bool operator<(const Address &o) const { return (start < o); }
};

struct GPUAddressRangeTracker
{
  GPUAddressRangeTracker() {}
  // no copying
  GPUAddressRangeTracker(const GPUAddressRangeTracker &) = delete;
  GPUAddressRangeTracker &operator=(const GPUAddressRangeTracker &) = delete;

  void AddTo(const GPUAddressRange &range);
  void RemoveFrom(const GPUAddressRange &range);
  void Clear();
  rdcarray<GPUAddressRange> GetAddresses();
  rdcarray<ResourceId> GetIDs();

  void GetResIDFromAddr(GPUAddressRange::Address addr, ResourceId &id, uint64_t &offs)
  {
    return GetResIDFromAddr<false>(addr, id, offs);
  }
  void GetResIDFromAddrAllowOutOfBounds(GPUAddressRange::Address addr, ResourceId &id, uint64_t &offs)
  {
    return GetResIDFromAddr<true>(addr, id, offs);
  }

  rdcpair<ResourceId, uint64_t> GetResIDFromAddr(GPUAddressRange::Address addr)
  {
    rdcpair<ResourceId, uint64_t> ret;
    GetResIDFromAddr(addr, ret.first, ret.second);
    return ret;
  }
  rdcpair<ResourceId, uint64_t> GetResIDFromAddrAllowOutOfBounds(GPUAddressRange::Address addr)
  {
    rdcpair<ResourceId, uint64_t> ret;
    GetResIDFromAddrAllowOutOfBounds(addr, ret.first, ret.second);
    return ret;
  }
  void GetResIDBoundForAddr(GPUAddressRange::Address addr, ResourceId &lower,
                            GPUAddressRange::Address &lowerVA, ResourceId &upper,
                            GPUAddressRange::Address &upperVA);

private:
  rdcarray<GPUAddressRange> addresses;
  Threading::RWLock addressLock;

  template <bool allowOOB>
  void GetResIDFromAddr(GPUAddressRange::Address addr, ResourceId &id, uint64_t &offs);

  size_t FindLastRangeBeforeOrAtAddress(GPUAddressRange::Address start);
};
