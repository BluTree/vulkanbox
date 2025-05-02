local vma_version = '3.2.1'

local vma_dest_dir = mg.get_build_dir() .. 'deps/vma/'
local vma_url = 'https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v'.. vma_version .. '.zip'
if net.download(vma_url, vma_dest_dir) then
	io.write('vma: Updated to ', vma_version, '\n')
else
	io.write('vma: up-to-date\n')
end

return {includes = {'deps/' .. vma_dest_dir .. 'include/'}}