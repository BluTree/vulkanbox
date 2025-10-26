local modular_win32_dest_dir = mg.get_build_dir() .. 'deps/modular_win32/'
local modular_win32_url = 'https://github.com/BluTree/WindowsHModular/archive/refs/heads/dev.zip'
if net.download(modular_win32_url, modular_win32_dest_dir) then
	io.write('modular_win32: Updated \'dev\' branch\n')
else
	io.write('modular_win32: up-to-date\n')
end

return {includes = 'deps/' .. modular_win32_dest_dir .. 'include/'}