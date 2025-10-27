--[[
This file contains the weak parts of current mingen. Some works needs to be done to avoid dealing with this issues more frequently later
By ease of use, the current project uses default win32 and vulkan backends. Win32 backend is fine, but vulkan backends has issues:
 - The vulkan backend needs includes to vulkan headers (which is fine), but vulkan is currently already defined as a project in vulkan.lua.
   A correct way of doing this would be to require('vulkan') in order to have access to vulkan.includes.
   But the way require is implemented make it impossible. The require always looks for scripts relative to the cwd, and not the executing script.
   Moreover, the implementation of require preventing to run multiple times the same script cannot work, even if we manually resolve the path to 'vulkan',
   because it only register the name of the require, and not the complete file path asked.

 - The script returns a table to be used by the main script (../mingen.lua). This is mainly architectural, it could be done in other ways.
   But the includes path returned by the script have issues, because the relative path is resolved when using it, not creating it.
   In this case, without the 'deps/' before imgui_dest_dir, the path returned would be '../build/deps/imgui/',
   but would be resolved on the main script, making it looking for a 'build/' directory outside the repo!
   - Solution : Convert path to absolute
--]]

local imgui_dest_dir = mg.get_build_dir() .. 'deps/imgui/'
local imgui_url = 'https://github.com/ocornut/imgui/archive/refs/heads/docking.zip'
if net.download(imgui_url, imgui_dest_dir) then
	io.write('imgui: Updated \'docking\' branch\n')
	os.copy_file('config/imgui/imconfig.h', imgui_dest_dir .. 'imconfig.h')
else
	io.write('imgui: up-to-date\n')
end

require('deps/vulkan')

if (mg.platform() == 'windows') then
	local imgui = mg.project({
		name = "imgui",
		type = mg.project_type.sources,
		sources = {
			imgui_dest_dir .. '*.cpp',
			imgui_dest_dir .. 'backends/imgui_impl_vulkan.cpp',
			imgui_dest_dir .. 'backends/imgui_impl_win32.cpp'},
		includes = merge(imgui_dest_dir, imgui_dest_dir .. 'backends/', vulkan.includes),
		compile_options = {'-g'}
	})

	return {project = imgui, includes = {'deps/' .. imgui_dest_dir, 'deps/' .. imgui_dest_dir .. 'backends/'}}
elseif (mg.platform() == 'linux') then
	local imgui = mg.project({
		name = "imgui",
		type = mg.project_type.sources,
		sources = {
			imgui_dest_dir .. '*.cpp',
			imgui_dest_dir .. 'backends/imgui_impl_vulkan.cpp'},
		includes = merge(imgui_dest_dir, imgui_dest_dir .. 'backends/', vulkan.includes),
		compile_options = {'-g'}
	})

	return {project = imgui, includes = {'deps/' .. imgui_dest_dir, 'deps/' .. imgui_dest_dir .. 'backends/'}}
end