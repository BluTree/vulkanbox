require('mingen/helpers')

if mg.need_generate() then
	mg.configurations({'debug', 'release'})
end

vulkan = require('deps/vulkan')
imgui = require('deps/imgui')
mincore = require('deps/mincore')

local vkb = mg.project({
	name = 'vulkanbox',
	type = mg.project_type.executable,
	sources = {'src/main.cpp'},
	includes = merge(
		{mg.get_build_dir() .. 'deps/'},
		imgui.includes,
		vulkan.includes,
		mincore.includes
	),
	compile_options = {'-g', '-std=c++20', '-Wall', '-Wextra', '-Werror'},
	link_options = {'-g'},
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

if mg.need_generate() then
	mg.generate({vkb})
end