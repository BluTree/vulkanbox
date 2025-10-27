#pragma once

#define VMA_CONFIGURATION_USER_INCLUDES_H <vkb/vk/vma/vma_user.hh>
#define VMA_ASSERT(expr) vkb::log::assert(expr, "vma")
// #define VMA_DEBUG_LOG_FORMAT(format, ...) vkb::log::info("vma: " format, __VA_ARGS__)
#define VMA_DEBUG_LEAK_LOG_FORMAT(format, ...) vkb::log::warn("vma: " format, __VA_ARGS__)
#define VMA_MIN(a, b) (a < b ? a : b)
#define VMA_MAX(a, b) (a > b ? a : b)
#define VMA_CPP20 0

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-private-field"
// Do not show implementation on intellisense, as it will cause include issues (due to
// nostdinc++ flag on sources)
#ifdef __INTELLISENSE__
#define OLD_VALUE __INTELLISENSE__
#undef __INTELLISENSE__
#endif
#include <vk_mem_alloc.h>
#ifdef OLD_VALUE
#define __INTELLISENSE__
#undef OLD_VALUE
#endif
#pragma clang diagnostic pop