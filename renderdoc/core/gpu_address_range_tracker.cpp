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

#include "core/gpu_address_range_tracker.h"
#include "api/replay/replay_enums.h"
#include "common/formatting.h"
#include "core/settings.h"

void GPUAddressRangeTracker::AddTo(const GPUAddressRange &range)
{
  SCOPED_WRITELOCK(addressLock);

  // insert ranges ordered by start first, then by size. Ranges with different sizes starting at the
  // same point will be ordered such that the last one is largest

  // search for the range. This will return the largest range which starts before or at this address
  size_t idx = FindLastRangeBeforeOrAtAddress(range.start);

  // if we search for an address that's past the end of the last range, we'll return that index. The
  // only case where we return no valid index is if the address is before the first range - so
  // insert ours at the start of the list and return
  if(idx == ~0U)
  {
    addresses.insert(0, range);
    return;
  }

  // if the range found doesn't start at the same point as us, insert immediately so we preserve the
  // sorting by range start
  if(addresses[idx].start != range.start)
  {
    addresses.insert(idx + 1, range);
    return;
  }

  // we get here if the range starts at the same point as us, so we need to sort by size.
  // if we are smaller than the found range, move backwards to insert before it. Keep going as long
  // as we're looking at ranges that start at the same address and are larger than us
  while(addresses[idx].start == range.start && addresses[idx].realEnd > range.realEnd)
  {
    // we could be smaller than the very first range in the list. If that's the case, insert at 0 and return now
    if(idx == 0)
    {
      addresses.insert(0, range);
      return;
    }

    // otherwise move backwards, to insert before the current range
    idx--;
  }

  // insert after the idx we arrived at, which is the first range either starting before, or that is smaller than us
  addresses.insert(idx + 1, range);
}

void GPUAddressRangeTracker::RemoveFrom(const GPUAddressRange &range)
{
  {
    SCOPED_WRITELOCK(addressLock);

    // search for the range. This will return the largest range which starts before or at this address
    size_t idx = FindLastRangeBeforeOrAtAddress(range.start);

    if(idx != ~0U)
    {
      // there might be multiple buffers with the same range start, find the exact range for this
      // buffer. We only have to search backwards because we returned the largest (aka last) range before this address
      while(addresses[idx].start == range.start)
      {
        if(addresses[idx].id == range.id)
        {
          addresses.erase(idx);
          return;
        }

        if(idx == 0)
          break;

        --idx;
      }
    }
  }

  // used only so the tests can EXPECT_ERROR()
  RDResult err;
  SET_ERROR_RESULT(err, ResultCode::InternalError, "Couldn't find matching range to remove for %s",
                   ToStr(range.id).c_str());
  (void)err;
}

void GPUAddressRangeTracker::Clear()
{
  SCOPED_WRITELOCK(addressLock);
  addresses.clear();
}

rdcarray<GPUAddressRange> GPUAddressRangeTracker::GetAddresses()
{
  SCOPED_READLOCK(addressLock);
  return addresses;
}

rdcarray<ResourceId> GPUAddressRangeTracker::GetIDs()
{
  rdcarray<ResourceId> ret;
  ret.reserve(addresses.size());

  {
    SCOPED_READLOCK(addressLock);
    for(size_t i = 0; i < addresses.size(); i++)
      ret.push_back(addresses[i].id);
  }

  return ret;
}

size_t GPUAddressRangeTracker::FindLastRangeBeforeOrAtAddress(GPUAddressRange::Address addr)
{
  // the caller must lock.

  if(addresses.empty())
    return ~0U;

  // start looking at the whole range
  size_t first = 0;
  size_t count = addresses.size();

  while(count > 1)
  {
    // look at the midpoint
    size_t halfrange = count / 2;
    size_t mid = first + halfrange;

    // if the midpoint is after our address, bisect down to the lower half and exclude the midpoint
    if(addr < addresses[mid].start)
    {
      count = halfrange;
    }
    else
    {
      // midpoint is before or at our address, use upper half
      first = mid;
      count -= halfrange;
    }
  }

  // if first is 0 and the address range doesn't match, indicate that by returning ~0U
  if(first == 0 && addr < addresses[first].start)
    return ~0U;

  return first;
}

