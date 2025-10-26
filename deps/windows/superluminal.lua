local superluminal_dir = os.getenv('SUPERLUMINAL_API_DIR')

if not superluminal_dir then
	io.write('superluminal: Cannot find SDK\n')
	return nil
else
	io.write('superluminal: Found SDK at \'', superluminal_dir, '\'\n')
end

if not string.find(superluminal_dir, '/', #superluminal_dir - 1) and not string.find(superluminal_dir, '\\', #superluminal_dir - 1) then
	superluminal_dir = superluminal_dir .. '/'
end

local superluminal = mg.project({
	name = 'superluminal',
	type = mg.project_type.prebuilt,
	static_libraries = {'PerformanceAPI_MD.lib'},
	static_library_directories = {superluminal_dir .. 'lib/x64/'}
})

return {project = superluminal, includes = {superluminal_dir .. 'include/'}}