#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbuiltin-macro-redefined"

#define cpp_ver __cplusplus
#undef __cplusplus
#include <vulkan/vk_enum_string_helper.h>
#define __cplusplus cpp_ver

#pragma clang diagnostic pop