template <bool allowOOB>
void GPUAddressRangeTracker::GetResIDFromAddr(GPUAddressRange::Address addr, ResourceId &id,
                                              uint64_t &offs)
{
  id = ResourceId();
  offs = 0;

  if(addr == 0)
    return;

  GPUAddressRange range;

  {
    SCOPED_READLOCK(addressLock);

    // search for the address. This will return the largest range which starts before or at this address
    size_t idx = FindLastRangeBeforeOrAtAddress(addr);

    // ~0U is returned if the address is before the first range in our list. That means no match
    if(idx == ~0U)
      return;

    // this range is already the largest before or at the address by virtue of our sorting and search
    range = addresses[idx];
  }

  if(addr < range.start)
    return;

  // if OOB isn't allowed, check against real end
  if(!allowOOB)
  {
    if(addr >= range.realEnd)
      return;
  }

  // always check against OOB end
  if(addr >= range.oobEnd)
    return;

  id = range.id;
  offs = addr - range.start;
}

template void GPUAddressRangeTracker::GetResIDFromAddr<false>(GPUAddressRange::Address addr,
                                                              ResourceId &id, uint64_t &offs);
template void GPUAddressRangeTracker::GetResIDFromAddr<true>(GPUAddressRange::Address addr,
                                                             ResourceId &id, uint64_t &offs);

void GPUAddressRangeTracker::GetResIDBoundForAddr(GPUAddressRange::Address addr, ResourceId &lower,
                                                  GPUAddressRange::Address &lowerVA,
                                                  ResourceId &upper,
                                                  GPUAddressRange::Address &upperVA)
{
  lower = upper = ResourceId();
  lowerVA = upperVA = 0;

  if(addr == 0)
    return;

  {
    SCOPED_READLOCK(addressLock);

    if(addresses.empty())
      return;

    size_t idx = FindLastRangeBeforeOrAtAddress(addr);

    // if the addr is before first known range, it's bounded on upper only
    if(idx == ~0U)
    {
      upper = addresses[idx].id;
      upperVA = addresses[idx].start;
      return;
    }

    lower = addresses[idx].id;
    lowerVA = addresses[idx].start;

    // if this range contains the address exactly, return it as a tight bound
    if(addresses[idx].realEnd > addr)
    {
      upper = addresses[idx].id;
      upperVA = addresses[idx].realEnd;
      return;
    }

    // otherwise the address is past its end but before the next. Move one allocation along - we
    // already know that we picked the largest allocation that covers this address
    idx++;

    // if this wasn't the end, return the upper bound
    if(idx < addresses.size())
    {
      upper = addresses[idx].id;
      upperVA = addresses[idx].realEnd;
    }
  }
}

#if ENABLED(ENABLE_UNIT_TESTS)

#undef None
#undef Always

#include "catch/catch.hpp"

namespace TestIDs
{
ResourceId a = ResourceIDGen::GetNewUniqueID();
ResourceId b = ResourceIDGen::GetNewUniqueID();
ResourceId c = ResourceIDGen::GetNewUniqueID();
ResourceId d = ResourceIDGen::GetNewUniqueID();
ResourceId e = ResourceIDGen::GetNewUniqueID();
ResourceId f = ResourceIDGen::GetNewUniqueID();
ResourceId g = ResourceIDGen::GetNewUniqueID();
};

template <>
rdcstr DoStringise(const rdcpair<ResourceId, uint64_t> &el)
{
  using namespace TestIDs;

  rdcarray<ResourceId> ids = {a, b, c, d, e, f, g};
  rdcstr idname = "a";
  int idx = ids.indexOf(el.first);
  if(idx >= 0)
    idname[0] += (char)idx;
  else if(el.first == ResourceId())
    idname[0] = '-';
  else
    idname = "?";

  return "{ " + idname + ", " + StringFormat::Fmt("%#x", el.second) + " }";
}

static GPUAddressRange MakeRange(ResourceId id, GPUAddressRange::Address addr, uint64_t size,
                                 uint64_t oobPadding = 0)
{
  return {
      addr,
      addr + size,
      addr + size + oobPadding,
      id,
  };
}

