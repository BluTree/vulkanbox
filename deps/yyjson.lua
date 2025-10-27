local yyjson_version = '0.11.0'

local yyjson_dest_dir = mg.get_build_dir() .. 'deps/yyjson/'
local yyjson_url = 'https://github.com/ibireme/yyjson/archive/refs/tags/' .. yyjson_version ..'.zip'
if net.download(yyjson_url, yyjson_dest_dir) then
	io.write('yyjson: Updated to', yyjson_version, '\n')
else
	io.write('yyjson: up-to-date\n')
end

local yyjson = mg.project({
	name = "yyjson",
	type = mg.project_type.sources,
	sources = {
		yyjson_dest_dir .. 'src/yyjson.c'},
	compile_options = {'-g', '-O2', '-x', 'c'}
})

return {project = yyjson, includes = {'deps/' .. yyjson_dest_dir .. 'src/'}}