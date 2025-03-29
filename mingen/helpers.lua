-- merge multiples arrays into a single one. Also works with mix of single values and arrays.
function merge(...)
	res = {}
	i = 1
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