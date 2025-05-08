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
		else
			res[i] = v
			i = i + 1
		end
	end

	return res
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
			end
		end
		i = i + 1
	end
end