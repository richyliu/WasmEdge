# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2019-2022 Second State INC

# Only Linux systems support wasmedge_process now.
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_subdirectory(wasmedge_process)
endif()

if(WASMEDGE_WASINN_BACKEND)
  add_subdirectory(wasi_nn)
endif()
