local slang_dir = os.getenv('VULKAN_SDK')

if not slang_dir then
	error('slang: Cannot find SDK')
else
	io.write('slang: Found SDK at \'', slang_dir, '\'\n')
end

if not string.find(slang_dir, '/', #slang_dir - 1) and not string.find(slang_dir, '\\', #slang_dir - 1) then
	slang_dir = slang_dir .. '/'
end

local slang = mg.project({
	name = 'slang',
	type = mg.project_type.prebuilt,
	static_libraries = {'slang.lib'},
	static_library_directories = {slang_dir .. 'lib/'}
})

return {project = slang, includes = {slang_dir .. 'Include/'}}