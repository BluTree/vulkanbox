local vulkan_dir = os.getenv('VULKAN_SDK')

if not vulkan_dir then
	error_msg = ''
	if (mg.platform() == 'linux') then
		error_msg ='vulkan: Cannot find SDK\nLinux currently only supports LunarG\' official archive'
	else
		error_msg = 'vulkan: Cannot find SDK'
	end
	error(error_msg)
else
	io.write('vulkan: Found SDK at \'', vulkan_dir, '\'\n')
end

if not string.find(vulkan_dir, '/', #vulkan_dir - 1) and not string.find(vulkan_dir, '\\', #vulkan_dir - 1) then
	vulkan_dir = vulkan_dir .. '/'
end

if (mg.platform() == 'windows') then
	local vulkan = mg.project({
		name = 'vulkan',
		type = mg.project_type.prebuilt,
		static_libraries = {'vulkan-1.lib'},
		static_library_directories = {vulkan_dir .. 'lib/'}
	})

	return {project = vulkan, includes = {vulkan_dir .. 'Include/'}}
elseif (mg.platform() == 'linux') then
	local vulkan = mg.project({
		name = 'vulkan',
		type = mg.project_type.prebuilt,
		static_libraries = {'vulkan'},
		static_library_directories = {vulkan_dir .. 'lib/'}
	})

	return {project = vulkan, includes = {vulkan_dir .. 'include/'}}
end