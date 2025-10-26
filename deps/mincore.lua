local mincore_dest_dir = mg.get_build_dir() .. 'deps/mincore/'
-- local mincore_url = 'https://github.com/BluTree/mincore/archive/refs/heads/main.zip'
-- if net.download(mincore_url, mincore_dest_dir) then
-- 	io.write('mincore: Updated \'main\' branch\n')
-- 	os.copy_file('config/mincore/config.hh', mincore_dest_dir .. 'config.hh')
-- else
-- 	io.write('mincore: up-to-date\n')
-- end

local mincore = require('build/deps/mincore/mingen')

return {project = mincore.project, includes = 'deps/' .. mincore_dest_dir .. 'src/'}