require('mingen/helpers')

if mg.need_generate() then
	mg.configurations({'debug', 'release'})
end

vulkan = require('deps/vulkan')
imgui = require('deps/imgui')
mincore = require('deps/mincore')

include_dirs = {}
if (mg.platform() == 'windows') then
	modular_win32 = require('deps/modular_win32')
	include_dirs = merge(
		{mg.get_build_dir() .. 'deps/', 'src/'},
		imgui.includes,
		vulkan.includes,
		mincore.includes,
		modular_win32.includes
	)
else
	include_dirs = merge(
		{mg.get_build_dir() .. 'deps/', 'src/'},
		imgui.includes,
		vulkan.includes,
		mincore.includes
	)
end

platform_define = ''
if (mg.platform() == 'windows') then
	platform_define = '-D"VKB_WINDOWS" -D"_CRT_SECURE_NO_WARNINGS"'
elseif (mg.platform() == 'linux') then
	platform_define = '-D"VKB_LINUX"'
else
	error("Unsupported platform")
end

platform_compile_options = {}
if (mg.platform() == 'windows') then
	platform_compile_options = {
		'-Wno-ignored-pragma-intrinsic' -- Warning raised on win32/misc.h pragma intrinsic
	}
end

platform_link_options = {}
if (mg.platform() == 'windows') then
	platform_link_options = {
		'-Xlinker /incremental:no'
	}
end

local vkb = mg.project({
	name = 'vulkanbox',
	type = mg.project_type.executable,
	sources = {'src/**.cc'},
	includes = include_dirs,
	compile_options = merge('-g', '-std=c++20', '-Wall', '-Wextra', '-Werror', platform_define, platform_compile_options),
	link_options = merge('-g', platform_link_options),
	dependencies = {imgui.project, vulkan.project, mincore.project}
})

-- Manage list of source patterns that are platform specific
local excluded_sources = {'.windows.cc', '.linux.cc'}
local platform_specific_sources = '.' .. mg.platform() .. '.cc'
for i=1,#excluded_sources do
	if excluded_sources[i] == platform_specific_sources then
		table.remove(excluded_sources, i)
		break
	end
end

-- Remove the platform specific sources not valid on the generated platform
local i = 1
local j = #vkb.sources
while i <= j do
	for k=1,#excluded_sources do
		pos = string.find(vkb.sources[i].file, excluded_sources[k])
		if pos ~= nil then
			vkb.sources[i] = vkb.sources[j]
			vkb.sources[j] = nil
			j = j - 1
		end
	end
	i = i + 1
end

-- Shaders
shaders = mg.collect_files('res/shaders/*.glsl')
for i=1,#shaders do
	shader_stage = ''
	if (string.find(shaders[i], '.vert') ~= nil) then
		shader_stage = 'vertex'
	elseif (string.find(shaders[i], '.frag') ~= nil) then
		shader_stage = 'fragment'
	end

	spirv = mg.get_build_dir() .. 'bin/' .. string.gsub(shaders[i], '.glsl', '.spirv')
	mg.add_post_build_cmd(vkb, {
		input = shaders[i],
		output = spirv,
		cmd = 'glslc -fshader-stage=' .. shader_stage .. ' ${in} -o ${out}'
	})
end

if mg.need_generate() then
	mg.generate({vkb})
end