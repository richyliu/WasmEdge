# Fuzzing WasmEdge

## TODO

Investigate AOT section (can we inject malicious code?)

## Building

Create a `build` directory and enter it.

Configure project to use cmake and libfuzzer

debug mode and no fuzzer:
``` sh
cmake \
  -DCMAKE_C_COMPILER=clang-12 \
  -DCMAKE_CXX_COMPILER=clang++-12 \
  -DCMAKE_C_FLAGS=-fno-omit-frame-pointer \
  -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer \
  -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld-12 -rdynamic" \
  -DWASMEDGE_BUILD_TOOLS=OFF \
  -DWASMEDGE_BUILD_AOT_RUNTIME=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  ..
```

Release with debug info with libfuzzer and asan:
``` sh
cmake \
  -DCMAKE_C_COMPILER=clang-12 \
  -DCMAKE_CXX_COMPILER=clang++-12 \
  -DCMAKE_C_FLAGS=-fsanitize="fuzzer-no-link,address" \
  -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link,address" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld-12" \
  -DWASMEDGE_BUILD_TOOLS=OFF \
  -DWASMEDGE_BUILD_AOT_RUNTIME=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  ..
```

debug mode with libfuzzer and asan:
``` sh
cmake \
  -DCMAKE_C_COMPILER=clang-12 \
  -DCMAKE_CXX_COMPILER=clang++-12 \
  -DCMAKE_C_FLAGS=-fsanitize="fuzzer-no-link,address" \
  -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link,address" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld-12" \
  -DWASMEDGE_BUILD_TOOLS=OFF \
  -DWASMEDGE_BUILD_AOT_RUNTIME=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  ..
```

(with UBSan:)
``` sh
cmake \
  -DCMAKE_C_COMPILER=clang-12 \
  -DCMAKE_CXX_COMPILER=clang++-12 \
  -DCMAKE_C_FLAGS=-fsanitize="fuzzer-no-link,address,undefined" \
  -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link,address,undefined" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld-12 -lubsan" \
  -DWASMEDGE_BUILD_TOOLS=OFF \
  -DWASMEDGE_BUILD_AOT_RUNTIME=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  ..
```

Then, run `make` (or `make -j` to use multiple threads).

## 0-days (exploitable bugs)

### No offset oob check

Poc in file `poc/oob_arbitrary_write_crash-724754a922c4e8c5981dcbc27a2fc4b684bf0ec2`.

Arbitrary write caused by `std::copy` at
`/home/richyliu/WasmEdge/lib/loader/shared_library.cpp:101`, which does not
check for bounds on `Offset`. This allows for arbitrary write relative to
`Binary` (mmap-ed chunk).

I can make multiple writes and make arbitrary mmap regions executable.

Since it is relative to a mmap chunk, I essentially have write access to all mmap chunks and the libc region

Another poc in `poc/crash-b1bd75785aab4a6da41e967804f17dca6c81c109` (only
causes a crash sometimes).

All caused by integer overflow/oob write.

```
#5  0x00007ffff7dfc68f in WasmEdge::Loader::SharedLibrary::load (this=0x555555570e70, AOTSec=...) at /home/richyliu/WasmEdge/lib/loader/shared_library.cpp:111
#6  0x00007ffff7dd47dc in WasmEdge::Loader::Loader::loadModule (this=0x5555555702a8) at /home/richyliu/WasmEdge/lib/loader/ast/module.cpp:277
#7  0x00007ffff7dcd3b5 in WasmEdge::Loader::Loader::parseModule (this=0x5555555702a8, Code=...) at /home/richyliu/WasmEdge/lib/loader/loader.cpp:163
#8  0x00007ffff7daa159 in WasmEdge::VM::VM::unsafeRunWasmFile(cxx20::span<unsigned char const, 18446744073709551615ul>, std::basic_string_view<char, std::char_traits<char> >, cxx20::span<WasmEdge::Varia
```

## Current bugs

### Large allocations

FIXED

Several large allocations have been detected. Some of these can be easily fixed
with bounds checks. See `poc/` folder for proof of concepts.

