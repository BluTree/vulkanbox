require('mg/helpers')

if mg.need_generate() then
	mg.configurations({'debug', 'release'})
end

vulkan = require('deps/vulkan')
imgui = require('deps/imgui')
mincore = require('deps/mincore')
stb = require('deps/stb')
vma = require('deps/vma')
slang = require('deps/slang')
yyjson = require('deps/yyjson')


include_dirs = merge(
	{mg.get_build_dir() .. 'deps/', 'src/'},
	imgui.includes,
	vulkan.includes,
	mincore.includes,
	-- stb.includes
	vma.includes,
	yyjson.includes
)

platform_define = {}
if (mg.platform() == 'windows') then
	platform_define = {
		'-D"VKB_WINDOWS"',
		'-D"_CRT_SECURE_NO_WARNINGS"',
		'-D"_MT"',
		'-D"_DLL"',
		'-D_DISABLE_STRING_ANNOTATION',
		'-D_DISABLE_VECTOR_ANNOTATION',
	}
elseif (mg.platform() == 'linux') then
	platform_define = {'-D"VKB_LINUX"'}
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
		'-fuse-ld=lld-link',
		'-Xlinker /incremental:no',
		'-Xlinker /nodefaultlib:libcmt.lib',
		'-Xlinker /defaultlib:msvcrt.lib',
		-- '-lclang_rt.asan_dynamic_runtime_thunk-x86_64.lib',
		-- '-lclang_rt.asan_dynamic-x86_64.lib',
	}
end

platform_deps = {}
if (mg.platform() == 'windows') then
	modular_win32 = require('deps/windows/modular_win32')
	superluminal = require('deps/windows/superluminal')
	if superluminal ~= nil then
		include_dirs = merge(
			include_dirs,
			superluminal.includes,
			modular_win32.includes
		)
		platform_deps = {superluminal.project}
		table.insert(platform_define, '-D"USE_SUPERLUMINAL"')
		table.insert(platform_link_options, '-Xlinker /ignore:4099')
	else
		include_dirs = merge(
			include_dirs,
			modular_win32.includes
		)
	end
elseif (mg.platform() == 'linux') then
	wayland = require('deps/linux/wayland')
	xkb = require('deps/linux/xkbcommon')
	libdecor = require('deps/linux/libdecor')

	include_dirs = merge(
		include_dirs,
		wayland.includes,
		xkb.includes,
		libdecor.includes)
	platform_deps = merge(wayland.projects, xkb.project, libdecor.project)
end

local vkb = mg.project({
	name = 'vulkanbox',
	type = mg.project_type.executable,
	sources = {'src/vkb/**.cc'},
	includes = include_dirs,
	compile_options = merge('-g', '-std=c++20', '-Wall', '-Wextra', '-Werror', '-nostdinc++', platform_define, platform_compile_options),
	link_options = merge(platform_link_options, '-g'),
	dependencies = merge(imgui.project, vulkan.project, mincore.project, yyjson.project, platform_deps),
	release = {
		compile_options = {'-O2'}
	}
})

remove_platform_sources(vkb)

for i=1, #vkb.sources do
	if string.find(vkb.sources[i].file, 'vma') ~= nil then
		vkb.sources[i].compile_options = string.gsub(vkb.compile_options, ' %-nostdinc%+%+', '')
	end
end

local slangrc = mg.project({
	name = 'slangrc',
	type = mg.project_type.executable,
	sources = {'src/slangrc/**.cc'},
	includes = merge(include_dirs, slang.includes),
	compile_options = merge('-g', '-std=c++20', '-Wall', '-Wextra', '-Werror', platform_define, platform_compile_options),
	link_options = merge('-g', platform_link_options),
	dependencies = merge(slang.project, mincore.project),
	release = {
		compile_options = {'-O2'}
	}
})

remove_platform_sources(slangrc)

slangrc_ext = ''
if (mg.platform() == 'windows') then
	slangrc_ext = '.exe'
end
slangrc_bin = '"' .. mg.get_build_dir() .. 'bin/slangrc' .. slangrc_ext .. '"'
shaders = mg.collect_files('res/shaders/*.slang')
for i=1,#shaders do
	spirv = mg.get_build_dir() .. 'bin/' .. string.gsub(shaders[i], '.slang', '.spv')
	mg.add_post_build_cmd(slangrc, {
		input = shaders[i],
		output = spirv,
		cmd = slangrc_bin .. ' ${in} ${out}'
	})
end

-- Textures
textures = mg.collect_files('res/textures/*.png')
for i=1,#textures do
	mg.add_post_build_copy(vkb, {
		input = textures[i],
		output = mg.get_build_dir() .. 'bin/' .. textures[i];
	})
end

if mg.need_generate() then
	mg.generate({vkb, slangrc})
end