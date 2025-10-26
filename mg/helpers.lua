-- merge multiples arrays into a single one. Also works with mix of single values and arrays.
function merge(...)
	local res = {}
	local i = 1
	for _,v in ipairs({...}) do
		if type(v) == 'table' and v[1] ~= nil then
			for j=1,#v do
				res[i] = v[j]
				i = i + 1
			end
		elseif type(v) ~= nil then
			if type(v) ~= 'table' or next(v) ~= nil then
				res[i] = v
				i = i + 1
			end
		end
	end

	return res
end

function split_str(input, sep)
	if sep == nil then
		sep = '%s'
	end
	local t = {}
	for str in string.gmatch(input, "([^"..sep.."]+)") do
		table.insert(t, str)
  	end
	return t
end

-- remove platform specific sources in prj that aren't from the current platform
function remove_platform_sources(prj)
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
	local j = #prj.sources
	while i <= j do
		for k=1,#excluded_sources do
			pos = string.find(prj.sources[i].file, excluded_sources[k])
			if pos ~= nil then
				prj.sources[i] = prj.sources[j]
				prj.sources[j] = nil
				j = j - 1
				i = i - 1
				break
			end
		end
		i = i + 1
	end
end

function find_package(pkg_name)
	return os.execute('pkgconf --exists ' .. pkg_name).code == 0
end

function package_properties(pkg_name)
	local libs_stdout = os.execute('pkgconf --libs-only-l ' .. pkg_name).stdout
	local libs = {}
	if (libs_stdout ~= '\n') then
		for _,str in ipairs(split_str(string.sub(libs_stdout, 1, #libs_stdout - 1), ' ')) do
			table.insert(libs, string.sub(str, 3))
		end
	end

	local lib_dirs_stdout = os.execute('pkgconf --libs-only-l ' .. pkg_name).stdout
	local lib_dirs = {}
	if (lib_dirs_stdout ~= '\n') then
		for _,str in ipairs(split_str(string.sub(lib_dirs_stdout, 1, #lib_dirs_stdout - 1), ' ')) do
			table.insert(lib_dirs, string.sub(str, 3))
		end
	end

	local inc_dirs_stdout = os.execute('pkgconf --cflags-only-I ' .. pkg_name).stdout
	local inc_dirs = {}
	if (inc_dirs_stdout ~= '\n') then
		for _,str in ipairs(split_str(string.sub(inc_dirs_stdout, 1, #inc_dirs_stdout - 1), ' ')) do
			table.insert(inc_dirs, string.sub(str, 3))
		end
	end

	return {static_libraries = libs, static_library_directories = lib_dirs, includes = inc_dirs}
end

function package_property(pkg_name, var)
	local stdout = os.execute('pkgconf --variable=' .. var .. ' ' .. pkg_name).stdout
	return string.sub(stdout, 1, #stdout - 1)
end