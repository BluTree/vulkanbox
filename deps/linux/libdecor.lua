require('mg/helpers')

if (not find_package('libdecor-0')) then
	error('libdecor: Cannot find SDK')
end

libdecor_package = package_properties('libdecor-0')

local libdecor_prj = mg.project({
	name = 'libdecor',
	type = mg.project_type.prebuilt,
	static_libraries = libdecor_package.static_libraries,
	static_library_directories = libdecor_package.static_library_directories,
})

return {project = {libdecor_prj}, includes = libdecor_package.includes}