rdcpair<ResourceId, uint64_t> make_idoffs(ResourceId a, uint64_t b)
{
  return {a, b};
}

TEST_CASE("Check GPUAddressRangeTracker", "[gpuaddr]")
{
  GPUAddressRangeTracker tracker;

  rdcpair<ResourceId, uint64_t> none = make_idoffs(ResourceId(), 0ULL);

  using namespace TestIDs;

  SECTION("Basics")
  {
    tracker.AddTo(MakeRange(a, 0x1230000, 128));
    tracker.AddTo(MakeRange(b, 0x1250000, 128));

    CHECK(tracker.GetResIDFromAddr(0) == none);

    CHECK(tracker.GetResIDFromAddr(0x1230000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(a, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230001) == make_idoffs(a, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 127) == make_idoffs(a, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 128) == none);

    CHECK(tracker.GetResIDFromAddr(0x1250000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000) == make_idoffs(b, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1250001) == make_idoffs(b, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 127) == make_idoffs(b, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 128) == none);

    tracker.RemoveFrom(MakeRange(b, 0x1250000, 128));

    CHECK(tracker.GetResIDFromAddr(0x1230000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(a, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230001) == make_idoffs(a, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 127) == make_idoffs(a, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 128) == none);

    CHECK(tracker.GetResIDFromAddr(0x1250000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250001) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 127) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 128) == none);

    tracker.AddTo(MakeRange(c, 0x1270000, 128));

    CHECK(tracker.GetResIDFromAddr(0x1230000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(a, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230001) == make_idoffs(a, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 127) == make_idoffs(a, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 128) == none);

    CHECK(tracker.GetResIDFromAddr(0x1250000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250001) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 127) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 128) == none);

    CHECK(tracker.GetResIDFromAddr(0x1270000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1270000) == make_idoffs(c, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270001) == make_idoffs(c, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270000 + 127) == make_idoffs(c, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270000 + 128) == none);

    EXPECT_ERROR();

    // wrong ID, don't remove
    tracker.RemoveFrom(MakeRange(g, 0x1270000, 128));

    CHECK(tracker.GetResIDFromAddr(0x1230000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(a, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230001) == make_idoffs(a, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 127) == make_idoffs(a, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 128) == none);

    CHECK(tracker.GetResIDFromAddr(0x1250000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250001) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 127) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 128) == none);

    CHECK(tracker.GetResIDFromAddr(0x1270000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1270000) == make_idoffs(c, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270001) == make_idoffs(c, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270000 + 127) == make_idoffs(c, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270000 + 128) == none);

    EXPECT_ERROR();

    // wrong address, don't remove
    tracker.RemoveFrom(MakeRange(a, 0x1000, 128));

    CHECK(tracker.GetResIDFromAddr(0x1230000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(a, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230001) == make_idoffs(a, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 127) == make_idoffs(a, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1230000 + 128) == none);

    CHECK(tracker.GetResIDFromAddr(0x1250000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250001) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 127) == none);
    CHECK(tracker.GetResIDFromAddr(0x1250000 + 128) == none);

    CHECK(tracker.GetResIDFromAddr(0x1270000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x1270000) == make_idoffs(c, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270001) == make_idoffs(c, 1ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270000 + 127) == make_idoffs(c, 127ULL));
    CHECK(tracker.GetResIDFromAddr(0x1270000 + 128) == none);
  }

  SECTION("Insertion order doesn't affect return value")
  {
    // smallest-to-largest
    tracker.AddTo(MakeRange(a, 0x1230000, 128));
    tracker.AddTo(MakeRange(b, 0x1230000, 256));
    tracker.AddTo(MakeRange(c, 0x1230000, 512));

    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(c, 0ULL));

    tracker.Clear();

    // largest-to-smallest
    tracker.AddTo(MakeRange(c, 0x1230000, 512));
    tracker.AddTo(MakeRange(b, 0x1230000, 256));
    tracker.AddTo(MakeRange(a, 0x1230000, 128));

    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(c, 0ULL));

    tracker.Clear();

    // out-of-order, largest last
    tracker.AddTo(MakeRange(b, 0x1230000, 256));
    tracker.AddTo(MakeRange(a, 0x1230000, 128));
    tracker.AddTo(MakeRange(c, 0x1230000, 512));

    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(c, 0ULL));

    tracker.Clear();

    // out-of-order, smallest last
    tracker.AddTo(MakeRange(b, 0x1230000, 256));
    tracker.AddTo(MakeRange(c, 0x1230000, 512));
    tracker.AddTo(MakeRange(a, 0x1230000, 128));

    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(c, 0ULL));

    tracker.Clear();

    // with a pre-existing address before the ranges
    tracker.AddTo(MakeRange(d, 0x1200000, 512));
    tracker.AddTo(MakeRange(c, 0x1230000, 512));
    tracker.AddTo(MakeRange(b, 0x1230000, 256));
    tracker.AddTo(MakeRange(a, 0x1230000, 128));

    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(c, 0ULL));

    tracker.Clear();

    // with a pre-existing address after the ranges
    tracker.AddTo(MakeRange(d, 0x1250000, 512));
    tracker.AddTo(MakeRange(c, 0x1230000, 512));
    tracker.AddTo(MakeRange(b, 0x1230000, 256));
    tracker.AddTo(MakeRange(a, 0x1230000, 128));

    CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(c, 0ULL));

    tracker.Clear();
  }

  SECTION("OOB")
  {
    tracker.AddTo(MakeRange(a, 0x1230000, 128, 128));

    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230001) == make_idoffs(a, 1ULL));
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230000 + 127) == make_idoffs(a, 127ULL));
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230000 + 128) == make_idoffs(a, 128ULL));
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230000 + 255) == make_idoffs(a, 255ULL));
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230000 + 256) == none);

    tracker.RemoveFrom(MakeRange(a, 0x1230000, 128, 128));

    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230001) == none);
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230000 + 127) == none);
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230000 + 128) == none);
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230000 + 255) == none);
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230000 + 256) == none);

    tracker.AddTo(MakeRange(a, 0x1230000, 0x10000, 0x10000));
    tracker.AddTo(MakeRange(b, 0x1250000, 0x10000, 0x10000));

    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1230001) == make_idoffs(a, 0x1ULL));
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1240000) == make_idoffs(a, 0x10000ULL));
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1240001) == make_idoffs(a, 0x10001ULL));
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x124ffff) == make_idoffs(a, 0x1ffffULL));
    CHECK(tracker.GetResIDFromAddrAllowOutOfBounds(0x1250000) == make_idoffs(b, 0ULL));
  }

  SECTION("co-sited overlap returning largest")
  {
    auto checker = [&tracker, none](ResourceId id) {
      CHECK(tracker.GetResIDFromAddr(0x1230000 - 1) == none);
      CHECK(tracker.GetResIDFromAddr(0x1230000) == make_idoffs(id, 0ULL));
      CHECK(tracker.GetResIDFromAddr(0x1230001) == make_idoffs(id, 1ULL));
      CHECK(tracker.GetResIDFromAddr(0x1230010) == make_idoffs(id, 0x10ULL));
      CHECK(tracker.GetResIDFromAddr(0x1230000 + 127) == make_idoffs(id, 127ULL));

      // check the range of a we expect
      if(id == a)
      {
        CHECK(tracker.GetResIDFromAddr(0x1230000 + 128) == make_idoffs(id, 128ULL));
        CHECK(tracker.GetResIDFromAddr(0x1230000 + 255) == make_idoffs(id, 255ULL));
      }
      else
      {
        CHECK(tracker.GetResIDFromAddr(0x1230000 + 128) == none);
        CHECK(tracker.GetResIDFromAddr(0x1230000 + 255) == none);
      }
    };

    SECTION("big before small")
    {
      tracker.AddTo(MakeRange(a, 0x1230000, 256));
      tracker.AddTo(MakeRange(b, 0x1230000, 128));

      // should find a regardless of added order
      checker(a);

      SECTION("remove a")
      {
        // if a is removed, we now find b
        tracker.RemoveFrom(MakeRange(a, 0x1230000, 256));

        checker(b);
      }

      SECTION("remove b")
      {
        // if b is removed, we still find a
        tracker.RemoveFrom(MakeRange(b, 0x1230000, 128));

        checker(a);
      }
    }

    SECTION("small before big")
    {
      tracker.AddTo(MakeRange(b, 0x1230000, 128));
      tracker.AddTo(MakeRange(a, 0x1230000, 256));

      // should find a regardless of added order
      checker(a);

      SECTION("remove a")
      {
        // if a is removed, we now find b
        tracker.RemoveFrom(MakeRange(a, 0x1230000, 256));

        checker(b);
      }

      SECTION("remove b")
      {
        // if b is removed, we still find a
        tracker.RemoveFrom(MakeRange(b, 0x1230000, 128));

        checker(a);
      }
    }
  }

  SECTION("Partially overlaping ranges that aren't super/subset")
  {
    tracker.AddTo(MakeRange(c, 0x12000, 0x0800));
    tracker.AddTo(MakeRange(d, 0x12600, 0x0800));
    tracker.AddTo(MakeRange(e, 0x12800, 0x0200));

    CHECK(tracker.GetResIDFromAddr(0x12000) == make_idoffs(c, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12100) == make_idoffs(c, 0x100ULL));
    CHECK(tracker.GetResIDFromAddr(0x125ff) == make_idoffs(c, 0x5ffULL));
    CHECK(tracker.GetResIDFromAddr(0x12600) == make_idoffs(d, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12700) == make_idoffs(d, 0x100ULL));
    CHECK(tracker.GetResIDFromAddr(0x127ff) == make_idoffs(d, 0x1ffULL));
    CHECK(tracker.GetResIDFromAddr(0x12800) == make_idoffs(e, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12900) == make_idoffs(e, 0x100ULL));
    CHECK(tracker.GetResIDFromAddr(0x129ff) == make_idoffs(e, 0x1ffULL));
  }

  SECTION("lots of overlap and removals")
  {
    tracker.AddTo(MakeRange(a, 0x12300000, 100));
    tracker.AddTo(MakeRange(b, 0x12300000, 200));
    tracker.AddTo(MakeRange(c, 0x12300000, 300));
    tracker.AddTo(MakeRange(d, 0x12300000, 400));
    tracker.AddTo(MakeRange(e, 0x12300000, 500));
    tracker.AddTo(MakeRange(f, 0x12300000, 600));

    CHECK(tracker.GetResIDFromAddr(0x12300000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x12300000) == make_idoffs(f, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12300f00) == none);

    tracker.RemoveFrom(MakeRange(c, 0x12300000, 300));

    CHECK(tracker.GetResIDFromAddr(0x12300000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x12300000) == make_idoffs(f, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12300f00) == none);

    tracker.RemoveFrom(MakeRange(f, 0x12300000, 600));

    CHECK(tracker.GetResIDFromAddr(0x12300000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x12300000) == make_idoffs(e, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12300f00) == none);

    tracker.RemoveFrom(MakeRange(a, 0x12300000, 100));

    CHECK(tracker.GetResIDFromAddr(0x12300000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x12300000) == make_idoffs(e, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12300f00) == none);

    tracker.RemoveFrom(MakeRange(d, 0x12300000, 100));

    CHECK(tracker.GetResIDFromAddr(0x12300000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x12300000) == make_idoffs(e, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12300f00) == none);

    tracker.RemoveFrom(MakeRange(e, 0x12300000, 100));

    CHECK(tracker.GetResIDFromAddr(0x12300000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x12300000) == make_idoffs(b, 0ULL));
    CHECK(tracker.GetResIDFromAddr(0x12300f00) == none);

    tracker.RemoveFrom(MakeRange(b, 0x12300000, 200));

    CHECK(tracker.GetResIDFromAddr(0x12300000 - 1) == none);
    CHECK(tracker.GetResIDFromAddr(0x12300000) == none);
    CHECK(tracker.GetResIDFromAddr(0x12300f00) == none);
  }
}

#endif
