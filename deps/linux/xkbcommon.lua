require('mg/helpers')

if (not find_package('xkbcommon')) then
	error('xkbcommon: Cannot find SDK')
end

xkbcommon_package = package_properties('xkbcommon')

local xkb_prj = mg.project({
	name = 'xkbcommon',
	type = mg.project_type.prebuilt,
	static_libraries = xkbcommon_package.static_libraries,
	static_library_directories = xkbcommon_package.static_library_directories,
})

return {project = {xkb_prj}, includes = xkbcommon_package.includes}