Most of these are due to lack of bounds check before a `resize` or `reserve` call.

(includes the errors in `crashes_5_length_error`)

### Large locals

TEMP FIX

`slow-unit-60999cd28e75bac80a8e2af86dda893f3c567381` has a lot of locals,
causing a DOS as the validation code tries to process it.

The validator (`lib/validator/validator.cpp:217`) adds each local to an array
for type checking. This used to be the approach of bytecode alliance's
wasm-tools as well, but they [changed it](wasm-tools-commit) to a version that
binary searches the locals array instead of going through it one by one.

To implement that here, we would have to change the logic in
`lib/loader/ast/segment.cpp:228-267` and in the validator.cpp specified above.

[wasm-tools-commit]: https://github.com/bytecodealliance/wasm-tools/commit/7811832a5be385f63f7465bb20137471576a378f#diff-48cb170301e575b8bb87caf250dcb16b4dab337304adfdb486a0eee542166b92L358-R158

### Stack overflow

TEMP FIX

`poc/oom-167f7169c9d516f7ae9b65ccd2fd37b63b2d934a` and
`poc/oom-99771daaaa3060a54975fdf294994d8db80e2a40` are stack overflow bugs.

Add stack overflow mechanism (see `include/runtime/stackmgr.h:69,88`)
(done, added to `push`; need to add to `pushFrame` as well)

Add test to `test/spec/spectest.cpp:614`

### Stack expansion integer overflow

FIXED

(probably just trying to grow a large memory region)

`poc/crash-607c3815baaceaf9d686a4278f3eb049edab7751`

stack trace:

```
#0  __GI_raise (sig=sig@entry=6) at ../sysdeps/unix/sysv/linux/raise.c:50
#1  0x00007ffff6f43859 in __GI_abort () at abort.c:79
#2  0x00007ffff7c1eaf0 in WasmEdge::Allocator::resize (Pointer=<optimized out>, OldPageCount=256, NewPageCount=<optimized out>) at /home/richyliu/WasmEdge/lib/system/allocator.cpp:105
#3  0x00007ffff78cb997 in WasmEdge::Runtime::Instance::MemoryInstance::growPage (this=<optimized out>, Count=240) at /home/richyliu/WasmEdge/include/runtime/instance/memory.h:113
#4  0x00007ffff7a83115 in WasmEdge::Executor::Executor::runMemoryGrowOp (this=<optimized out>, StackMgr=..., MemInst=...) at /home/richyliu/WasmEdge/lib/executor/engine/memoryInstr.cpp:25
#5  0x00007ffff79cbb66 in WasmEdge::Executor::Executor::execute(WasmEdge::Runtime::StackManager&, WasmEdge::AST::Instruction const*, WasmEdge::AST::Instruction const*)::$_0::operator()() const (
    this=0x7fffffffcc20) at /home/richyliu/WasmEdge/lib/executor/engine/engine.cpp:278
#6  0x00007ffff79c5a8c in WasmEdge::Executor::Executor::execute (this=<optimized out>, StackMgr=..., Start=<optimized out>, End=<optimized out>) at /home/richyliu/WasmEdge/lib/executor/engine/engine.cpp:1819
```

### None type value

FIXED

Webassembly does not define a `None` type. However, WasmEdge uses a `None` type
internally. Its value is defined to be 0x40. Malicious programs can use this as
the value type and cause a crash.

See `crashes_5/` folder for inputs

### `munmap_chunk(): invalid pointer`

TRIAGED

Very strange bug that only disappears when running with `-fsanitize=address`.
See `poc/crash-20588e176ffd6c07fbe00e4170727cdd6639e56a`. Triggered by the
`Statistics` destructor in `include/common/statistics.h:41` when it tries to
destruct `CostTab`.

Possibly due to uninitialized memory? (Since ASAN initializes everything)

Later triaged as another manifestation of the "no offset check `std::copy`" bug.
This can appear as different bugs if the write overflows into the `Statistics`
vector (which is very likely).
