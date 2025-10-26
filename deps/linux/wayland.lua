require('mg/helpers')

function generate_protocol(prj, template, output)
	mg.add_pre_build_cmd(prj, {
		input = template,
		output = output .. '.h',
		cmd = 'wayland-scanner client-header ${in} ${out}'
	})
	mg.add_pre_build_cmd(prj, {
		input = template,
		output = output .. '.c',
		cmd = 'wayland-scanner private-code ${in} ${out}'
	})
end

if ((not find_package('wayland-client')) or (not find_package('wayland-server')) or
	(not find_package('wayland-egl')) or (not find_package('wayland-cursor'))) then
	error('wayland: Cannot find SDK')
end

local wayland_client = package_properties('wayland-client')
local wayland_server = package_properties('wayland-server')
local wayland_egl = package_properties('wayland-egl')
local wayland_cursor = package_properties('wayland-cursor')

local wayland_lib_prj = mg.project({
	name = "wayland_libs",
	type = mg.project_type.prebuilt,
	static_libraries = merge(wayland_client.static_libraries,
                             wayland_server.static_libraries,
                             wayland_egl.static_libraries,
                             wayland_cursor.static_libraries),
	static_library_directories = merge(wayland_client.static_library_directories,
                                         wayland_server.static_library_directories,
                                         wayland_egl.static_library_directories,
                                         wayland_cursor.static_library_directories)
})

local lib_includes = merge(wayland_client.includes, wayland_server.includes,
                           wayland_egl.includes, wayland_cursor.includes)

local protocols_source_dir = mg.get_build_dir() .. 'deps/wayland/'

local wayland_protocols_prj = mg.project({
	name = "wayland_protocols",
	type = mg.project_type.sources,
	sources = {
		protocols_source_dir .. 'wayland-client-protocol.c',
		protocols_source_dir .. 'wayland-xdg-shell-client-protocol.c',
		protocols_source_dir .. 'wayland-xdg-decoration-protocol.c'},
	compile_options = {'-g', '-O2', '-x', 'c'},
	includes = lib_includes,
	dependencies = {wayland_lib_prj}
})

local wayland_client_root = package_property('wayland-client', 'pkgdatadir') .. '/'
local wayland_protocols_root = package_property('wayland-protocols', 'pkgdatadir') .. '/'

local protocols = {
	{
		input = wayland_client_root .. 'wayland.xml',
		output = protocols_source_dir .. 'wayland-client-protocol'
	},
	{
		input = wayland_protocols_root .. 'stable/xdg-shell/xdg-shell.xml',
		output = protocols_source_dir .. 'wayland-xdg-shell-client-protocol'
	},
	{
		input = wayland_protocols_root	 .. 'unstable/xdg-decoration/xdg-decoration-unstable-v1.xml',
		output = protocols_source_dir .. 'wayland-xdg-decoration-protocol'
	},
}

for _,protocol in ipairs(protocols) do
	generate_protocol(wayland_protocols_prj, protocol.input, protocol.output)
end

return {projects = {wayland_lib_prj, wayland_protocols_prj}, includes = merge(lib_includes, 'deps/linux/' .. protocols_source_dir)}
