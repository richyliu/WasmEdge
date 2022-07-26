#include <wasmedge/wasmedge.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <iterator>

#define ITERS 1
// #define ITERS 1000

int run(const uint8_t *WASM, size_t len) {
  /* Create the VM context. */
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(NULL, NULL);

  /* The parameters and returns arrays. */
  WasmEdge_Value Params[1] = { WasmEdge_ValueGenI32(10) };
  WasmEdge_Value Returns[1];
  /* Function name. */
  WasmEdge_String FuncName = WasmEdge_StringCreateByCString("main");
  /* Run the WASM function from file. */
  WasmEdge_Result Res;

  Res = WasmEdge_VMRunWasmFromBuffer(
    VMCxt, WASM, len, FuncName, Params, 0, Returns, 1);

  if (WasmEdge_ResultOK(Res)) {
    printf("Get result: %d\n", WasmEdge_ValueGetI32(Returns[0]));
  } else {
    printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
  }

  std::cout << "Done" << std::endl;

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_StringDelete(FuncName);
  return 0;
}

static std::vector<uint8_t> read_file (const std::string filename)
{
    // binary mode is only for switching off newline translation
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
      std::cout << "Error: cannot open " << filename << std::endl;
      exit(1);
    }
    file.unsetf(std::ios::skipws);

    std::streampos file_size;
    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> vec;
    vec.reserve(file_size);
    vec.insert(vec.begin(),
               std::istream_iterator<unsigned char>(file),
               std::istream_iterator<unsigned char>());
    return (vec);
}

int runfile(char* name) {
  std::vector<uint8_t> buffer = read_file(name);

  int ret;

  for (int i = 0; i < ITERS; i++) {
    ret = run(buffer.data(), buffer.size());
    if (ret != 0) {
      std::cout << "Error: run failed" << std::endl;
      exit(1);
    }
    if (i % 100 == 99) {
      std::cout << "Iteration " << i << std::endl;
    }
  }

  return ret;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s WASM_FILE\n", argv[0]);
    return -1;
  }
  return runfile(argv[1]);
}
